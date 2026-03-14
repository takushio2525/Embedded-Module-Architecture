// main.cpp — エントリーポイント
// 3フェーズ実行モデル: 入力 → ロジック → 出力
#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// バスインスタンス（グローバルスコープで生成）
static TwoWire  mpuWire  = TwoWire(0);   // MPU6500用I2C
static TFT_eSPI tftDriver;               // TFT LCD + タッチ共有ドライバ

// システムデータ
SystemData systemData;

// ===== モジュールインスタンス =====

// 入力モジュール
TouchModule   touchModule(TOUCH_CONFIG, &tftDriver);
Mpu6500Module mpu6500Module(MPU6500_CONFIG, &mpuWire);
CameraModule  cameraModule(CAMERA_CONFIG);

// 出力モジュール
LedModule  ledModule(LED_CONFIG);
TftModule  tftModule(TFT_CONFIG, &tftDriver);

// モジュール配列
IModule* inputModules[] = {
    &touchModule,
    &mpu6500Module,
    &cameraModule,
};
const int INPUT_COUNT = sizeof(inputModules) / sizeof(inputModules[0]);

IModule* outputModules[] = {
    &ledModule,
    &tftModule,
};
const int OUTPUT_COUNT = sizeof(outputModules) / sizeof(outputModules[0]);

// ===== ロジックフェーズ =====

ModuleTimer blinkTimer;

void applyPattern(SystemData& data) {
    // Lチカ（1秒ごとにトグル）
    if (blinkTimer.getNowTime() >= 1000) {
        blinkTimer.setTime();
        data.led.state = !data.led.state;
    }

    // MPU-6500データをTFT表示用にフォーマット
    if (data.mpu.isValid) {
        snprintf(data.tft.line1, sizeof(data.tft.line1),
                 "Ax:%.2f Ay:%.2f Az:%.2f",
                 data.mpu.accelX, data.mpu.accelY, data.mpu.accelZ);
        snprintf(data.tft.line2, sizeof(data.tft.line2),
                 "Gx:%.1f Gy:%.1f Gz:%.1f",
                 data.mpu.gyroX, data.mpu.gyroY, data.mpu.gyroZ);
        snprintf(data.tft.line3, sizeof(data.tft.line3),
                 "Temp: %.1f C", data.mpu.temperature);
    } else {
        strncpy(data.tft.line1, "MPU: No data", sizeof(data.tft.line1));
    }

    // カメラ情報をTFT表示用にフォーマット
    if (data.camera.isValid && data.camera.frameReady) {
        snprintf(data.tft.line4, sizeof(data.tft.line4),
                 "CAM: %dx%d (%d B)",
                 data.camera.width, data.camera.height, (int)data.camera.frameSize);
    } else {
        strncpy(data.tft.line4, "CAM: No frame", sizeof(data.tft.line4));
    }

    // タッチ入力のデバッグ出力（タッチ開始時のみ）
    static bool lastTouched = false;
    if (data.touch.touchPressed && !lastTouched) {
        Serial.printf("[Logic] Touch: (%d, %d)\n", data.touch.touchX, data.touch.touchY);
    }
    lastTouched = data.touch.touchPressed;
}

// ===== セットアップ =====

static void initModuleArray(IModule** modules, int count, const char* label) {
    const int MAX_RETRY = 3;
    for (int i = 0; i < count; i++) {
        bool success = false;
        for (int r = 0; r < MAX_RETRY; r++) {
            if (modules[i]->init()) { success = true; break; }
            delay(100);
        }
        if (!success) {
            Serial.printf("[System] %s Module %d: init failed, disabled\n", label, i);
            modules[i]->enabled = false;
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("[System] 起動");

    // バス初期化（全モジュールのinit()より前に実行）
    // TFT_eSPIはTftModule::init()内で tft->init() を呼ぶことで初期化される
    mpuWire.begin(MPU6500_CONFIG.sdaPin, MPU6500_CONFIG.sclPin);

    // モジュール初期化（TftModuleのinit()でTFT_eSPIが初期化されるため先に実行）
    initModuleArray(outputModules, OUTPUT_COUNT, "Output");
    initModuleArray(inputModules,  INPUT_COUNT,  "Input");

    blinkTimer.setTime();
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

    // カメラフレームバッファは logicフェーズ終了時に解放
    cameraModule.releaseFrame();

    // 3. 出力フェーズ
    for (int i = 0; i < OUTPUT_COUNT; i++) {
        if (outputModules[i]->enabled) {
            outputModules[i]->update(systemData);
        }
    }
}
