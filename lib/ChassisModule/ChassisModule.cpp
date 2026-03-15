// ChassisModule.cpp — 4輪統合足回り制御モジュール
#include "ChassisModule.h"
#include "SystemData.h"

// ===== 方向行列 =====
// [方向: 前進/横移動/回転][モーター: 左前/右前/左後/右後]
// 符号: 正=正転（前進方向）、負=逆転

// オムニホイール
static const float OMNI_MATRIX[3][CHASSIS_MOTOR_COUNT] = {
    {-1.0f,  1.0f, -1.0f,  1.0f},  // 前進
    { 1.0f,  1.0f, -1.0f, -1.0f},  // 横移動（右が正）
    {-1.0f, -1.0f, -1.0f, -1.0f},  // 回転（右回転が正）
};

// メカナムホイール
static const float MECANUM_MATRIX[3][CHASSIS_MOTOR_COUNT] = {
    {-1.0f,  1.0f, -1.0f,  1.0f},  // 前進
    { 1.0f, -1.0f, -1.0f,  1.0f},  // 横移動（右が正）
    {-1.0f, -1.0f, -1.0f, -1.0f},  // 回転（右回転が正）
};

// 4WD（横移動なし）
static const float FOURWD_MATRIX[3][CHASSIS_MOTOR_COUNT] = {
    {-1.0f,  1.0f, -1.0f,  1.0f},  // 前進
    { 0.0f,  0.0f,  0.0f,  0.0f},  // 横移動（不可）
    {-1.0f, -1.0f, -1.0f, -1.0f},  // 回転（右回転が正）
};

// ===== コンストラクタ =====

ChassisModule::ChassisModule(const ChassisConfig& config)
    : _config(config),
      _motorFL(config.motors[0]),
      _motorFR(config.motors[1]),
      _motorRL(config.motors[2]),
      _motorRR(config.motors[3])
{
    _motors[0] = &_motorFL;
    _motors[1] = &_motorFR;
    _motors[2] = &_motorRL;
    _motors[3] = &_motorRR;
}

// ===== 初期化 =====

bool ChassisModule::init() {
    const char* motorNames[] = {"FL", "FR", "RL", "RR"};

    for (int i = 0; i < CHASSIS_MOTOR_COUNT; i++) {
        if (!_motors[i]->init()) {
            Serial.printf("[Chassis] motor %s init failed\n", motorNames[i]);
            return false;
        }
    }

    Serial.println("[Chassis] init OK (4 motors)");
    return true;
}

// ===== 更新 =====

void ChassisModule::update(SystemData& data) {
    // 入力速度を取得
    float speeds[3] = {
        data.chassis.forwardSpeed,
        data.chassis.lateralSpeed,
        data.chassis.rotationSpeed,
    };

    // ベクトル合成: 方向行列 × 入力速度 → 各モーター出力
    float motorPowers[CHASSIS_MOTOR_COUNT] = {};
    const float (*matrix)[CHASSIS_MOTOR_COUNT] = _getMatrix();

    for (int dir = 0; dir < 3; dir++) {
        for (int m = 0; m < CHASSIS_MOTOR_COUNT; m++) {
            motorPowers[m] += matrix[dir][m] * speeds[dir];
        }
    }

    // リミッタ: 最大絶対値が100を超えたら全体を等比縮小
    float maxAbs = 0.0f;
    for (int m = 0; m < CHASSIS_MOTOR_COUNT; m++) {
        float absVal = fabsf(motorPowers[m]);
        if (absVal > maxAbs) maxAbs = absVal;
    }
    if (maxAbs > 100.0f) {
        float scale = 100.0f / maxAbs;
        for (int m = 0; m < CHASSIS_MOTOR_COUNT; m++) {
            motorPowers[m] *= scale;
        }
    }

    // 最大出力割合でスケーリング & 各モーター駆動
    float outputScale = _config.maxOutputPercent / 100.0f;
    for (int m = 0; m < CHASSIS_MOTOR_COUNT; m++) {
        motorPowers[m] *= outputScale;
        _motors[m]->drive(motorPowers[m]);
        data.chassis.motorOutputs[m] = motorPowers[m];
    }
}

// ===== 終了処理 =====

void ChassisModule::deinit() {
    // 全モーター停止
    for (int m = 0; m < CHASSIS_MOTOR_COUNT; m++) {
        _motors[m]->drive(0.0f);
    }
    Serial.println("[Chassis] deinit: all motors stopped");
}

// ===== 内部関数 =====

const float (*ChassisModule::_getMatrix() const)[CHASSIS_MOTOR_COUNT] {
    switch (_config.wheelPattern) {
        case WheelPattern::MECANUM: return MECANUM_MATRIX;
        case WheelPattern::FOURWD:  return FOURWD_MATRIX;
        case WheelPattern::OMNI:
        default:                    return OMNI_MATRIX;
    }
}
