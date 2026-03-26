// main.cpp — テストベンチ エントリーポイント
// 3フェーズ実行モデル: 入力 → ロジック → 出力
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== バスインスタンス（グローバルスコープで生成）=====
static TwoWire  mpuWire   = TwoWire(0);       // MPU6500用I2C
static SPIClass sharedSpi = SPIClass(FSPI);    // SD用 共有SPIバス

// システムデータ
SystemData systemData;

// ===== モジュールインスタンス =====

// 出力モジュール（TftModuleはLovyanGFXコンポーネントを内部生成）
TftModule   tftModule(TFT_CONFIG);
ServoModule servoModule(SERVO_CONFIG);

// 入力モジュール（TouchModuleはTftModule内部のLCDデバイスを参照）
TouchModule   touchModule(TOUCH_CONFIG, tftModule.getLcd());
Mpu6500Module mpu6500Module(MPU6500_CONFIG, &mpuWire);
SdModule      sdModule(SD_CONFIG, &sharedSpi);
CameraModule  cameraModule(CAMERA_CONFIG);

// 入出力モジュール（入力/出力フェーズで個別メソッドを呼び分ける）
BleModule bleModule(BLE_CONFIG);

// モジュール配列
// 注意: BleModuleは入出力両方の処理を持つため配列には含めず、
//       loop()内でupdateInput()/updateOutput()を個別に呼び出す
IModule* inputModules[] = {
    &touchModule,
    &mpu6500Module,
    &sdModule,
    &cameraModule,
};
const int INPUT_COUNT = sizeof(inputModules) / sizeof(inputModules[0]);

IModule* outputModules[] = {
    &tftModule,
    &servoModule,
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
        snprintf(data.tft.line1, sizeof(data.tft.line1),
                 "Ax:%.2f Ay:%.2f Az:%.2f  ",
                 data.mpu.accelX, data.mpu.accelY, data.mpu.accelZ);
        snprintf(data.tft.line2, sizeof(data.tft.line2),
                 "Gx:%.1f Gy:%.1f Gz:%.1f  ",
                 data.mpu.gyroX, data.mpu.gyroY, data.mpu.gyroZ);
    } else {
        strncpy(data.tft.line1, "MPU6500: No data      ", sizeof(data.tft.line1));
        data.tft.line2[0] = '\0';
    }

    // タッチでサーボ制御（X座標 0-320 → 角度 0-180）
    if (data.touch.touchPressed) {
        data.servo.targetAngle = map(data.touch.touchX, 0, 320, 0, 180);
        snprintf(data.tft.line3, sizeof(data.tft.line3),
                 "Touch:X=%3d Y=%3d Sv:%3d ",
                 data.touch.touchX, data.touch.touchY, data.servo.targetAngle);
    } else {
        snprintf(data.tft.line3, sizeof(data.tft.line3),
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
    snprintf(data.tft.line4, sizeof(data.tft.line4),
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
    snprintf(data.tft.line5, sizeof(data.tft.line5),
             "Heap:%dK PSRAM:%dK  ",
             (int)(ESP.getFreeHeap() / 1024),
             (int)(ESP.getFreePsram() / 1024));

    // init失敗モジュールの定期再試行
    if (reinitTimer.getNowTime() >= REINIT_INTERVAL_MS) {
        reinitTimer.setTime();
        IModule* allModules[] = {
            &touchModule, &mpu6500Module, &sdModule, &cameraModule,
            &bleModule, &tftModule, &servoModule,
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

// ===== ユーティリティ =====

static void initModuleArray(IModule** modules, int count, const char* label) {
    const int MAX_RETRY = 3;
    for (int i = 0; i < count; i++) {
        bool success = false;
        for (int r = 0; r < MAX_RETRY; r++) {
            if (modules[i]->init()) { success = true; break; }
            delay(100);
        }
        if (!success) {
            Serial.printf("[System] %s Module %d: init失敗、無効化\n", label, i);
            modules[i]->enabled = false;
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

    // モジュール初期化
    // TftModuleのinit()でLovyanGFXが初期化される（SPIバスを内部で構成）
    // 出力モジュールを先に初期化（TouchModuleがLCDデバイスを使用するため）
    initModuleArray(outputModules, OUTPUT_COUNT, "Output");
    initModuleArray(inputModules,  INPUT_COUNT,  "Input");

    // BleModuleは配列外のため個別にinit
    {
        const int MAX_RETRY = 3;
        bool success = false;
        for (int r = 0; r < MAX_RETRY; r++) {
            if (bleModule.init()) { success = true; break; }
            delay(100);
        }
        if (!success) {
            Serial.println("[System] BLE Module: init失敗、無効化");
            bleModule.enabled = false;
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
            inputModules[i]->update(systemData);
        }
    }
    if (bleModule.enabled) {
        bleModule.updateInput(systemData);
    }

    // 2. ロジックフェーズ
    applyPattern(systemData);

    // カメラフレームバッファはロジックフェーズ終了時に解放
    cameraModule.releaseFrame();

    // 3. 出力フェーズ
    for (int i = 0; i < OUTPUT_COUNT; i++) {
        if (outputModules[i]->enabled) {
            outputModules[i]->update(systemData);
        }
    }
    if (bleModule.enabled) {
        bleModule.updateOutput(systemData);
    }
}
