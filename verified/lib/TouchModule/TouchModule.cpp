// TouchModule.cpp — XPT2046タッチパネルモジュール実装
#include "TouchModule.h"
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "SystemData.h"

TouchModule::TouchModule(const TouchConfig& config, lgfx::LGFX_Device* lcd)
    : _config(config), _lcd(lcd) {}

bool TouchModule::init() {
    Serial.println("[Touch] init OK");
    return true;
}

void TouchModule::update(SystemData& data) {
    lgfx::touch_point_t tp;
    int touchCount = _lcd->getTouch(&tp, 1);
    data.touch.touchPressed = (touchCount > 0);
    if (data.touch.touchPressed) {
        data.touch.touchX = tp.x;
        data.touch.touchY = tp.y;
    }
}
