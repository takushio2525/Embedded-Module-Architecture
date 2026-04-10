// BleModule.h — BLE通信モジュール（スレッドセーフ・volatileフラグ方式）
// BLEコールバックは別タスクで実行されるため、内部バッファ経由でSystemDataに渡す
#pragma once
#include <Arduino.h>
#include "IModule.h"

// --- 定数 ---
static constexpr uint8_t BLE_RX_BUFFER_SIZE = 128;
static constexpr uint8_t BLE_TX_BUFFER_SIZE = 128;

// --- Config構造体 ---
struct BleConfig {
    const char* deviceName;    // BLEデバイス名（アドバタイズ名）
    const char* serviceUuid;   // サービスUUID文字列
    const char* rxCharUuid;    // RX Characteristic UUID（クライアント→デバイス）
    const char* txCharUuid;    // TX Characteristic UUID（デバイス→クライアント）
};

// --- Data構造体 ---
struct BleData {
    bool    connected     = false;  // BLE接続状態
    bool    newDataIn     = false;  // 新規受信データあり（ロジックフェーズで参照後クリアする）
    uint8_t rxData[BLE_RX_BUFFER_SIZE] = {};  // 受信データ
    uint8_t rxLength      = 0;      // 受信データ長
    // 送信リクエスト（ロジックフェーズで書き込む）
    bool    sendRequest   = false;  // true: updateOutput()でtxDataを送信
    uint8_t txData[BLE_TX_BUFFER_SIZE] = {};  // 送信データ
    uint8_t txLength      = 0;      // 送信データ長
};

// --- モジュール実装 ---
struct SystemData;

// BLEライブラリのクラスを前方宣言（<BLEDevice.h>はcppでインクルード）
class BLEServer;
class BLECharacteristic;

class BleModule : public IModule {
private:
    BleConfig _config;

    // BLEオブジェクト（init()で生成）
    BLEServer*         _server = nullptr;
    BLECharacteristic* _txChar = nullptr;
    BLECharacteristic* _rxChar = nullptr;

    // --- 内部バッファ（コールバックスレッドが書き込む） ---
    uint8_t      _rxBuffer[BLE_RX_BUFFER_SIZE] = {};
    uint8_t      _rxBufferLength = 0;
    volatile bool _dataReceived  = false;  // volatileフラグ
    volatile bool _clientConnected = false;

public:
    BleModule(const BleConfig& config);
    bool init() override;
    void updateInput(SystemData& data) override;
    void updateOutput(SystemData& data) override;
    void deinit() override;

    // BLEコールバックから呼ばれるメソッド（別タスクで実行される）
    void onConnect();
    void onDisconnect();
    void onReceive(const uint8_t* payload, uint8_t length);
};
