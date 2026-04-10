// LedModule.cpp — LED制御モジュールの実装
#include "LedModule.h"
#include "SystemData.h"
#include <Arduino.h>

LedModule::LedModule(const LedConfig& config) : _config(config) {}

bool LedModule::init() {
    pinMode(_config.ledPin, OUTPUT);
    Serial.println("[Led] init OK");
    return true;
}

void LedModule::updateOutput(SystemData& data) {
    digitalWrite(_config.ledPin, data.led.state ? HIGH : LOW);
}
