// TftModule.h — 2.8インチ SPI TFT LCD 表示モジュール（ILI9341 + XPT2046）
// 画面描画を担う出力モジュール。LovyanGFXコンポーネントを内部で生成・管理する。
// タッチ読み込みはTouchModuleがgetLcd()経由で行う。
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// LovyanGFX前方宣言（<LovyanGFX.hpp>はcppでインクルード）
namespace lgfx { inline namespace v1 { class LGFX_Device; } }

// LGFX内部コンポーネント（定義はcpp内、ヘッダへのLovyanGFX依存を防ぐ）
struct TftLgfxImpl;

// --- Config構造体 ---
// 表示ボード上の全ハードウェア（ILI9341 + XPT2046）の設定を含む
struct TftConfig {
    // 共有SPIバスピン（LovyanGFXが内部でバスを構成するため必要）
    int8_t   mosiPin;
    int8_t   misoPin;
    int8_t   sckPin;
    // TFTパネルピン
    int8_t   csPin;            // TFT CSピン
    int8_t   dcPin;            // TFT DCピン
    int8_t   rstPin;           // TFT RSTピン (-1で未使用)
    // タッチピン（表示ボード上のXPT2046）
    int8_t   touchCsPin;       // タッチCSピン
    int8_t   touchIrqPin;      // タッチIRQピン (-1で未使用)
    // 表示設定
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
    TftModule(const TftConfig& config);
    ~TftModule();
    bool init()                   override;
    void update(SystemData& data) override;
    void deinit()                 override;

    // TouchModuleがタッチ読み取りに使用するLCDデバイスポインタ
    lgfx::v1::LGFX_Device* getLcd();

private:
    TftConfig    _config;
    TftLgfxImpl* _impl;         // LovyanGFX内部コンポーネント群
    ModuleTimer  _updateTimer;
    bool         _initialized = false;

    void renderDisplay(const SystemData& data);
};
