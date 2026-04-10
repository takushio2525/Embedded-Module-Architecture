// LedModule.h — LED制御モジュール
#pragma once
#include <Arduino.h>
#include "IModule.h"

// --- Config構造体 ---
struct LedConfig {
    uint8_t ledPin;  // LED出力ピン
};

// --- Data構造体 ---
struct LedData {
    bool state = false;
    uint8_t brightness = 0;
};

// SystemDataの前方宣言はIModule.h内で行われている

// --- モジュール実装 ---
class LedModule : public IModule {
private:
    LedConfig _config;

public:
    LedModule(const LedConfig& config);
    bool init() override;
    void updateOutput(SystemData& data) override;
};
