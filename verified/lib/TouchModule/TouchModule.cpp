// TouchModule.cpp — XPT2046タッチパネルモジュール実装
#include "TouchModule.h"
#include <TFT_eSPI.h>
#include "SystemData.h"

TouchModule::TouchModule(const TouchConfig& config, TFT_eSPI* tft)
    : _config(config), _tft(tft) {}

bool TouchModule::init() {
    Serial.println("[Touch] init OK");
    return true;
}

void TouchModule::update(SystemData& data) {
    uint16_t x, y;
    data.touch.touchPressed = _tft->getTouch(&x, &y);
    if (data.touch.touchPressed) {
        data.touch.touchX = x;
        data.touch.touchY = y;
    }
}
