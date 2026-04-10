// ButtonModule.h — ボタン入力モジュール（デバウンス処理付き）
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// --- Config構造体 ---
struct ButtonConfig {
    uint8_t       pin;              // ボタンピン
    bool          activeLow;        // true: LOW=押下（プルアップ）、false: HIGH=押下（プルダウン）
    unsigned long debounceMs;       // デバウンス安定時間 [ms]（例: 30）
};

// --- Data構造体 ---
struct ButtonData {
    bool pressed      = false;  // デバウンス後の押下状態
    bool justPressed  = false;  // 今ループで押下された（立ち上がりエッジ）
    bool justReleased = false;  // 今ループで離された（立ち下がりエッジ）
};

// --- モジュール実装 ---
struct SystemData;

class ButtonModule : public IModule {
private:
    ButtonConfig _config;
    ModuleTimer  _debounceTimer;
    bool         _lastRaw       = false;  // 前回の生入力値
    bool         _stableState   = false;  // デバウンス確定後の状態
    bool         _prevStable    = false;  // 前ループの確定状態（エッジ検出用）

public:
    ButtonModule(const ButtonConfig& config);
    bool init() override;
    void updateInput(SystemData& data) override;
};
