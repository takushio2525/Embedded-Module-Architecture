// DisplayBoardModule.h — TFT表示ボードモジュール（ST7789 + XPT2046タッチ）
// 同一基板上のLCD表示（出力）とタッチ読み取り（入力）を統合管理する。
// TFT_eSPIをモジュール内部で保持し、外部からのドライバ注入は不要。
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// TFT_eSPIはポインタのみ使用するため前方宣言（<TFT_eSPI.h>はcppでインクルード）
class TFT_eSPI;

// --- Config構造体 ---
// 注意: SPI/タッチのピン番号は platformio.ini の build_flags で設定すること
// TFT_eSPIが内部でバスを管理するため、外部SPIClassは使用しない
struct DisplayBoardConfig {
    uint8_t  rotation;         // 画面回転 0-3 (1=横向き)
    int8_t   blPin;            // バックライトPWMピン (-1 で常時ON)
    uint32_t updateIntervalMs; // 画面更新周期 [ms]
};

// --- Data構造体 ---
struct DisplayBoardData {
    // 出力: 表示内容（ロジックフェーズで設定、出力フェーズで描画）
    char line1[48] = "";
    char line2[48] = "";
    char line3[48] = "";
    char line4[48] = "";
    char line5[48] = "";       // タッチ座標など（黄色テキスト）
    // 入力: タッチ状態（入力フェーズで更新）
    bool     touchPressed = false;
    uint16_t touchX       = 0; // ピクセル座標
    uint16_t touchY       = 0;
};

// --- モジュール実装 ---
struct SystemData;

class DisplayBoardModule : public IModule {
public:
    DisplayBoardModule(const DisplayBoardConfig& config);
    ~DisplayBoardModule();
    bool init()                          override;
    void updateInput(SystemData& data)   override;  // 入力フェーズ: タッチ読み取り
    void updateOutput(SystemData& data)  override;  // 出力フェーズ: 画面描画
    void deinit()                        override;

private:
    DisplayBoardConfig _config;
    TFT_eSPI*          _tft;             // TFT_eSPIインスタンス（内部で生成）
    ModuleTimer        _updateTimer;
    bool               _initialized = false;

    void renderDisplay(const SystemData& data);
};
