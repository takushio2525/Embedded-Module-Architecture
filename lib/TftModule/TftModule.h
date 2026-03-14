// TftModule.h — 2.8インチ SPI TFT LCD + タッチモジュール（ST7789 / ILI9341）
// タッチ読み込み（入力）と画面描画（出力）を1モジュールで担う
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "IModule.h"
#include "ModuleTimer.h"

// --- Config構造体 ---
// 注意: SPI/タッチのピン番号は platformio.ini の build_flags で設定すること
//       TFT_MOSI / TFT_SCLK / TFT_MISO / TFT_CS / TFT_DC / TFT_RST / TOUCH_CS
struct TftConfig {
    uint8_t rotation;         // 画面回転 0-3 (1=横向き)
    int8_t  blPin;            // バックライトPWMピン (-1 で常時ON)
    uint32_t updateIntervalMs; // 画面更新周期 [ms]
};

// --- Data構造体 ---
struct TftData {
    // タッチ情報（outputフェーズで更新、logicフェーズで読み取る）
    bool     touchPressed = false;
    uint16_t touchX       = 0;  // ピクセル座標
    uint16_t touchY       = 0;

    // 表示内容（logicフェーズで設定、outputフェーズで描画）
    char line1[48] = "";
    char line2[48] = "";
    char line3[48] = "";
    char line4[48] = "";
};

// SystemDataの前方宣言はIModule.hで行われている

// --- モジュール実装 ---
class TftModule : public IModule {
public:
    TftModule(const TftConfig& config, SPIClass* spi);
    bool init()                    override;
    void update(SystemData& data)  override;
    void deinit()                  override;

private:
    TftConfig  _config;
    SPIClass*  _spi;
    ModuleTimer _updateTimer;
    bool _initialized = false;

    void readTouch(SystemData& data);
    void renderDisplay(const SystemData& data);
};
