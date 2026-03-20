// SdModule.h — SDカードモジュール（SPI接続、TFT/Touchとバス共有）
// テスト用: 読み書きテストとファイルリスト取得
#pragma once
#include <Arduino.h>
#include "IModule.h"

// SPIClassはポインタのみ使用するため前方宣言
class SPIClass;

// --- Config構造体 ---
struct SdConfig {
    int8_t csPin;  // SDカードのCSピン
};

// --- Data構造体 ---
struct SdData {
    bool isValid       = false;  // SDカード検出済み
    bool testCompleted = false;  // 読み書きテスト完了
    bool testPassed    = false;  // テスト結果
    uint64_t totalBytes = 0;     // カード容量 [byte]
    uint64_t usedBytes  = 0;     // 使用量 [byte]
};

// --- モジュール実装 ---
struct SystemData;

class SdModule : public IModule {
public:
    SdModule(const SdConfig& config, SPIClass* spi);
    bool init()                   override;
    void update(SystemData& data) override;
    void deinit()                 override;

private:
    SdConfig   _config;
    SPIClass*  _spi;
    bool       _initialized = false;

    // 読み書きテスト（init時に1回実行）
    bool runReadWriteTest();
};
