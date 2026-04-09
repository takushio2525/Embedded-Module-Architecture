// main.cpp — 本実装 エントリーポイント
// 3フェーズ実行モデル: 入力 → ロジック → 出力
// 縦画面LCD: 上部にタッチ座標・MPU値、下部にカメラ画像を表示
// 注意: サーボ(SG90)は電流量の問題で除外、SDカードは未テストのため除外
//       （Configインスタンスは将来の再有効化のため残す）
#include <Arduino.h>
#include <SPI.h>
#include "driver/i2c.h"
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== バスインスタンス（グローバルスコープで生成）=====
// I2C: ESP-IDFレガシーAPI使用（Arduino Wireはesp_camera SCCBのレガシードライバと共存不可）
//      setup()内でi2c_param_config() + i2c_driver_install()により初期化する
// SD用SPIバス（LovyanGFXがFSPIを内部管理するが、use_lock=trueで排他制御されるため共存可能）
static SPIClass sharedSpi = SPIClass(FSPI);

// システムデータ
SystemData systemData;

// ===== モジュールインスタンス =====

// 入出力モジュール（両方の配列に含める）
DisplayBoardModule displayBoardModule(DISPLAY_BOARD_CONFIG);
BleModule          bleModule(BLE_CONFIG);

// 入力専用モジュール
Mpu6500Module mpu6500Module(MPU6500_CONFIG, 1);  // I2Cポート1（ポート0はesp_camera SCCBが使用）
CameraModule  cameraModule(CAMERA_CONFIG);

// 未使用モジュール（Configは残すがモジュール配列には含めない）
// SdModule      sdModule(SD_CONFIG, &sharedSpi);  // SDカード未テスト
// ServoModule   servoModule(SERVO_CONFIG);         // SG90は電流量の問題で除外

// モジュール配列
// 配列の並び順 = フェーズ内の実行順序
IModule* inputModules[] = {
    &bleModule,           // BLE受信を先に反映
    &displayBoardModule,  // タッチ読み取り
    &mpu6500Module,
    &cameraModule,
};
const int INPUT_COUNT = sizeof(inputModules) / sizeof(inputModules[0]);

IModule* outputModules[] = {
    &displayBoardModule,  // 画面描画
    &bleModule,           // 全データ確定後にBLE送信
};
const int OUTPUT_COUNT = sizeof(outputModules) / sizeof(outputModules[0]);

// ===== ロジックフェーズ =====

// BLE経由でIMUデータを送信する周期タイマー
static ModuleTimer bleSendTimer;
static constexpr uint32_t BLE_SEND_INTERVAL_MS = 100;  // 100ms (10Hz)

// init失敗モジュールの再試行タイマーと間隔
static ModuleTimer reinitTimer;
static constexpr uint32_t REINIT_INTERVAL_MS = 5000;  // 5秒ごとに再試行

void applyPattern(SystemData& data) {
    // タッチ座標の表示
    if (data.display.touchPressed) {
        snprintf(data.display.line1, sizeof(data.display.line1),
                 "Touch: X=%3d Y=%3d  ",
                 data.display.touchX, data.display.touchY);
    } else {
        strncpy(data.display.line1, "Touch: ---            ",
                sizeof(data.display.line1));
    }

    // MPU6500データの表示
    if (data.mpu.isValid) {
        snprintf(data.display.line2, sizeof(data.display.line2),
                 "A:%.1f %.1f %.1f  ",
                 data.mpu.accelX, data.mpu.accelY, data.mpu.accelZ);
        snprintf(data.display.line3, sizeof(data.display.line3),
                 "G:%.0f %.0f %.0f  ",
                 data.mpu.gyroX, data.mpu.gyroY, data.mpu.gyroZ);
    } else {
        strncpy(data.display.line2, "MPU: No data          ",
                sizeof(data.display.line2));
        data.display.line3[0] = '\0';
    }

    // ステータス行（BLE + カメラ）
    snprintf(data.display.line4, sizeof(data.display.line4),
             "BLE:%s CAM:%s  ",
             data.ble.connected ? "ON" : "--",
             (data.camera.isValid && data.camera.frameReady) ? "OK" : "--");

    // メモリ情報
    snprintf(data.display.line5, sizeof(data.display.line5),
             "Heap:%dK PSRAM:%dK  ",
             (int)(ESP.getFreeHeap() / 1024),
             (int)(ESP.getFreePsram() / 1024));

    // カメラJPEGフレームをディスプレイDataに設定
    if (data.camera.isValid && data.camera.frameReady) {
        data.display.jpegData = cameraModule.getFrameBuffer();
        data.display.jpegSize = data.camera.frameSize;
    } else {
        data.display.jpegData = nullptr;
        data.display.jpegSize = 0;
    }

    // BLE接続中はIMUデータをNotifyで定期送信
    if (data.ble.connected && data.mpu.isValid &&
        bleSendTimer.getNowTime() >= BLE_SEND_INTERVAL_MS) {
        bleSendTimer.setTime();
        // CSV形式: "ax,ay,az,gx,gy,gz\n"
        int len = snprintf((char*)data.ble.txData, BLE_TX_BUFFER_SIZE,
                           "%.2f,%.2f,%.2f,%.1f,%.1f,%.1f\n",
                           data.mpu.accelX, data.mpu.accelY, data.mpu.accelZ,
                           data.mpu.gyroX, data.mpu.gyroY, data.mpu.gyroZ);
        data.ble.txLength    = (uint8_t)len;
        data.ble.sendRequest = true;
    }

    // init失敗モジュールの定期再試行
    if (reinitTimer.getNowTime() >= REINIT_INTERVAL_MS) {
        reinitTimer.setTime();
        IModule* allModules[] = {
            &displayBoardModule, &mpu6500Module, &cameraModule, &bleModule,
        };
        for (auto* mod : allModules) {
            if (!mod->enabled) {
                if (mod->init()) {
                    mod->enabled = true;
                    Serial.println("[System] モジュール再init成功、有効化");
                }
            }
        }
    }
}

