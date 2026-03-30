// Mpu6500Module.h — MPU-6500 IMUセンサーモジュール（I2C接続）
// ESP-IDF レガシーI2C API使用（esp_camera SCCBとの共存のためWire不可）
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// --- Config構造体 ---
struct Mpu6500Config {
    uint8_t  address;          // I2Cアドレス (AD0=LOW: 0x68 / AD0=HIGH: 0x69)
    uint32_t sampleIntervalMs; // サンプリング周期 [ms]
    // バスピン(SDA/SCL)はmain.cppのsetup()でi2c_param_config()に指定する（Config外）
};

// --- Data構造体 ---
struct Mpu6500Data {
    // 加速度 [g]
    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 0.0f;
    // 角速度 [deg/s]
    float gyroX  = 0.0f;
    float gyroY  = 0.0f;
    float gyroZ  = 0.0f;
    // 温度 [°C]
    float temperature = -999.0f;
    // 状態
    bool isValid = false;
};

// --- モジュール実装 ---
struct SystemData;

class Mpu6500Module : public IModule {
public:
    // i2cPort: ESP-IDF I2Cポート番号 (0 or 1)
    explicit Mpu6500Module(const Mpu6500Config& config, uint8_t i2cPort);
    bool init()                         override;
    void updateInput(SystemData& data)  override;

private:
    Mpu6500Config _config;
    uint8_t       _i2cPort;
    ModuleTimer   _sampleTimer;

    // I2Cレジスタアクセス（ESP-IDF レガシーAPI）
    bool     writeReg(uint8_t reg, uint8_t value);
    uint8_t  readReg(uint8_t reg);
    bool     readBurst(uint8_t startReg, uint8_t* buf, uint8_t len);
};
