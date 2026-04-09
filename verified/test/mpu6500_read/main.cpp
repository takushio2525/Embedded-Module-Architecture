// mpu6500_read — MPU-6500 IMUセンサー読み取りテスト
// 3フェーズモデルで加速度・ジャイロ・温度をシリアル出力する
// 書き込み＋シリアルモニター:
//   cd verified && pio run -e test-mpu6500 -t upload && pio device monitor
#include <Arduino.h>
#include <Wire.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== モジュール・データ =====
// Wire1（I2Cポート1）を使用 — ポート0はesp_camera SCCBが使用
Mpu6500Module mpu(MPU6500_CONFIG, Wire1);
SystemData systemData;

// シリアル出力用タイマー（見やすいよう500ms間隔で表示）
static ModuleTimer printTimer;
static constexpr uint32_t PRINT_INTERVAL_MS = 500;

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    Serial.println("[Mpu6500Test] 起動");

    // I2Cバス初期化（Arduino Wire API）
    Wire1.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000);

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
