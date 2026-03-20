// BatteryModule.h — バッテリー電圧監視モジュール（ADC + 移動平均フィルタ）
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// --- 定数 ---
static constexpr uint8_t BATTERY_FILTER_SIZE = 16;  // 移動平均のサンプル数

// --- Config構造体 ---
struct BatteryConfig {
    uint8_t       adcPin;          // ADC入力ピン
    float         voltageDivider;  // 分圧比（例: 2.0 = 1/2分圧）
    float         adcRefVoltage;   // ADC基準電圧 [V]（ESP32: 3.3）
    uint16_t      adcResolution;   // ADC分解能（ESP32: 4095 = 12bit）
    unsigned long sampleIntervalMs;// サンプリング間隔 [ms]
    float         lowVoltageThreshold; // 低電圧警告閾値 [V]
};

// --- Data構造体 ---
struct BatteryData {
    float voltage    = 0.0f;   // フィルタ済み電圧 [V]
    float rawVoltage = 0.0f;   // 最新の生電圧 [V]
    bool  isValid    = false;  // サンプル数がフィルタサイズに達したらtrue
    bool  isLow      = false;  // 低電圧警告フラグ
};

// --- モジュール実装 ---
struct SystemData;

class BatteryModule : public IModule {
private:
    BatteryConfig _config;
    ModuleTimer   _sampleTimer;

    // 移動平均フィルタ用リングバッファ
    float   _samples[BATTERY_FILTER_SIZE] = {};
    uint8_t _sampleIndex = 0;
    uint8_t _sampleCount = 0;  // 蓄積済みサンプル数（最大 BATTERY_FILTER_SIZE）

    // ADC生値を電圧に変換
    float _adcToVoltage(uint16_t adcValue) const;
    // リングバッファの平均を計算
    float _calcAverage() const;

public:
    BatteryModule(const BatteryConfig& config);
    bool init() override;
    void update(SystemData& data) override;
};
