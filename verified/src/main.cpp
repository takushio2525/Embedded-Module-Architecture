// main.cpp — テストベンチ エントリーポイント
// 3フェーズ実行モデル: 入力 → ロジック → 出力
#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// バスインスタンス（グローバルスコープで生成）
static TwoWire  mpuWire  = TwoWire(0);  // MPU6500用I2C
static TFT_eSPI tftDriver;              // TFT LCD + タッチ共有ドライバ

// システムデータ
SystemData systemData;

// ===== モジュールインスタンス =====

// 入力モジュール
TouchModule   touchModule(TOUCH_CONFIG, &tftDriver);
Mpu6500Module mpu6500Module(MPU6500_CONFIG, &mpuWire);

// 出力モジュール
TftModule   tftModule(TFT_CONFIG, &tftDriver);
ServoModule servoModule(SERVO_CONFIG);

// モジュール配列
IModule* inputModules[] = {
    &touchModule,
    &mpu6500Module,
};
const int INPUT_COUNT = sizeof(inputModules) / sizeof(inputModules[0]);

IModule* outputModules[] = {
    &tftModule,
    &servoModule,
};
const int OUTPUT_COUNT = sizeof(outputModules) / sizeof(outputModules[0]);

// ===== ロジックフェーズ =====

void applyPattern(SystemData& data) {
    // MPU6500データをTFT表示
    if (data.mpu.isValid) {
        snprintf(data.tft.line1, sizeof(data.tft.line1),
                 "Ax:%.2f Ay:%.2f Az:%.2f  ",
                 data.mpu.accelX, data.mpu.accelY, data.mpu.accelZ);
        snprintf(data.tft.line2, sizeof(data.tft.line2),
                 "Gx:%.1f Gy:%.1f Gz:%.1f  ",
                 data.mpu.gyroX, data.mpu.gyroY, data.mpu.gyroZ);
        snprintf(data.tft.line3, sizeof(data.tft.line3),
                 "Temp: %.1f C  ", data.mpu.temperature);
    } else {
        strncpy(data.tft.line1, "MPU6500: No data      ", sizeof(data.tft.line1));
        data.tft.line2[0] = '\0';
        data.tft.line3[0] = '\0';
    }

    // タッチでサーボ制御（X座標 0-320 → 角度 0-180）
    if (data.touch.touchPressed) {
        data.servo.targetAngle = map(data.touch.touchX, 0, 320, 0, 180);
        snprintf(data.tft.line4, sizeof(data.tft.line4),
                 "Touch:X=%3d Y=%3d Sv:%3d ",
                 data.touch.touchX, data.touch.touchY, data.servo.targetAngle);
    } else {
        snprintf(data.tft.line4, sizeof(data.tft.line4),
                 "Servo: %3d deg        ", data.servo.currentAngle);
    }

    // システム情報
    snprintf(data.tft.line5, sizeof(data.tft.line5),
             "Heap:%dB PSRAM:%dB  ",
             (int)ESP.getFreeHeap(), (int)ESP.getFreePsram());
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
    Serial.begin(115200);
    Serial.println("[System] テストベンチ起動");

    // バス初期化（全モジュールのinit()より前に実行）
    mpuWire.begin(I2C_SDA_PIN, I2C_SCL_PIN);  // I2Cバス
    tftDriver.init();  // TFT_eSPIドライバ初期化（TouchModuleも共有）

    // モジュール初期化
    initModuleArray(inputModules,  INPUT_COUNT,  "Input");
    initModuleArray(outputModules, OUTPUT_COUNT, "Output");

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

    // 2. ロジックフェーズ
    applyPattern(systemData);

    // 3. 出力フェーズ
    for (int i = 0; i < OUTPUT_COUNT; i++) {
        if (outputModules[i]->enabled) {
            outputModules[i]->update(systemData);
        }
    }
}
