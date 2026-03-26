// servo_sweep — サーボモーター スイープ動作テスト
// 3フェーズモデルでModuleTimerを使い0→180→0度をスイープする
#include <Arduino.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== モジュール・データ =====
ServoModule servo(SERVO_CONFIG);
SystemData systemData;

// スイープ制御
static ModuleTimer stepTimer;
static constexpr uint32_t STEP_INTERVAL_MS = 50;  // 50msごとに角度変更
static constexpr uint8_t STEP_DEGREE = 5;         // 1ステップ5度
static int8_t direction = 1;                       // 1=増加, -1=減少

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    Serial.println("[ServoTest] 起動");

    // モジュール初期化
    if (!servo.init()) {
        Serial.println("[ServoTest] init失敗");
        servo.enabled = false;
    }

    // 初期角度を0度に設定
    systemData.servo.targetAngle = 0;

    stepTimer.setTime();
    Serial.println("[ServoTest] スイープ開始 (0-180度)");
}

void loop() {
    // 1. 入力フェーズ（なし）

    // 2. ロジックフェーズ
    if (stepTimer.getNowTime() >= STEP_INTERVAL_MS) {
        stepTimer.setTime();

        int16_t angle = systemData.servo.targetAngle + direction * STEP_DEGREE;

        // 端に到達したら方向反転
        if (angle >= 180) {
            angle = 180;
            direction = -1;
        } else if (angle <= 0) {
            angle = 0;
            direction = 1;
        }

        systemData.servo.targetAngle = (uint8_t)angle;
        Serial.printf("Servo: %3d deg\n", systemData.servo.targetAngle);
    }

    // 3. 出力フェーズ
    if (servo.enabled) {
        servo.update(systemData);
    }
}
