// test_servo — サーボモーター制御モジュールのテスト
// SG90サーボの実機接続が必要
// 各角度で実際の動作をシリアル経由で確認する
#include <Arduino.h>
#include <unity.h>
#include "ServoModule.h"
#include "ProjectConfig.h"
#include "../test_utils.h"

static ServoModule* servo = nullptr;
static SystemData systemData;

// init()が成功すること
void test_servo_init() {
    bool result = servo->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "サーボのinit()が失敗");
}

// デフォルト角度（90度）に移動 → ユーザー確認
void test_servo_default_angle() {
    systemData.servo = ServoData{};
    systemData.servo.targetAngle = SERVO_CONFIG.defaultAngle;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(SERVO_CONFIG.defaultAngle,
                            systemData.servo.currentAngle);

    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm("サーボが90度（中央）にありますか？"),
        "デフォルト角度の目視確認に失敗");
}

// 0度に移動 → ユーザー確認
void test_servo_angle_0() {
    systemData.servo.targetAngle = 0;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(0, systemData.servo.currentAngle);
    delay(500);  // サーボ移動待ち

    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm("サーボが0度（端）に移動しましたか？"),
        "0度移動の目視確認に失敗");
}

// 180度に移動 → ユーザー確認
void test_servo_angle_180() {
    systemData.servo.targetAngle = 180;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(180, systemData.servo.currentAngle);
    delay(500);  // サーボ移動待ち

    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm("サーボが180度（反対側の端）に移動しましたか？"),
        "180度移動の目視確認に失敗");
}

// 90度に戻る → ユーザー確認
void test_servo_angle_90() {
    systemData.servo.targetAngle = 90;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(90, systemData.servo.currentAngle);
    delay(500);

    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm("サーボが90度（中央）に戻りましたか？"),
        "90度移動の目視確認に失敗");
}

// 同じ角度を再設定してもcurrentAngleが変わらないこと
void test_servo_no_change() {
    systemData.servo.targetAngle  = 90;
    systemData.servo.currentAngle = 90;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(90, systemData.servo.currentAngle);
}

// 範囲外の値がconstrain(0-180)されること
void test_servo_clamp() {
    // 255は180にクランプされるはず
    systemData.servo.targetAngle = 255;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(180, systemData.servo.currentAngle);
}

// スイープテスト（0→180→0を連続動作）→ ユーザー確認
void test_servo_sweep() {
    Serial.println("[Servo] スイープテスト: 0→180→0");
    // 0→180
    for (int angle = 0; angle <= 180; angle += 10) {
        systemData.servo.targetAngle = angle;
        servo->update(systemData);
        delay(30);
    }
    // 180→0
    for (int angle = 180; angle >= 0; angle -= 10) {
        systemData.servo.targetAngle = angle;
        servo->update(systemData);
        delay(30);
    }
    // 中央に戻す
    systemData.servo.targetAngle = 90;
    servo->update(systemData);

    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm("サーボが0→180→0とスムーズにスイープしましたか？"),
        "スイープ動作の目視確認に失敗");
}

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    servo = new ServoModule(SERVO_CONFIG);

    Serial.println();
    Serial.println("========================================");
    Serial.println(" サーボ テスト（対話型）");
    Serial.println(" 各角度でサーボの動きを確認し、");
    Serial.println(" シリアルに 'y' を送信してください。");
    Serial.println("========================================");

    UNITY_BEGIN();

    RUN_TEST(test_servo_init);
    RUN_TEST(test_servo_default_angle);
    RUN_TEST(test_servo_angle_0);
    RUN_TEST(test_servo_angle_180);
    RUN_TEST(test_servo_angle_90);
    RUN_TEST(test_servo_no_change);
    RUN_TEST(test_servo_clamp);
    RUN_TEST(test_servo_sweep);

    UNITY_END();

    delete servo;
}

void loop() {}
