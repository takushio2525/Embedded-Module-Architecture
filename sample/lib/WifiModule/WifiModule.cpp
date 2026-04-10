// WifiModule.cpp — WiFi接続管理モジュールの実装
#include "WifiModule.h"
#include "SystemData.h"
#include <WiFi.h>

WifiModule::WifiModule(const WifiConfig& config) : _config(config) {}

bool WifiModule::init() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);  // 自前で再接続管理する
    Serial.printf("[WiFi] init OK (SSID=%s)\n", _config.ssid);
    return true;
}

void WifiModule::updateInput(SystemData& data) {
    // ステートマシン（WiFi状態の監視・自動再接続）
    switch (_state) {
        case WifiState::DISCONNECTED:
            // 何もしない（接続リクエスト待ち）
            break;

        case WifiState::CONNECTING:
        case WifiState::RECONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                // 接続成功
                _state = WifiState::CONNECTED;
                _retryCount = 0;
                Serial.printf("[WiFi] connected (IP=%s, RSSI=%d)\n",
                              WiFi.localIP().toString().c_str(), WiFi.RSSI());
            } else if (_stateTimer.getNowTime() >= _config.connectTimeoutMs) {
                // タイムアウト
                WiFi.disconnect();
                _retryCount++;
                if (_config.maxRetries > 0 && _retryCount >= _config.maxRetries) {
                    _state = WifiState::FAILED;
                    Serial.printf("[WiFi] failed after %d retries\n", _retryCount);
                } else {
                    // 再接続待機
                    _state = WifiState::RECONNECTING;
                    _stateTimer.setTime();
                    Serial.printf("[WiFi] timeout, retry %d\n", _retryCount);
                }
            }
            break;

        case WifiState::CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                // 接続が切れた → 再接続開始
                _state = WifiState::RECONNECTING;
                _stateTimer.setTime();
                Serial.println("[WiFi] disconnected, reconnecting...");
            }
            break;

        case WifiState::FAILED:
            // 接続リクエストで復帰可能（updateOutputで処理）
            break;
    }

    // RECONNECTING状態: 再接続間隔待ち
    if (_state == WifiState::RECONNECTING && WiFi.status() != WL_CONNECTED) {
        if (_stateTimer.getNowTime() >= _config.reconnectIntervalMs) {
            _startConnect();
        }
    }

    // SystemDataに状態を反映
    data.wifi.state       = _state;
    data.wifi.isConnected = (_state == WifiState::CONNECTED);
    data.wifi.retryCount  = _retryCount;
    if (_state == WifiState::CONNECTED) {
        data.wifi.rssi    = WiFi.RSSI();
        data.wifi.localIp = (uint32_t)WiFi.localIP();
    } else {
        data.wifi.rssi    = 0;
        data.wifi.localIp = 0;
    }
}

void WifiModule::updateOutput(SystemData& data) {
    // ロジックフェーズからのリクエスト処理
    if (data.wifi.requestConnect) {
        data.wifi.requestConnect = false;
        if (_state == WifiState::DISCONNECTED || _state == WifiState::FAILED) {
            _retryCount = 0;
            _startConnect();
        }
    }
    if (data.wifi.requestDisconnect) {
        data.wifi.requestDisconnect = false;
        _disconnect();
    }
}

void WifiModule::deinit() {
    _disconnect();
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFi] deinit OK");
}

void WifiModule::_startConnect() {
    WiFi.begin(_config.ssid, _config.password);
    _state = WifiState::CONNECTING;
    _stateTimer.setTime();
    Serial.printf("[WiFi] connecting to %s...\n", _config.ssid);
}

void WifiModule::_disconnect() {
    WiFi.disconnect();
    _state = WifiState::DISCONNECTED;
    _retryCount = 0;
    Serial.println("[WiFi] disconnected");
}