// ===== セットアップ =====

void setup() {
    Serial.begin(115200);
    delay(3000); // USB CDC 接続待ち
    Serial.println("[System] 本実装起動");
    Serial.printf("[System] Free heap: %d, PSRAM: %d\n",
                  ESP.getFreeHeap(), ESP.getFreePsram());

    // バス初期化（全モジュールのinit()より前に実行）
    // I2Cバス（ポート1、esp_camera SCCBはポート0を使用）
    // ESP-IDFレガシーAPI使用（Arduino Wireはesp_camera SCCBと共存不可）
    i2c_config_t i2cConf = {};
    i2cConf.mode = I2C_MODE_MASTER;
    i2cConf.sda_io_num = I2C_SDA_PIN;
    i2cConf.scl_io_num = I2C_SCL_PIN;
    i2cConf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2cConf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2cConf.master.clk_speed = 400000;
    i2c_param_config(I2C_NUM_1, &i2cConf);
    i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0);

    sharedSpi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);  // 共有SPIバス

    Serial.println("[System] バス初期化完了");

    // 全ユニークモジュールを1回だけinit()する
    // 注意: 初期化順序 — ディスプレイ → カメラ → MPU → BLE
    //       BLEはメモリ消費が大きいため最後に初期化
    IModule* allModules[] = {
        &displayBoardModule, &mpu6500Module, &cameraModule, &bleModule,
    };
    const int ALL_COUNT = sizeof(allModules) / sizeof(allModules[0]);

    const char* moduleNames[] = {"Display", "MPU6500", "Camera", "BLE"};
    const int MAX_RETRY = 3;
    for (int i = 0; i < ALL_COUNT; i++) {
        Serial.printf("[System] %s init開始... (heap=%d)\n",
                      moduleNames[i], ESP.getFreeHeap());
        bool success = false;
        for (int r = 0; r < MAX_RETRY; r++) {
            if (allModules[i]->init()) { success = true; break; }
            delay(100);
        }
        if (success) {
            Serial.printf("[System] %s init成功\n", moduleNames[i]);
        } else {
            Serial.printf("[System] %s init失敗、無効化\n", moduleNames[i]);
            allModules[i]->enabled = false;
        }
    }

    bleSendTimer.setTime();
    reinitTimer.setTime();
    Serial.println("[System] 起動完了");
}

// ===== メインループ =====

void loop() {
    // 1. 入力フェーズ
    for (int i = 0; i < INPUT_COUNT; i++) {
        if (inputModules[i]->enabled) {
            inputModules[i]->updateInput(systemData);
        }
    }

    // 2. ロジックフェーズ
    applyPattern(systemData);

    // 3. 出力フェーズ
    for (int i = 0; i < OUTPUT_COUNT; i++) {
        if (outputModules[i]->enabled) {
            outputModules[i]->updateOutput(systemData);
        }
    }

    // カメラフレームバッファは出力フェーズ後に解放（描画で使用するため）
    cameraModule.releaseFrame();
}
