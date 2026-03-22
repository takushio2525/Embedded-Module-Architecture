// test_servo — サーボモーター制御モジュールのテスト
// SG90サーボの実機接続が必要（PWM出力の確認は目視）
#include <Arduino.h>
#include <unity.h>
#include "ServoModule.h"
#include "ProjectConfig.h"

static ServoModule* servo = nullptr;
static SystemData systemData;

// init()が成功すること
void test_servo_init() {
    bool result = servo->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "サーボのinit()が失敗");
}

// デフォルト角度が設定されること
void test_servo_default_angle() {
    systemData.servo = ServoData{};
    systemData.servo.targetAngle = SERVO_CONFIG.defaultAngle;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(SERVO_CONFIG.defaultAngle,
                            systemData.servo.currentAngle);
}

// 0度に移動できること
void test_servo_angle_0() {
    systemData.servo.targetAngle = 0;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(0, systemData.servo.currentAngle);
    delay(500);  // サーボが移動する時間を待つ（目視確認用）
}

// 90度に移動できること
void test_servo_angle_90() {
    systemData.servo.targetAngle = 90;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(90, systemData.servo.currentAngle);
    delay(500);
}

// 180度に移動できること
void test_servo_angle_180() {
    systemData.servo.targetAngle = 180;
    servo->update(systemData);
    TEST_ASSERT_EQUAL_UINT8(180, systemData.servo.currentAngle);
    delay(500);
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

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    servo = new ServoModule(SERVO_CONFIG);

    UNITY_BEGIN();

    RUN_TEST(test_servo_init);
    RUN_TEST(test_servo_default_angle);
    RUN_TEST(test_servo_angle_0);
    RUN_TEST(test_servo_angle_90);
    RUN_TEST(test_servo_angle_180);
    RUN_TEST(test_servo_no_change);
    RUN_TEST(test_servo_clamp);

    UNITY_END();

    delete servo;
}

void loop() {}
