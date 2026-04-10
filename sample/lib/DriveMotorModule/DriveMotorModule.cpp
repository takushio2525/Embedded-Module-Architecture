// DriveMotorModule.cpp — DCモーター単体制御モジュール
#include "DriveMotorModule.h"

static constexpr uint8_t  MOTOR_PWM_RES  = 16;
static constexpr uint32_t MOTOR_DUTY_MAX = 65535;  // 2^16 - 1

DriveMotorModule::DriveMotorModule(const DriveMotorConfig& config)
    : _config(config) {}

bool DriveMotorModule::init() {
    // 方向制御ピン
    pinMode(_config.in1Pin, OUTPUT);
    pinMode(_config.in2Pin, OUTPUT);
    digitalWrite(_config.in1Pin, LOW);
    digitalWrite(_config.in2Pin, LOW);

    // PWMピン（ESP32 Arduino Core v2.x）
    ledcSetup(_config.pwmChannel, _config.pwmFreqHz, MOTOR_PWM_RES);
    ledcAttachPin(_config.pwmPin, _config.pwmChannel);
    ledcWrite(_config.pwmChannel, 0);

    Serial.printf("[DriveMotor] init OK (IN1=%d, IN2=%d, PWM=%d)\n",
                  _config.in1Pin, _config.in2Pin, _config.pwmPin);
    return true;
}

void DriveMotorModule::updateOutput(SystemData& /* data */) {
    // このモジュールはChassisModule等の統合モジュールが内部で保持し、
    // drive()メソッドを直接呼び出す設計のため、updateOutput()は空実装。
    // outputModules[]に直接登録して使うモジュールではない。
    //
    // 単体モーターをoutputModules[]で使いたい場合は、
    // DriveMotorDataをSystemDataに追加し、以下のように実装する:
    //   void DriveMotorModule::updateOutput(SystemData& data) {
    //       _applyMotor(data.driveMotor.power);
    //   }
}

void DriveMotorModule::drive(float power) {
    _applyMotor(power);
}

void DriveMotorModule::_applyMotor(float power) {
    // 現在の方向を判定
    int8_t direction = 0;
    if (power > _config.minPowerThreshold) {
        direction = 1;
    } else if (power < -_config.minPowerThreshold) {
        direction = -1;
    }

    // 逆転検出: 正転↔逆転の場合は1フレームブレーキを挿入
    if ((direction == 1 && _prevDirection == -1) ||
        (direction == -1 && _prevDirection == 1)) {
        // ブレーキ（IN1=HIGH, IN2=HIGH で短絡ブレーキ）
        digitalWrite(_config.in1Pin, HIGH);
        digitalWrite(_config.in2Pin, HIGH);
        ledcWrite(_config.pwmChannel, MOTOR_DUTY_MAX);
        _prevDirection = 0;
        return;  // 今フレームはブレーキで終了、次フレームで方向転換
    }

    // 停止
    if (direction == 0) {
        digitalWrite(_config.in1Pin, LOW);
        digitalWrite(_config.in2Pin, LOW);
        ledcWrite(_config.pwmChannel, 0);
        _prevDirection = 0;
        return;
    }

    // 方向設定
    if (direction == 1) {
        digitalWrite(_config.in1Pin, HIGH);
        digitalWrite(_config.in2Pin, LOW);
    } else {
        digitalWrite(_config.in1Pin, LOW);
        digitalWrite(_config.in2Pin, HIGH);
    }

    // PWM出力（powerの絶対値をデューティに変換）
    float absPower = fabsf(power);
    if (absPower > 100.0f) absPower = 100.0f;
    uint32_t duty = (uint32_t)(absPower / 100.0f * MOTOR_DUTY_MAX);
    ledcWrite(_config.pwmChannel, duty);

    _prevDirection = direction;
}
