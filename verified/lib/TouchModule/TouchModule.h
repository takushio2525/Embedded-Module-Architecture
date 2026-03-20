// TouchModule.h — XPT2046タッチパネルモジュール（TFT_eSPI統合）
// タッチ読み込みを担う入力モジュール
#pragma once
#include <Arduino.h>
#include "IModule.h"

// TFT_eSPIはポインタのみ使用するため前方宣言（<TFT_eSPI.h>はcppでインクルード）
class TFT_eSPI;

// --- Config構造体 ---
// タッチパネルのCSピンはTFT_eSPIと共有されており、
// platformio.iniのbuild_flags（TOUCH_CS）で指定する。
struct TouchConfig {
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
    TouchModule(const TouchConfig& config, TFT_eSPI* tft);
    bool init()                   override;
    void update(SystemData& data) override;

private:
    TouchConfig _config;
    TFT_eSPI*   _tft;
};
