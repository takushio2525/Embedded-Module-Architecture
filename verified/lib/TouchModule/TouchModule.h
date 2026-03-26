// TouchModule.h — XPT2046タッチパネルモジュール（LovyanGFX統合）
// タッチ読み込みを担う入力モジュール
#pragma once
#include <Arduino.h>
#include "IModule.h"

// LgfxDriverはポインタのみ使用するため前方宣言（LgfxDriver.hはcppでインクルード）
class LgfxDriver;

// --- Config構造体 ---
// ピン・バス設定は LgfxDriver.h に集約
struct TouchConfig {
    // キャリブレーション等の追加設定があればここに定義
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
    TouchModule(const TouchConfig& config, LgfxDriver* lcd);
    bool init()                   override;
    void update(SystemData& data) override;

private:
    TouchConfig _config;
    LgfxDriver* _lcd;
};
