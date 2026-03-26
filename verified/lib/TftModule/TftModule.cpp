// TftModule.cpp — TFT LCD 表示モジュール実装
// LovyanGFX ライブラリを使用（ピン設定は ProjectConfig.h で管理）
#include "TftModule.h"
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "SystemData.h"

TftModule::TftModule(const TftConfig& config, lgfx::LGFX_Device* lcd)
    : _config(config), _lcd(lcd) {}

bool TftModule::init() {
    // lcd->init() はsetup()で呼び出し済み
    _lcd->setRotation(_config.rotation);
    _lcd->fillScreen(TFT_BLACK);
    _lcd->setFont(&lgfx::fonts::Font2);

    _updateTimer.setTime();
    _initialized = true;

    // 起動画面
    _lcd->setTextColor(TFT_CYAN, TFT_BLACK);
    _lcd->drawString("Module Test Bench", 10, 10);
    _lcd->setTextColor(TFT_WHITE, TFT_BLACK);
    _lcd->drawString("Initializing...", 10, 34);

    Serial.println("[TFT] init OK");
    return true;
}

void TftModule::deinit() {
    _initialized = false;
    Serial.println("[TFT] deinit");
}

void TftModule::update(SystemData& data) {
    if (!_initialized) return;

    if (_updateTimer.getNowTime() >= _config.updateIntervalMs) {
        _updateTimer.setTime();
        renderDisplay(data);
    }
}

void TftModule::renderDisplay(const SystemData& data) {
    const uint16_t lineHeight = 24;
    const uint16_t startX = 8;
    uint16_t y = 8;

    _lcd->setTextColor(TFT_WHITE, TFT_BLACK);
    _lcd->drawString(data.tft.line1, startX, y); y += lineHeight;
    _lcd->drawString(data.tft.line2, startX, y); y += lineHeight;
    _lcd->drawString(data.tft.line3, startX, y); y += lineHeight;
    _lcd->drawString(data.tft.line4, startX, y); y += lineHeight;

    // line5（タッチ座標など）
    if (data.tft.line5[0] != '\0') {
        _lcd->setTextColor(TFT_YELLOW, TFT_BLACK);
        _lcd->drawString(data.tft.line5, startX, y);
    } else {
        _lcd->fillRect(startX, y, 300, lineHeight, TFT_BLACK);
    }
}
