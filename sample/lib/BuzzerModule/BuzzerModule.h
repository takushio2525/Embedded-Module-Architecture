// BuzzerModule.h — ブザー出力モジュール（PWMトーン再生 + deinit()リソース解放）
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// --- Config構造体 ---
struct BuzzerConfig {
    uint8_t pin;           // ブザー出力ピン
    uint8_t pwmChannel;    // LEDCチャネル番号（0-15）
};

// --- Data構造体 ---
struct BuzzerData {
    uint16_t frequency = 0;       // 再生周波数 [Hz]（0=消音）
    unsigned long durationMs = 0; // 再生時間 [ms]（0=無制限）
    bool     requestPlay = false; // ロジックフェーズからの再生リクエスト
};

// --- モジュール実装 ---
struct SystemData;

class BuzzerModule : public IModule {
private:
    BuzzerConfig _config;
    ModuleTimer  _durationTimer;
    bool         _playing = false;     // 現在再生中か
    bool         _timed   = false;     // 時間指定ありか

    void _toneOn(uint16_t freq);
    void _toneOff();

public:
    BuzzerModule(const BuzzerConfig& config);
    bool init() override;
    void updateOutput(SystemData& data) override;
    void deinit() override;
};
