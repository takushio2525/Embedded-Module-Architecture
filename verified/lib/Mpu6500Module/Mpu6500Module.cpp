// Mpu6500Module.cpp — MPU-6500 IMUセンサーモジュール実装
#include "Mpu6500Module.h"
#include <Wire.h>
#include "SystemData.h"

// MPU-6500 レジスタアドレス
static constexpr uint8_t REG_SMPLRT_DIV   = 0x19;
static constexpr uint8_t REG_CONFIG        = 0x1A;
static constexpr uint8_t REG_GYRO_CONFIG   = 0x1B;
static constexpr uint8_t REG_ACCEL_CONFIG  = 0x1C;
static constexpr uint8_t REG_ACCEL_XOUT_H  = 0x3B; // ACCEL_X ~ GYRO_Z の14バイト先頭
static constexpr uint8_t REG_PWR_MGMT_1    = 0x6B;
static constexpr uint8_t REG_WHO_AM_I      = 0x75;

// スケールファクタ（デフォルト設定）
// ACCEL: ±2g → 1g = 16384 LSB
// GYRO:  ±250°/s → 1°/s = 131 LSB
static constexpr float ACCEL_SCALE = 1.0f / 16384.0f;
static constexpr float GYRO_SCALE  = 1.0f / 131.0f;

// MPU-6500 の WHO_AM_I 期待値
static constexpr uint8_t WHO_AM_I_MPU6500 = 0x70;

Mpu6500Module::Mpu6500Module(const Mpu6500Config& config, TwoWire* wire)
    : _config(config), _wire(wire) {}

bool Mpu6500Module::init() {
    // デバイス確認
    uint8_t whoAmI = readReg(REG_WHO_AM_I);
    if (whoAmI != WHO_AM_I_MPU6500) {
        Serial.printf("[MPU6500] WHO_AM_I mismatch: 0x%02X (expected 0x%02X)\n",
                      whoAmI, WHO_AM_I_MPU6500);
        return false;
    }

    // リセット解除（スリープ解除 + クロックソース: PLL）
    if (!writeReg(REG_PWR_MGMT_1, 0x01)) return false;
    delay(10);

    // サンプリングレート設定（1kHz / (1 + SMPLRT_DIV)）
    writeReg(REG_SMPLRT_DIV, 0x04);  // 200Hz

    // デジタルローパスフィルタ設定（92Hz帯域幅）
    writeReg(REG_CONFIG, 0x02);

    // ジャイロ設定: ±250°/s
    writeReg(REG_GYRO_CONFIG, 0x00);

    // 加速度設定: ±2g
    writeReg(REG_ACCEL_CONFIG, 0x00);

    _sampleTimer.setTime(_config.sampleIntervalMs);  // 即時採取開始
    Serial.println("[MPU6500] init OK");
    return true;
}

void Mpu6500Module::updateInput(SystemData& data) {
    if (_sampleTimer.getNowTime() < _config.sampleIntervalMs) return;
    _sampleTimer.setTime();

    // ACCEL_XOUT_H から GYRO_ZOUT_L まで14バイト一括読み込み
    uint8_t buf[14];
    if (!readBurst(REG_ACCEL_XOUT_H, buf, 14)) {
        data.mpu.isValid = false;
        Serial.println("[MPU6500] read error");
        return;
    }

    // 生データをint16_tに変換
    int16_t rawAx = (int16_t)((buf[0]  << 8) | buf[1]);
    int16_t rawAy = (int16_t)((buf[2]  << 8) | buf[3]);
    int16_t rawAz = (int16_t)((buf[4]  << 8) | buf[5]);
    // buf[6-7]: 温度
    int16_t rawTp = (int16_t)((buf[6]  << 8) | buf[7]);
    int16_t rawGx = (int16_t)((buf[8]  << 8) | buf[9]);
    int16_t rawGy = (int16_t)((buf[10] << 8) | buf[11]);
    int16_t rawGz = (int16_t)((buf[12] << 8) | buf[13]);

    // 物理値へ変換
    data.mpu.accelX      = rawAx * ACCEL_SCALE;
    data.mpu.accelY      = rawAy * ACCEL_SCALE;
    data.mpu.accelZ      = rawAz * ACCEL_SCALE;
    data.mpu.gyroX       = rawGx * GYRO_SCALE;
    data.mpu.gyroY       = rawGy * GYRO_SCALE;
    data.mpu.gyroZ       = rawGz * GYRO_SCALE;
    // 温度: データシートの換算式
    data.mpu.temperature = (rawTp / 333.87f) + 21.0f;
    data.mpu.isValid     = true;
}

// --- I2Cヘルパー ---

bool Mpu6500Module::writeReg(uint8_t reg, uint8_t value) {
    _wire->beginTransmission(_config.address);
    _wire->write(reg);
    _wire->write(value);
    return (_wire->endTransmission() == 0);
}

uint8_t Mpu6500Module::readReg(uint8_t reg) {
    _wire->beginTransmission(_config.address);
    _wire->write(reg);
    _wire->endTransmission(false);
    _wire->requestFrom(_config.address, (uint8_t)1);
    return _wire->available() ? _wire->read() : 0xFF;
}

bool Mpu6500Module::readBurst(uint8_t startReg, uint8_t* buf, uint8_t len) {
    _wire->beginTransmission(_config.address);
    _wire->write(startReg);
    if (_wire->endTransmission(false) != 0) return false;
    _wire->requestFrom(_config.address, len);
    if (_wire->available() < len) return false;
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = _wire->read();
    }
    return true;
}
