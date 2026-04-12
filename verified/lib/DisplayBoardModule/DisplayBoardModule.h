// DisplayBoardModule.h — 2.8インチ TFT表示ボードモジュール（ILI9341 + XPT2046）
// 同一基板上のLCD表示（出力）とタッチ読み取り（入力）を統合管理する。
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// LovyanGFX内部コンポーネント（定義はcpp内、ヘッダへのLovyanGFX依存を防ぐ）
struct DisplayBoardLgfxImpl;

// --- Config構造体 ---
// 表示ボード上の全ハードウェア（ILI9341 + XPT2046）の設定
struct DisplayBoardConfig
{
    // SPIバスピン（LovyanGFXが内部でバスを構成するため必要。
    //             外部SPIClassは使用しない）
    int8_t spiMosiPin;
    int8_t spiMisoPin;
    int8_t spiSckPin;
    // TFTパネルピン
    int8_t tftCsPin;  // TFT CSピン
    int8_t tftDcPin;  // TFT DCピン
    int8_t tftRstPin; // TFT RSTピン (-1で未使用)
    // タッチパネルピン（XPT2046）
    int8_t touchCsPin;  // タッチCSピン
    int8_t touchIrqPin; // タッチIRQピン (-1で未使用)
    // 表示設定
    uint8_t rotation;          // 画面回転 0-3 (1=横向き)
    uint32_t updateIntervalMs; // 画面更新周期 [ms]
};

// --- Data構造体 ---
struct DisplayBoardData
{
    // 出力: 表示内容（ロジックフェーズで設定、出力フェーズで描画）
    char line1[48] = "";
    char line2[48] = "";
    char line3[48] = "";
    char line4[48] = "";
    char line5[48] = ""; // メモリ情報など（黄色テキスト）
    // 出力: カメラJPEG画像（ロジックフェーズで設定、出力フェーズで描画）
    const uint8_t *jpegData = nullptr; // JPEGフレームバッファへのポインタ
    size_t jpegSize = 0;               // JPEGデータサイズ [bytes]
    // 入力: タッチ状態（入力フェーズで更新）
    bool touchPressed = false;
    uint16_t touchX = 0; // ピクセル座標
    uint16_t touchY = 0;
};

// --- モジュール実装 ---
struct SystemData;

class DisplayBoardModule : public IModule
{
public:
    DisplayBoardModule(const DisplayBoardConfig &config);
    ~DisplayBoardModule();
    bool init() override;
    void updateInput(SystemData &data) override;  // 入力フェーズ: タッチ読み取り
    void updateOutput(SystemData &data) override; // 出力フェーズ: 画面描画
    void deinit() override;

private:
    DisplayBoardConfig _config;
    DisplayBoardLgfxImpl *_impl; // LovyanGFX内部コンポーネント群
    ModuleTimer _updateTimer;
    bool _initialized = false;

    void renderDisplay(const SystemData &data);
};
