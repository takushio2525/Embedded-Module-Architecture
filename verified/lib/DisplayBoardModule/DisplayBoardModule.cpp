// DisplayBoardModule.cpp — TFT表示ボードモジュール実装
// LovyanGFXコンポーネントを内部で生成・設定し、LCD表示とタッチ読み取りを統合管理する
#include "DisplayBoardModule.h"
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "SystemData.h"

// LGFX内部コンポーネント（ヘッダではforward declareのみ）
struct DisplayBoardLgfxImpl {
    lgfx::LGFX_Device    lcd;
    lgfx::Panel_ILI9341  panel;
    lgfx::Bus_SPI        bus;
    lgfx::Touch_XPT2046  touch;
};

DisplayBoardModule::DisplayBoardModule(const DisplayBoardConfig& config)
    : _config(config), _impl(new DisplayBoardLgfxImpl()) {}

DisplayBoardModule::~DisplayBoardModule() {
    delete _impl;
}

bool DisplayBoardModule::init() {
    // SPIバス設定
    {
        auto cfg = _impl->bus.config();
        cfg.spi_host   = SPI2_HOST;   // FSPI (ESP32-S3)
        cfg.freq_write = 40000000;    // 書き込み 40MHz
        cfg.freq_read  = 20000000;    // 読み込み 20MHz
        cfg.pin_mosi   = _config.spiMosiPin;
        cfg.pin_miso   = _config.spiMisoPin;
        cfg.pin_sclk   = _config.spiSckPin;
        cfg.pin_dc     = _config.tftDcPin;
        cfg.spi_mode   = 0;
        cfg.use_lock   = true;        // バス排他制御を有効化（SD共有）
        _impl->bus.config(cfg);
        _impl->panel.setBus(&_impl->bus);
    }

    // パネル設定 (ILI9341, 2.8インチ 320x240)
    {
        auto cfg = _impl->panel.config();
        cfg.pin_cs        = _config.tftCsPin;
        cfg.pin_rst       = _config.tftRstPin;
        cfg.panel_width   = 240;
        cfg.panel_height  = 320;
        cfg.memory_width  = 240;
        cfg.memory_height = 320;
        cfg.offset_x      = 0;
        cfg.offset_y      = 0;
        cfg.readable      = true;
        cfg.invert        = false;
        cfg.rgb_order     = false;
        cfg.bus_shared    = true;     // SDカードとSPIバス共有
        _impl->panel.config(cfg);
    }

    // タッチパネル設定 (XPT2046)
    {
        auto cfg = _impl->touch.config();
        cfg.spi_host = SPI2_HOST;     // TFTと同じSPIバス
        cfg.freq     = 2500000;       // タッチSPI 2.5MHz
        cfg.pin_cs   = _config.touchCsPin;
        cfg.pin_int  = _config.touchIrqPin;
        cfg.x_min    = 300;
        cfg.x_max    = 3900;
        cfg.y_min    = 200;
        cfg.y_max    = 3700;
        _impl->touch.config(cfg);
        _impl->panel.setTouch(&_impl->touch);
    }

    // LCDデバイス組み立て＋初期化
    _impl->lcd.setPanel(&_impl->panel);
    _impl->lcd.init();
    _impl->lcd.setRotation(_config.rotation);
    _impl->lcd.fillScreen(TFT_BLACK);
    _impl->lcd.setFont(&lgfx::fonts::Font2);

    _updateTimer.setTime();
    _initialized = true;

    // 起動画面
    _impl->lcd.setTextColor(TFT_CYAN, TFT_BLACK);
    _impl->lcd.drawString("Module Test Bench", 10, 10);
    _impl->lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    _impl->lcd.drawString("Initializing...", 10, 34);

    Serial.println("[DisplayBoard] init OK");
    return true;
}

void DisplayBoardModule::deinit() {
    _initialized = false;
    Serial.println("[DisplayBoard] deinit");
}

void DisplayBoardModule::update(SystemData& data) {
    updateInput(data);
    updateOutput(data);
}

void DisplayBoardModule::updateInput(SystemData& data) {
    if (!_initialized) return;

    // タッチ読み取り
    lgfx::touch_point_t tp;
    int touchCount = _impl->lcd.getTouch(&tp, 1);
    data.display.touchPressed = (touchCount > 0);
    if (data.display.touchPressed) {
        data.display.touchX = tp.x;
        data.display.touchY = tp.y;
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

    _impl->lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    _impl->lcd.drawString(data.display.line1, startX, y); y += lineHeight;
    _impl->lcd.drawString(data.display.line2, startX, y); y += lineHeight;
    _impl->lcd.drawString(data.display.line3, startX, y); y += lineHeight;
    _impl->lcd.drawString(data.display.line4, startX, y); y += lineHeight;

    // line5（メモリ情報など）
    if (data.display.line5[0] != '\0') {
        _impl->lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
        _impl->lcd.drawString(data.display.line5, startX, y);
    } else {
        _impl->lcd.fillRect(startX, y, 300, lineHeight, TFT_BLACK);
    }
}
