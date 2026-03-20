// TftModule.cpp — TFT LCD 表示モジュール実装
// TFT_eSPI ライブラリを使用（ピン設定は platformio.ini の build_flags で管理）
#include "TftModule.h"
#include <TFT_eSPI.h>
#include "SystemData.h"

TftModule::TftModule(const TftConfig& config, TFT_eSPI* tft)
    : _config(config), _tft(tft) {}

bool TftModule::init() {
    // _tft->init() はsetup()でtftDriver.init()として呼び済み
    _tft->setRotation(_config.rotation);
    _tft->fillScreen(TFT_BLACK);

    _updateTimer.setTime();
    _initialized = true;

    // 起動画面
    _tft->setTextColor(TFT_CYAN, TFT_BLACK);
    _tft->drawString("Module Test Bench", 10, 10, 2);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString("Initializing...", 10, 34, 2);

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

    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString(data.tft.line1, startX, y, 2); y += lineHeight;
    _tft->drawString(data.tft.line2, startX, y, 2); y += lineHeight;
    _tft->drawString(data.tft.line3, startX, y, 2); y += lineHeight;
    _tft->drawString(data.tft.line4, startX, y, 2); y += lineHeight;

    // line5（タッチ座標など）
    if (data.tft.line5[0] != '\0') {
        _tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        _tft->drawString(data.tft.line5, startX, y, 2);
    } else {
        _tft->fillRect(startX, y, 300, lineHeight, TFT_BLACK);
    }
}
