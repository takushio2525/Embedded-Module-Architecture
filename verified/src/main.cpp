// main.cpp — テストベンチ エントリーポイント
// 3フェーズ実行モデル: 入力 → ロジック → 出力
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// バスインスタンス（グローバルスコープで生成）
static TFT_eSPI tftDriver;  // TFT LCD + タッチ共有ドライバ

// システムデータ
SystemData systemData;

// ===== モジュールインスタンス =====

// 入力モジュール
TouchModule touchModule(TOUCH_CONFIG, &tftDriver);

// 出力モジュール
TftModule tftModule(TFT_CONFIG, &tftDriver);

// モジュール配列
IModule* inputModules[] = {
    &touchModule,
};
const int INPUT_COUNT = sizeof(inputModules) / sizeof(inputModules[0]);

IModule* outputModules[] = {
    &tftModule,
};
const int OUTPUT_COUNT = sizeof(outputModules) / sizeof(outputModules[0]);

// ===== ロジックフェーズ =====

void applyPattern(SystemData& data) {
    // タッチ座標をTFTに表示
    if (data.touch.touchPressed) {
        snprintf(data.tft.line1, sizeof(data.tft.line1),
                 "Touch: X=%3d  Y=%3d  ", data.touch.touchX, data.touch.touchY);
    } else {
        strncpy(data.tft.line1, "Touch: ---            ", sizeof(data.tft.line1));
    }

    // システム情報
    snprintf(data.tft.line3, sizeof(data.tft.line3),
             "Heap: %d B  ", (int)ESP.getFreeHeap());
    snprintf(data.tft.line4, sizeof(data.tft.line4),
             "PSRAM: %d B  ", (int)ESP.getFreePsram());
    snprintf(data.tft.line5, sizeof(data.tft.line5),
             "Uptime: %lu s  ", millis() / 1000);
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
