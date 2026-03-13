// main.cpp — エントリーポイント
#include <Arduino.h>
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// システムデータ
SystemData systemData;

// 出力モジュール
LedModule ledModule(LED_CONFIG);

// 出力モジュール配列
IModule<SystemData>* outputModules[] = {
    &ledModule,
};
const int OUTPUT_COUNT = sizeof(outputModules) / sizeof(outputModules[0]);

// ロジックフェーズ: Lチカ（1秒ごとにトグル）
ModuleTimer blinkTimer;

void applyPattern(SystemData& data) {
    if (blinkTimer.getNowTime() >= 1000) {
        blinkTimer.setTime();
        data.led.state = !data.led.state;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("[System] 起動");

    // 全出力モジュールを初期化
    const int MAX_RETRY = 3;
    for (int i = 0; i < OUTPUT_COUNT; i++) {
        bool success = false;
        for (int r = 0; r < MAX_RETRY; r++) {
            if (outputModules[i]->init()) { success = true; break; }
            delay(100);
        }
        if (!success) {
            Serial.printf("[System] Output Module %d: init failed, disabled\n", i);
            outputModules[i]->enabled = false;
        }
    }

    blinkTimer.setTime();
}

void loop() {
    // 1. 入力フェーズ（現在は入力モジュールなし）

    // 2. ロジックフェーズ
    applyPattern(systemData);

    // 3. 出力フェーズ
    for (int i = 0; i < OUTPUT_COUNT; i++) {
        if (outputModules[i]->enabled) {
            outputModules[i]->update(systemData);
        }
    }
}
