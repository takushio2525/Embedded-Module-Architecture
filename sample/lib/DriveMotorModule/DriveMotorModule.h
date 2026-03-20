// DriveMotorModule.h — DCモーター単体制御モジュール
// IN1/IN2で方向制御、PWMで速度制御するHブリッジドライバ用
#pragma once
#include <Arduino.h>
#include "IModule.h"

// ===== 設定 =====
struct DriveMotorConfig {
    uint8_t  in1Pin;             // 方向制御ピン1
    uint8_t  in2Pin;             // 方向制御ピン2
    uint8_t  pwmPin;             // PWM速度制御ピン
    uint8_t  pwmChannel;         // LEDCチャネル番号（0-15、他と重複しないこと）
    uint32_t pwmFreqHz;          // PWM周波数 [Hz]（例: 1220）
    float    minPowerThreshold;  // 最小出力閾値 [%]（これ以下は停止扱い）
};

// ===== 共有データ =====
struct DriveMotorData {
    float power = 0.0f;  // 目標出力 [-100.0 ~ 100.0]（正:正転, 負:逆転）
};

// ===== モジュール =====
// 注意: このモジュールはChassisModule等の統合モジュールが内部で保持し、
// drive()を直接呼ぶ前提の設計。outputModules[]には登録しない。
struct SystemData;

class DriveMotorModule : public IModule {
private:
    DriveMotorConfig _config;
    int8_t _prevDirection = 0;  // 前回方向（-1:逆転, 0:停止, 1:正転）

    // 方向とPWMを設定してモーターを駆動
    void _applyMotor(float power);

public:
    DriveMotorModule(const DriveMotorConfig& config);
    bool init() override;
    void update(SystemData& data) override;

    // ChassisModuleなど外部から直接制御する用（SystemData非経由）
    void drive(float power);
};
