// TouchModule.h — XPT2046タッチパネルモジュール（LovyanGFX統合）
// タッチ読み込みを担う入力モジュール
#pragma once
#include <Arduino.h>
#include "IModule.h"

// LovyanGFX前方宣言（<LovyanGFX.hpp>はcppでインクルード）
namespace lgfx { inline namespace v1 { class LGFX_Device; } }

// --- Config構造体 ---
struct TouchConfig {
    int8_t csPin;   // タッチCSピン
    int8_t irqPin;  // タッチIRQピン (-1で未使用)
};

// --- Data構造体 ---
struct TouchData {
    bool     touchPressed = false;
    uint16_t touchX       = 0;  // ピクセル座標
    uint16_t touchY       = 0;
};

// --- モジュール実装 ---
struct SystemData;

class TouchModule : public IModule {
public:
    TouchModule(const TouchConfig& config, lgfx::v1::LGFX_Device* lcd);
    bool init()                   override;
    void update(SystemData& data) override;

private:
    TouchConfig            _config;
    lgfx::v1::LGFX_Device* _lcd;
};
