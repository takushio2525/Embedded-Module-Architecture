// TouchModule.h — XPT2046タッチパネルモジュール（LovyanGFX統合）
// タッチ読み込みを担う入力モジュール
// タッチハードウェアの初期化はTftModuleが担当（表示ボード上の同一IC）
// 本モジュールはTftModule::getLcd()経由でタッチデータを読み取る
#pragma once
#include <Arduino.h>
#include "IModule.h"

// LovyanGFX前方宣言（<LovyanGFX.hpp>はcppでインクルード）
namespace lgfx { inline namespace v1 { class LGFX_Device; } }

// --- Config構造体 ---
struct TouchConfig {
    // タッチ動作設定（キャリブレーション閾値等があればここに追加）
    // ハードウェアピンはTftConfig内（表示ボード上の同一デバイスのため）
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
    lgfx::v1::LGFX_Device* _lcd;   // TftModuleが所有するLCDデバイス
};
