// LgfxDriver.h — LovyanGFX ディスプレイドライバ設定
// ILI9341 (TFT) + XPT2046 (タッチ) を共有SPIバスで駆動
// ピン・バス・パネル・タッチの全ハードウェア設定をこのクラスに集約する
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LgfxDriver : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel;
    lgfx::Bus_SPI       _bus;
    lgfx::Touch_XPT2046 _touch;

public:
    LgfxDriver() {
        // SPIバス設定（TFT + Touch + SD 共有バス）
        {
            auto cfg = _bus.config();
            cfg.spi_host   = SPI2_HOST;   // FSPI (ESP32-S3)
            cfg.freq_write = 40000000;    // 書き込み 40MHz
            cfg.freq_read  = 20000000;    // 読み込み 20MHz
            cfg.pin_mosi   = 38;
            cfg.pin_miso   = 40;
            cfg.pin_sclk   = 39;
            cfg.pin_dc     = 42;          // TFT DCピン
            cfg.spi_mode   = 0;
            cfg.use_lock   = true;        // バス排他制御を有効化
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }

        // パネル設定 (ILI9341, 2.8インチ 320x240)
        {
            auto cfg = _panel.config();
            cfg.pin_cs        = 41;       // TFT CSピン
            cfg.pin_rst       = 48;       // TFT RSTピン
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
            _panel.config(cfg);
        }

        // タッチパネル設定 (XPT2046)
        {
            auto cfg = _touch.config();
            cfg.spi_host = SPI2_HOST;     // TFTと同じSPIバス
            cfg.freq     = 2500000;       // タッチSPI 2.5MHz
            cfg.pin_cs   = 47;            // タッチCSピン
            cfg.pin_int  = 14;            // タッチIRQピン
            cfg.x_min    = 300;
            cfg.x_max    = 3900;
            cfg.y_min    = 200;
            cfg.y_max    = 3700;
            _touch.config(cfg);
            _panel.setTouch(&_touch);
        }

        setPanel(&_panel);
    }
};
