// mpu6500_read — MPU-6500 IMUセンサー読み取りテスト
// 3フェーズモデルで加速度・ジャイロ・温度をシリアル出力する
// 書き込み＋シリアルモニター:
//   cd verified && pio run -e test-mpu6500 -t upload && pio device monitor
#include <Arduino.h>
#include "driver/i2c.h"
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== モジュール・データ =====
Mpu6500Module mpu(MPU6500_CONFIG, 1);  // I2Cポート1（ポート0はesp_camera SCCBが使用）
SystemData systemData;

// シリアル出力用タイマー（見やすいよう500ms間隔で表示）
static ModuleTimer printTimer;
static constexpr uint32_t PRINT_INTERVAL_MS = 500;

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    Serial.println("[Mpu6500Test] 起動");

    // I2Cバス初期化（レガシーAPI — esp_camera SCCBとの共存のためWire不可）
    i2c_config_t i2cConf = {};
    i2cConf.mode = I2C_MODE_MASTER;
    i2cConf.sda_io_num = I2C_SDA_PIN;
    i2cConf.scl_io_num = I2C_SCL_PIN;
    i2cConf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2cConf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2cConf.master.clk_speed = 400000;
    i2c_param_config(I2C_NUM_1, &i2cConf);
    i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0);

    // モジュール初期化
    if (!mpu.init()) {
        Serial.println("[Mpu6500Test] init失敗");
        mpu.enabled = false;
    }

    printTimer.setTime();
    Serial.println("[Mpu6500Test] 開始");
}

void loop() {
    // 1. 入力フェーズ
    if (mpu.enabled) {
        mpu.updateInput(systemData);
    }

    // 2. ロジックフェーズ
    if (printTimer.getNowTime() >= PRINT_INTERVAL_MS) {
        printTimer.setTime();

        if (systemData.mpu.isValid) {
            Serial.printf("Accel: X=%.2f Y=%.2f Z=%.2f [g]\n",
                          systemData.mpu.accelX,
                          systemData.mpu.accelY,
                          systemData.mpu.accelZ);
            Serial.printf("Gyro:  X=%.1f Y=%.1f Z=%.1f [deg/s]\n",
                          systemData.mpu.gyroX,
                          systemData.mpu.gyroY,
                          systemData.mpu.gyroZ);
            Serial.printf("Temp:  %.1f [C]\n\n",
                          systemData.mpu.temperature);
        } else {
            Serial.println("MPU6500: データ無効");
        }
    }

    // 3. 出力フェーズ（なし）
}
