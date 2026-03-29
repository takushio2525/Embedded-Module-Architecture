// sd_readwrite — SDカード読み書きテスト
// 3フェーズモデルでSDカードの状態をシリアル出力する
// SdModuleはinit時にテストを実行し、結果をDataに格納する
#include <Arduino.h>
#include <SPI.h>
#include "IModule.h"
#include "ProjectConfig.h"

// ===== バスインスタンス =====
static SPIClass sdSpi = SPIClass(FSPI);

// ===== モジュール・データ =====
SdModule sd(SD_CONFIG, &sdSpi);
SystemData systemData;

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    Serial.println("[SdTest] 起動");

    // バス初期化（モジュールinit()より前）
    sdSpi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);

    // モジュール初期化
    if (!sd.init()) {
        Serial.println("[SdTest] init失敗");
        sd.enabled = false;
    }

    Serial.println("[SdTest] 開始");
}

void loop() {
    // 1. 入力フェーズ
    if (sd.enabled) {
        sd.updateInput(systemData);
    }

    // 2. ロジックフェーズ
    if (systemData.sd.isValid) {
        Serial.printf("SD: 容量=%llu MB  使用=%llu MB  テスト=%s\n",
                      systemData.sd.totalBytes / (1024 * 1024),
                      systemData.sd.usedBytes / (1024 * 1024),
                      systemData.sd.testPassed ? "OK" : "NG");
    } else {
        Serial.println("SD: 未検出");
    }

    // 結果を表示したら停止（SDは繰り返しテスト不要）
    Serial.println("[SdTest] 完了（リセットで再実行）");
    while (true) { ; }

    // 3. 出力フェーズ（なし）
}
