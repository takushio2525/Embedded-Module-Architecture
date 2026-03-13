// ProjectConfig.h — プロジェクト固有設定の集約
#pragma once
#include "LedModule.h"

// ===== モジュールConfig（ピンアサイン・設定値） =====
const LedConfig LED_CONFIG = { .ledPin = 2 };  // GPIO2: オンボードLED

// ===== システムデータ =====
struct SystemData {
    LedData led;
};
