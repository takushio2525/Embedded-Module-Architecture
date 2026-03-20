// EncoderModule.h — ロータリーエンコーダ入力モジュール（割り込み連携）
// ハードウェア割り込みでパルスをカウントし、update()でSystemDataにコピーする
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// --- Config構造体 ---
struct EncoderConfig {
    uint8_t       pinA;             // エンコーダA相ピン
    uint8_t       pinB;             // エンコーダB相ピン
    uint16_t      pulsesPerRev;     // 1回転あたりのパルス数（例: 20）
    unsigned long sampleIntervalMs; // 速度計算間隔 [ms]（例: 50）
};

// --- Data構造体 ---
struct EncoderData {
    int32_t count    = 0;     // 累積パルス数（正:CW、負:CCW）
    float   rpm      = 0.0f;  // 回転速度 [rpm]
    bool    isValid  = false;  // 速度計算が1回以上行われたらtrue
};

// --- モジュール実装 ---
struct SystemData;

class EncoderModule : public IModule {
private:
    EncoderConfig _config;
    ModuleTimer   _sampleTimer;

    // 割り込みハンドラが更新する変数（volatile）
    volatile int32_t _isrCount = 0;

    // update()内で使う前回値（速度計算用）
    int32_t _prevCount = 0;

    // 割り込みハンドラ（staticでなければattachInterruptに渡せない）
    // インスタンスポインタを使って実体にアクセスする
    static void IRAM_ATTR _isrHandlerA();
    static EncoderModule* _instance;  // 割り込み用シングルインスタンス参照

    // B相の現在値を読み取って方向を判定
    void IRAM_ATTR _onPulseA();

public:
    EncoderModule(const EncoderConfig& config);
    bool init() override;
    void update(SystemData& data) override;
    void deinit() override;
};
