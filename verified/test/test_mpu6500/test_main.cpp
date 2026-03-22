// test_mpu6500 — MPU-6500 IMUセンサーモジュールのテスト
// I2Cバス経由で実機接続が必要
#include <Arduino.h>
#include <unity.h>
#include <Wire.h>
#include "Mpu6500Module.h"
#include "ProjectConfig.h"

static TwoWire testWire = TwoWire(0);
static Mpu6500Module* mpu = nullptr;
static SystemData systemData;

// I2Cバス上でMPU6500が応答すること
void test_mpu6500_i2c_scan() {
    testWire.beginTransmission(0x68);
    uint8_t error = testWire.endTransmission();
    TEST_ASSERT_EQUAL_MESSAGE(0, error,
        "MPU6500がI2Cアドレス0x68で応答しない");
}

// WHO_AM_Iレジスタが0x70を返すこと
void test_mpu6500_who_am_i() {
    testWire.beginTransmission(0x68);
    testWire.write(0x75);  // WHO_AM_I レジスタ
    testWire.endTransmission(false);
    testWire.requestFrom((uint8_t)0x68, (uint8_t)1);
    TEST_ASSERT_TRUE_MESSAGE(testWire.available(), "WHO_AM_Iの読み取りに失敗");
    uint8_t whoAmI = testWire.read();
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x70, whoAmI,
        "WHO_AM_I値が不一致（MPU6500=0x70）");
}

// init()が成功すること
void test_mpu6500_init() {
    bool result = mpu->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "MPU6500のinit()が失敗");
    TEST_ASSERT_TRUE_MESSAGE(mpu->enabled, "init成功後にenabledがfalse");
}

// update()でデータが取得できること
void test_mpu6500_update_valid() {
    // init済みの前提
    systemData.mpu = Mpu6500Data{};  // リセット
    delay(50);  // サンプリング間隔待ち
    mpu->update(systemData);
    TEST_ASSERT_TRUE_MESSAGE(systemData.mpu.isValid,
        "update()後にisValidがfalse");
}

// 加速度値が妥当な範囲であること（静止状態で約1g）
void test_mpu6500_accel_range() {
    systemData.mpu = Mpu6500Data{};
    delay(50);
    mpu->update(systemData);
    TEST_ASSERT_TRUE(systemData.mpu.isValid);

    // 各軸±4g以内（静止でも配置によって各軸の値は変わる）
    TEST_ASSERT_FLOAT_WITHIN(4.0f, 0.0f, systemData.mpu.accelX);
    TEST_ASSERT_FLOAT_WITHIN(4.0f, 0.0f, systemData.mpu.accelY);
    TEST_ASSERT_FLOAT_WITHIN(4.0f, 0.0f, systemData.mpu.accelZ);

    // 合成加速度は0.5g〜1.5gの範囲（静止状態）
    float magnitude = sqrtf(
        systemData.mpu.accelX * systemData.mpu.accelX +
        systemData.mpu.accelY * systemData.mpu.accelY +
        systemData.mpu.accelZ * systemData.mpu.accelZ);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.5f, 1.0f, magnitude,
        "静止状態の合成加速度が1g付近でない");
}

// ジャイロ値が妥当な範囲であること（静止状態で約0°/s）
void test_mpu6500_gyro_range() {
    systemData.mpu = Mpu6500Data{};
    delay(50);
    mpu->update(systemData);
    TEST_ASSERT_TRUE(systemData.mpu.isValid);

    // 静止状態: 各軸±30°/s以内（ドリフト考慮）
    TEST_ASSERT_FLOAT_WITHIN(30.0f, 0.0f, systemData.mpu.gyroX);
    TEST_ASSERT_FLOAT_WITHIN(30.0f, 0.0f, systemData.mpu.gyroY);
    TEST_ASSERT_FLOAT_WITHIN(30.0f, 0.0f, systemData.mpu.gyroZ);
}

// 温度が妥当な範囲であること
void test_mpu6500_temperature() {
    systemData.mpu = Mpu6500Data{};
    delay(50);
    mpu->update(systemData);
    TEST_ASSERT_TRUE(systemData.mpu.isValid);

    // -10°C〜60°Cの範囲
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(35.0f, 25.0f, systemData.mpu.temperature,
        "温度が妥当な範囲外（-10〜60°C）");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }  // USB-CDC接続待ち
    delay(500);

    // I2Cバス初期化
    testWire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // モジュール生成
    mpu = new Mpu6500Module(MPU6500_CONFIG, &testWire);

    UNITY_BEGIN();

    RUN_TEST(test_mpu6500_i2c_scan);
    RUN_TEST(test_mpu6500_who_am_i);
    RUN_TEST(test_mpu6500_init);
    RUN_TEST(test_mpu6500_update_valid);
    RUN_TEST(test_mpu6500_accel_range);
    RUN_TEST(test_mpu6500_gyro_range);
    RUN_TEST(test_mpu6500_temperature);

    UNITY_END();

    delete mpu;
}

void loop() {}
