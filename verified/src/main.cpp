// main.cpp — テストベンチ エントリーポイント
// 3フェーズ実行モデル: 入力 → ロジック → 出力
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== バスインスタンス（グローバルスコープで生成）=====
static TwoWire  mpuWire   = TwoWire(1);       // MPU6500用I2C（バス0はesp_cameraのSCCBが使用）
// SD用SPIバス（LovyanGFXがFSPIを内部管理するが、use_lock=trueで排他制御されるため共存可能）
static SPIClass sharedSpi = SPIClass(FSPI);

// システムデータ
SystemData systemData;

// ===== モジュールインスタンス =====

// 入出力モジュール（両方の配列に含める）
DisplayBoardModule displayBoardModule(DISPLAY_BOARD_CONFIG);
BleModule          bleModule(BLE_CONFIG);

// 入力専用モジュール
Mpu6500Module mpu6500Module(MPU6500_CONFIG, &mpuWire);
SdModule      sdModule(SD_CONFIG, &sharedSpi);
CameraModule  cameraModule(CAMERA_CONFIG);

// 出力専用モジュール
ServoModule servoModule(SERVO_CONFIG);

// モジュール配列
// 配列の並び順 = フェーズ内の実行順序
IModule* inputModules[] = {
    &bleModule,           // BLE受信を先に反映
    &displayBoardModule,  // タッチ読み取り
    &mpu6500Module,
    &sdModule,
    &cameraModule,
};
const int INPUT_COUNT = sizeof(inputModules) / sizeof(inputModules[0]);

IModule* outputModules[] = {
    &servoModule,
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
    // MPU6500データをTFT表示
    if (data.mpu.isValid) {
        snprintf(data.display.line1, sizeof(data.display.line1),
                 "Ax:%.2f Ay:%.2f Az:%.2f  ",
                 data.mpu.accelX, data.mpu.accelY, data.mpu.accelZ);
        snprintf(data.display.line2, sizeof(data.display.line2),
                 "Gx:%.1f Gy:%.1f Gz:%.1f  ",
                 data.mpu.gyroX, data.mpu.gyroY, data.mpu.gyroZ);
    } else {
        strncpy(data.display.line1, "MPU6500: No data      ", sizeof(data.display.line1));
        data.display.line2[0] = '\0';
    }

    // タッチでサーボ制御（X座標 0-320 → 角度 0-180）
    if (data.display.touchPressed) {
        data.servo.targetAngle = map(data.display.touchX, 0, 320, 0, 180);
        snprintf(data.display.line3, sizeof(data.display.line3),
                 "Touch:X=%3d Y=%3d Sv:%3d ",
                 data.display.touchX, data.display.touchY, data.servo.targetAngle);
    } else {
        snprintf(data.display.line3, sizeof(data.display.line3),
                 "Servo: %3d deg        ", data.servo.currentAngle);
    }

    // カメラ + SD + BLE ステータス
    char camInfo[16] = "CAM:--";
    if (data.camera.isValid && data.camera.frameReady) {
        snprintf(camInfo, sizeof(camInfo), "CAM:OK");
    }
    char sdInfo[16] = "SD:--";
    if (data.sd.isValid) {
        snprintf(sdInfo, sizeof(sdInfo), "SD:%s",
                 data.sd.testPassed ? "OK" : "NG");
    }
    snprintf(data.display.line4, sizeof(data.display.line4),
             "%s %s BLE:%s  ", camInfo, sdInfo,
             data.ble.connected ? "ON" : "--");

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

    // メモリ情報
    snprintf(data.display.line5, sizeof(data.display.line5),
             "Heap:%dK PSRAM:%dK  ",
             (int)(ESP.getFreeHeap() / 1024),
             (int)(ESP.getFreePsram() / 1024));

    // init失敗モジュールの定期再試行
    if (reinitTimer.getNowTime() >= REINIT_INTERVAL_MS) {
        reinitTimer.setTime();
        IModule* allModules[] = {
            &displayBoardModule, &mpu6500Module, &sdModule, &cameraModule,
            &bleModule, &servoModule,
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
    delay(3000); // シリアルモニタ接続待ち

    Serial.begin(115200);
    Serial.println("[System] テストベンチ起動");

    // バス初期化（全モジュールのinit()より前に実行）
    mpuWire.begin(I2C_SDA_PIN, I2C_SCL_PIN);                       // I2Cバス
    sharedSpi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);  // 共有SPIバス（SDカード用）

    // 全モジュールの重複initを防ぐため、ユニークなモジュールのみ初期化
    // inputModules と outputModules に重複するモジュールがあるため、
    // 全ユニークモジュールを1回だけinit()する
    IModule* allModules[] = {
        &bleModule, &displayBoardModule, &mpu6500Module,
        &sdModule, &cameraModule, &servoModule,
    };
    const int ALL_COUNT = sizeof(allModules) / sizeof(allModules[0]);

    const int MAX_RETRY = 3;
    for (int i = 0; i < ALL_COUNT; i++) {
        bool success = false;
        for (int r = 0; r < MAX_RETRY; r++) {
            if (allModules[i]->init()) { success = true; break; }
            delay(100);
        }
        if (!success) {
            Serial.printf("[System] Module %d: init失敗、無効化\n", i);
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

    // カメラフレームバッファはロジックフェーズ終了時に解放
    cameraModule.releaseFrame();

    // 3. 出力フェーズ
    for (int i = 0; i < OUTPUT_COUNT; i++) {
        if (outputModules[i]->enabled) {
            outputModules[i]->updateOutput(systemData);
        }
    }
}
