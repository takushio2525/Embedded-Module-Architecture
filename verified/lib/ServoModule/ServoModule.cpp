// ServoModule.cpp — サーボモーター制御モジュール
#include "ServoModule.h"
#include "SystemData.h"

// PWM設定: 50Hz（周期20ms）、16bit分解能（0-65535）
static constexpr uint32_t SERVO_PWM_FREQ = 50;
static constexpr uint8_t  SERVO_PWM_RES  = 16;
static constexpr uint32_t SERVO_PERIOD_US = 20000;  // 1/50Hz = 20ms
static constexpr uint32_t SERVO_DUTY_MAX  = 65535;   // 2^16 - 1

ServoModule::ServoModule(const ServoConfig& config) : _config(config) {}

bool ServoModule::init() {
    // ESP32 Arduino Core v3.x: ledcAttach（setup + attachが統合）
    ledcAttach(_config.pin, SERVO_PWM_FREQ, SERVO_PWM_RES);

    // 初期角度にセット
    ledcWrite(_config.pin, _angleToDuty(_config.defaultAngle));
    Serial.printf("[Servo] init OK (pin=%d, angle=%d)\n",
                  _config.pin, _config.defaultAngle);
    return true;
}

void ServoModule::update(SystemData& data) {
    // 目標角度を0-180に制限
    uint8_t target = constrain(data.servo.targetAngle, 0, 180);

    // 変化がある場合のみPWM更新
    if (target != data.servo.currentAngle) {
        ledcWrite(_config.pin, _angleToDuty(target));
        data.servo.currentAngle = target;
    }
}

uint32_t ServoModule::_angleToDuty(uint8_t angle) const {
    // 角度 → パルス幅 [μs]
    uint32_t pulseUs = map(angle, 0, 180, _config.minPulseUs, _config.maxPulseUs);
    // パルス幅 → デューティ値（16bit）
    return (uint32_t)((uint64_t)pulseUs * SERVO_DUTY_MAX / SERVO_PERIOD_US);
}
