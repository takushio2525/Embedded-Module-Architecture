// DisplayBoardModule.cpp — TFT表示ボードモジュール実装
// TFT_eSPIライブラリを内部で保持し、LCD表示とタッチ読み取りを統合管理する。
// ピン設定は platformio.ini の build_flags で管理する。
#include "DisplayBoardModule.h"
#include <TFT_eSPI.h>
#include "SystemData.h"

DisplayBoardModule::DisplayBoardModule(const DisplayBoardConfig& config)
    : _config(config), _tft(new TFT_eSPI()) {}

DisplayBoardModule::~DisplayBoardModule() {
    delete _tft;
}

bool DisplayBoardModule::init() {
    _tft->init();
    _tft->setRotation(_config.rotation);
    _tft->fillScreen(TFT_BLACK);

    // バックライト制御
    if (_config.blPin >= 0) {
        pinMode(_config.blPin, OUTPUT);
        digitalWrite(_config.blPin, HIGH);
    }

    _updateTimer.setTime();
    _initialized = true;

    // 起動画面
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString("Module Architecture", 10, 10, 2);
    _tft->drawString("Initializing...", 10, 34, 2);

    Serial.println("[DisplayBoard] init OK");
    return true;
}

void DisplayBoardModule::deinit() {
    if (_config.blPin >= 0) {
        digitalWrite(_config.blPin, LOW);
    }
    _initialized = false;
    Serial.println("[DisplayBoard] deinit");
}

void DisplayBoardModule::updateInput(SystemData& data) {
    if (!_initialized) return;

    // タッチ読み取り
    uint16_t x, y;
    data.display.touchPressed = _tft->getTouch(&x, &y);
    if (data.display.touchPressed) {
        data.display.touchX = x;
        data.display.touchY = y;
    }
}

void DisplayBoardModule::updateOutput(SystemData& data) {
    if (!_initialized) return;

    if (_updateTimer.getNowTime() >= _config.updateIntervalMs) {
        _updateTimer.setTime();
        renderDisplay(data);
    }
}

void DisplayBoardModule::renderDisplay(const SystemData& data) {
    const uint16_t lineHeight = 24;
    const uint16_t startX = 8;
    uint16_t y = 8;

    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString(data.display.line1, startX, y, 2); y += lineHeight;
    _tft->drawString(data.display.line2, startX, y, 2); y += lineHeight;
    _tft->drawString(data.display.line3, startX, y, 2); y += lineHeight;
    _tft->drawString(data.display.line4, startX, y, 2); y += lineHeight;

    // line5（タッチ座標など、ロジックフェーズで設定済み）
    if (data.display.line5[0] != '\0') {
        _tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        _tft->drawString(data.display.line5, startX, y, 2);
    } else {
        _tft->fillRect(startX, y, 240, lineHeight, TFT_BLACK);
    }
}
