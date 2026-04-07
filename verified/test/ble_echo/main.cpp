// ble_echo — BLE通信 エコーバックテスト
// 3フェーズモデルで受信データをそのまま送り返す
// 書き込み＋シリアルモニター:
//   cd verified && pio run -e test-ble -t upload && pio device monitor
#include <Arduino.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== モジュール・データ =====
BleModule ble(BLE_CONFIG);
SystemData systemData;

// ステータス表示用タイマー
static ModuleTimer statusTimer;
static constexpr uint32_t STATUS_INTERVAL_MS = 3000;

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    Serial.println("[BleTest] 起動");

    // モジュール初期化
    if (!ble.init()) {
        Serial.println("[BleTest] init失敗");
        ble.enabled = false;
    }

    statusTimer.setTime();
    Serial.println("[BleTest] アドバタイズ中（BLEクライアントから接続してください）");
}

void loop() {
    // 1. 入力フェーズ
    if (ble.enabled) {
        ble.updateInput(systemData);
    }

    // 2. ロジックフェーズ

    // 受信データをエコーバック
    if (systemData.ble.newDataIn) {
        Serial.printf("BLE RX (%d bytes): %.*s\n",
                      systemData.ble.rxLength,
                      systemData.ble.rxLength,
                      systemData.ble.rxData);

        // 受信データをそのまま送信バッファにコピー
        memcpy(systemData.ble.txData, systemData.ble.rxData,
               systemData.ble.rxLength);
        systemData.ble.txLength    = systemData.ble.rxLength;
        systemData.ble.sendRequest = true;
        systemData.ble.newDataIn   = false;
    }

    // 定期ステータス表示
    if (statusTimer.getNowTime() >= STATUS_INTERVAL_MS) {
        statusTimer.setTime();
        Serial.printf("BLE: %s\n",
                      systemData.ble.connected ? "接続中" : "未接続");
    }

    // 3. 出力フェーズ
    if (ble.enabled) {
        ble.updateOutput(systemData);
    }
}
