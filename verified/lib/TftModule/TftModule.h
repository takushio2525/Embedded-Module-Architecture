// TftModule.h — 2.8インチ SPI TFT LCD 表示モジュール（ILI9341）
// 画面描画のみを担う出力モジュール（タッチ読み込みはTouchModuleが担当）
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// TFT_eSPIはポインタのみ使用するため前方宣言（<TFT_eSPI.h>はcppでインクルード）
class TFT_eSPI;

// --- Config構造体 ---
// 注意: SPI/タッチのピン番号は platformio.ini の build_flags で設定すること
struct TftConfig {
    uint8_t  rotation;         // 画面回転 0-3 (1=横向き)
    uint32_t updateIntervalMs; // 画面更新周期 [ms]
};

// --- Data構造体 ---
struct TftData {
    // 表示内容（ロジックフェーズで設定、出力フェーズで描画）
    char line1[48] = "";
    char line2[48] = "";
    char line3[48] = "";
    char line4[48] = "";
    char line5[48] = "";
};

// --- モジュール実装 ---
struct SystemData;

class TftModule : public IModule {
public:
    TftModule(const TftConfig& config, TFT_eSPI* tft);
    bool init()                   override;
    void update(SystemData& data) override;
    void deinit()                 override;

private:
    TftConfig   _config;
    TFT_eSPI*   _tft;
    ModuleTimer _updateTimer;
    bool        _initialized = false;

    void renderDisplay(const SystemData& data);
};
