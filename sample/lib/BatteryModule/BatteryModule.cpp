// BatteryModule.cpp — バッテリー電圧監視モジュールの実装
#include "BatteryModule.h"
#include "SystemData.h"

BatteryModule::BatteryModule(const BatteryConfig& config) : _config(config) {}

bool BatteryModule::init() {
    pinMode(_config.adcPin, INPUT);
    _sampleTimer.setTime();
    Serial.printf("[Battery] init OK (pin=%d, divider=%.1f)\n",
                  _config.adcPin, _config.voltageDivider);
    return true;
}

void BatteryModule::updateInput(SystemData& data) {
    if (_sampleTimer.getNowTime() < _config.sampleIntervalMs) {
        return;
    }
    _sampleTimer.setTime();

    // ADC読み取り → 電圧変換
    uint16_t adcRaw = analogRead(_config.adcPin);
    float voltage = _adcToVoltage(adcRaw);

    // リングバッファに追加
    _samples[_sampleIndex] = voltage;
    _sampleIndex = (_sampleIndex + 1) % BATTERY_FILTER_SIZE;
    if (_sampleCount < BATTERY_FILTER_SIZE) {
        _sampleCount++;
    }

    // SystemDataに書き込み
    data.battery.rawVoltage = voltage;
    data.battery.voltage    = _calcAverage();
    data.battery.isValid    = (_sampleCount >= BATTERY_FILTER_SIZE);
    data.battery.isLow      = (data.battery.voltage < _config.lowVoltageThreshold);
}

float BatteryModule::_adcToVoltage(uint16_t adcValue) const {
    // ADC値 → ピン電圧 → 実電圧（分圧比で逆算）
    float pinVoltage = (float)adcValue / _config.adcResolution * _config.adcRefVoltage;
    return pinVoltage * _config.voltageDivider;
}

float BatteryModule::_calcAverage() const {
    if (_sampleCount == 0) return 0.0f;
    float sum = 0.0f;
    for (uint8_t i = 0; i < _sampleCount; i++) {
        sum += _samples[i];
    }
    return sum / _sampleCount;
}
