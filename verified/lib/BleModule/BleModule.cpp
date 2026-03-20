// BleModule.cpp — BLE通信モジュールの実装
#include "BleModule.h"
#include "SystemData.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ===== BLEコールバッククラス =====

// 接続・切断コールバック
class BleServerCallbacks : public BLEServerCallbacks {
private:
    BleModule* _module;
public:
    BleServerCallbacks(BleModule* module) : _module(module) {}
    void onConnect(BLEServer* server) override {
        _module->onConnect();
    }
    void onDisconnect(BLEServer* server) override {
        _module->onDisconnect();
    }
};

// RX Characteristic 書き込みコールバック
class BleRxCallbacks : public BLECharacteristicCallbacks {
private:
    BleModule* _module;
public:
    BleRxCallbacks(BleModule* module) : _module(module) {}
    void onWrite(BLECharacteristic* characteristic) override {
        std::string value = characteristic->getValue();
        if (value.length() > 0) {
            uint8_t len = (value.length() > BLE_RX_BUFFER_SIZE)
                          ? BLE_RX_BUFFER_SIZE : value.length();
            _module->onReceive(
                reinterpret_cast<const uint8_t*>(value.data()), len);
        }
    }
};

// ===== BleModule実装 =====

BleModule::BleModule(const BleConfig& config) : _config(config) {}

bool BleModule::init() {
    BLEDevice::init(_config.deviceName);
    _server = BLEDevice::createServer();
    if (!_server) {
        Serial.println("[BLE] init failed: createServer");
        return false;
    }

    _server->setCallbacks(new BleServerCallbacks(this));

    // サービス作成
    BLEService* service = _server->createService(_config.serviceUuid);
    if (!service) {
        Serial.println("[BLE] init failed: createService");
        return false;
    }

    // TX Characteristic（Notify）
    _txChar = service->createCharacteristic(
        _config.txCharUuid,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    _txChar->addDescriptor(new BLE2902());

    // RX Characteristic（Write）
    _rxChar = service->createCharacteristic(
        _config.rxCharUuid,
        BLECharacteristic::PROPERTY_WRITE
    );
    _rxChar->setCallbacks(new BleRxCallbacks(this));

    // サービス開始・アドバタイズ開始
    service->start();
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(_config.serviceUuid);
    advertising->start();

    Serial.printf("[BLE] init OK (name=%s)\n", _config.deviceName);
    return true;
}

void BleModule::update(SystemData& data) {
    // IModule互換: 入力・出力の両方を実行
    updateInput(data);
    updateOutput(data);
}

void BleModule::updateInput(SystemData& data) {
    // 接続状態をSystemDataに反映
    data.ble.connected = _clientConnected;

    // 受信データの受け渡し（volatileフラグ方式）
    if (_dataReceived) {
        memcpy(data.ble.rxData, _rxBuffer, _rxBufferLength);
        data.ble.rxLength  = _rxBufferLength;
        data.ble.newDataIn = true;
        _dataReceived = false;  // フラグクリア
    }
}

void BleModule::updateOutput(SystemData& data) {
    // 送信リクエスト処理
    if (data.ble.sendRequest && _clientConnected && _txChar) {
        _txChar->setValue(data.ble.txData, data.ble.txLength);
        _txChar->notify();
        data.ble.sendRequest = false;
    }
}

void BleModule::deinit() {
    BLEDevice::deinit(false);
    _server = nullptr;
    _txChar = nullptr;
    _rxChar = nullptr;
    Serial.println("[BLE] deinit OK");
}

// --- コールバックメソッド（別タスクから呼ばれる） ---

void BleModule::onConnect() {
    _clientConnected = true;
    Serial.println("[BLE] client connected");
}

void BleModule::onDisconnect() {
    _clientConnected = false;
    // 再アドバタイズ開始
    BLEDevice::getAdvertising()->start();
    Serial.println("[BLE] client disconnected, re-advertising");
}

void BleModule::onReceive(const uint8_t* payload, uint8_t length) {
    // 内部バッファに書き込み → volatileフラグを立てる
    memcpy(_rxBuffer, payload, length);
    _rxBufferLength = length;
    _dataReceived = true;
}
