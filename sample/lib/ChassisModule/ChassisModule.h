// ChassisModule.h — 4輪統合足回り制御モジュール（機能統合系）
// 内部でDriveMotorModuleを4つ保持し、ベクトル合成で足回りを制御する
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "DriveMotorModule.h"

static constexpr int CHASSIS_MOTOR_COUNT = 4;

// ===== ホイールパターン =====
enum class WheelPattern : uint8_t {
    OMNI,     // オムニホイール（全方向移動 + 回転）
    MECANUM,  // メカナムホイール（全方向移動 + 回転）
    FOURWD,   // 4輪駆動（前後 + 回転のみ、横移動不可）
};

// ===== 設定 =====
struct ChassisConfig {
    DriveMotorConfig motors[CHASSIS_MOTOR_COUNT];  // [0]:左前, [1]:右前, [2]:左後, [3]:右後
    WheelPattern     wheelPattern;                 // ホイールパターン
    float            maxOutputPercent;              // 最大出力割合 [%]（例: 90.0）
};

// ===== 共有データ =====
struct ChassisData {
    // 入力（ロジックフェーズで書き込む）
    float forwardSpeed  = 0.0f;  // 前進速度 [-100 ~ 100]（正:前進, 負:後退）
    float lateralSpeed  = 0.0f;  // 横移動速度 [-100 ~ 100]（正:右, 負:左）
    float rotationSpeed = 0.0f;  // 回転速度 [-100 ~ 100]（正:右回転, 負:左回転）

    // 出力（updateOutput後に更新、デバッグ用）
    float motorOutputs[CHASSIS_MOTOR_COUNT] = {};  // 各モーター実出力値
};

// ===== モジュール =====
struct SystemData;

class ChassisModule : public IModule {
private:
    ChassisConfig    _config;
    DriveMotorModule _motorFL;  // 左前
    DriveMotorModule _motorFR;  // 右前
    DriveMotorModule _motorRL;  // 左後
    DriveMotorModule _motorRR;  // 右後
    DriveMotorModule* _motors[CHASSIS_MOTOR_COUNT];

    // ホイールパターンに応じた方向行列を取得
    const float (*_getMatrix() const)[CHASSIS_MOTOR_COUNT];

public:
    ChassisModule(const ChassisConfig& config);
    bool init() override;
    void updateOutput(SystemData& data) override;
    void deinit() override;
};
