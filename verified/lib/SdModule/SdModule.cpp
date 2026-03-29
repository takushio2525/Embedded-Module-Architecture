// SdModule.cpp — SDカードモジュール実装
// SD_MMCではなくSD(SPI)ライブラリを使用（TFT/Touchとバス共有のため）
#include "SdModule.h"
#include <SPI.h>
#include <SD.h>
#include "SystemData.h"

SdModule::SdModule(const SdConfig& config, SPIClass* spi)
    : _config(config), _spi(spi) {}

bool SdModule::init() {
    // SD.begin()にSPIバスインスタンスを渡してバス共有
    if (!SD.begin(_config.csPin, *_spi)) {
        Serial.println("[SD] init failed: カード未検出");
        return false;
    }

    _initialized = true;
    Serial.printf("[SD] init OK (type=%d)\n", SD.cardType());
    return true;
}

void SdModule::updateInput(SystemData& data) {
    if (!_initialized) return;

    data.sd.isValid    = true;
    data.sd.totalBytes = SD.totalBytes();
    data.sd.usedBytes  = SD.usedBytes();

    // 読み書きテストは1回だけ実行
    if (!data.sd.testCompleted) {
        data.sd.testPassed   = runReadWriteTest();
        data.sd.testCompleted = true;
    }
}

void SdModule::deinit() {
    if (_initialized) {
        SD.end();
        _initialized = false;
        Serial.println("[SD] deinit");
    }
}

bool SdModule::runReadWriteTest() {
    const char* testPath = "/module_test.txt";
    const char* testData = "Module Architecture Test Bench";

    // 書き込みテスト
    File file = SD.open(testPath, FILE_WRITE);
    if (!file) {
        Serial.println("[SD] テスト書き込み失敗: ファイルオープンエラー");
        return false;
    }
    file.println(testData);
    file.close();

    // 読み込みテスト
    file = SD.open(testPath, FILE_READ);
    if (!file) {
        Serial.println("[SD] テスト読み込み失敗: ファイルオープンエラー");
        return false;
    }
    String readBack = file.readStringUntil('\n');
    file.close();

    // テストファイル削除
    SD.remove(testPath);

    bool passed = readBack.startsWith(testData);
    Serial.printf("[SD] R/Wテスト: %s\n", passed ? "OK" : "NG");
    return passed;
}
