// main.cpp — テストベンチ エントリーポイント
// 3フェーズ実行モデル: 入力 → ロジック → 出力
#include <Arduino.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// システムデータ
SystemData systemData;

// ===== モジュールインスタンス =====
// Phase 2以降で追加

// モジュール配列（Phase 2以降で追加）
const int INPUT_COUNT  = 0;
const int OUTPUT_COUNT = 0;

// ===== ロジックフェーズ =====

void applyPattern(SystemData& data) {
    // Phase 2以降でテストモード切替ロジックを実装
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
    // Phase 2: SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    // Phase 3: Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // モジュール初期化（Phase 2以降で有効化）
    // initModuleArray(inputModules,  INPUT_COUNT,  "Input");
    // initModuleArray(outputModules, OUTPUT_COUNT, "Output");

    Serial.println("[System] 起動完了");
}

// ===== メインループ =====

void loop() {
    // 1. 入力フェーズ（Phase 2以降で有効化）

    // 2. ロジックフェーズ
    applyPattern(systemData);

    // 3. 出力フェーズ（Phase 2以降で有効化）
}
