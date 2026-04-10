// Mpu6500Module.h — MPU-6500 IMUセンサーモジュール（I2C接続）
// ライブラリ非依存：Wireによる直接レジスタアクセス
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// TwoWireはポインタのみ使用するため前方宣言で十分（<Wire.h>はcppでインクルード）
class TwoWire;

// --- Config構造体 ---
struct Mpu6500Config {
    uint8_t  address;        // I2Cアドレス (AD0=LOW: 0x68 / AD0=HIGH: 0x69)
    int8_t   sdaPin;         // I2C SDAピン
    int8_t   sclPin;         // I2C SCLピン
    uint32_t sampleIntervalMs; // サンプリング周期 [ms]
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

// SystemDataの前方宣言はIModule.hで行われている

// --- モジュール実装 ---
class Mpu6500Module : public IModule {
public:
    explicit Mpu6500Module(const Mpu6500Config& config, TwoWire* wire);
    bool init()                    override;
    void updateInput(SystemData& data)  override;

private:
    Mpu6500Config _config;
    TwoWire*      _wire;
    ModuleTimer   _sampleTimer;

    // I2Cレジスタアクセス
    bool     writeReg(uint8_t reg, uint8_t value);
    uint8_t  readReg(uint8_t reg);
    bool     readBurst(uint8_t startReg, uint8_t* buf, uint8_t len);
};
