// ServoModule.h — サーボモーター制御モジュール
#pragma once
#include <Arduino.h>
#include "IModule.h"

// ===== 設定 =====
struct ServoConfig {
    uint8_t  pin;            // サーボ信号ピン
    uint8_t  pwmChannel;     // LEDCチャネル番号（0-15、他と重複しないこと）
    uint16_t minPulseUs;     // 最小パルス幅 [μs]（例: 500）
    uint16_t maxPulseUs;     // 最大パルス幅 [μs]（例: 2500）
    uint8_t  defaultAngle;   // 初期角度 [度]（0-180）
};

// ===== 共有データ =====
struct ServoData {
    uint8_t targetAngle  = 90;  // 目標角度 [度]（0-180）
    uint8_t currentAngle = 90;  // 現在の角度 [度]（0-180）
};

// ===== モジュール =====
struct SystemData;

class ServoModule : public IModule {
public:
    ServoModule(const ServoConfig& config);
    bool init() override;
    void updateOutput(SystemData& data) override;

private:
    ServoConfig _config;

    // 角度からPWMデューティ値を計算
    uint32_t _angleToDuty(uint8_t angle) const;
};
