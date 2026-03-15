// WifiModule.h — WiFi接続管理モジュール（ステートマシン内蔵）
// 接続→切断→再接続→タイムアウトの状態遷移を自動管理する
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

// --- WiFi接続状態 ---
enum class WifiState : uint8_t {
    DISCONNECTED,  // 未接続
    CONNECTING,    // 接続試行中
    CONNECTED,     // 接続済み
    RECONNECTING,  // 再接続試行中
    FAILED,        // 接続失敗（タイムアウト超過）
};

// --- Config構造体 ---
struct WifiConfig {
    const char*   ssid;              // WiFi SSID
    const char*   password;          // WiFiパスワード
    unsigned long connectTimeoutMs;  // 接続タイムアウト [ms]（例: 10000）
    unsigned long reconnectIntervalMs; // 再接続間隔 [ms]（例: 5000）
    uint8_t       maxRetries;        // 最大リトライ回数（0=無制限）
};

// --- Data構造体 ---
struct WifiData {
    WifiState     state       = WifiState::DISCONNECTED;
    bool          isConnected = false;    // 接続中フラグ（簡易参照用）
    uint8_t       retryCount  = 0;       // 現在のリトライ回数
    int8_t        rssi        = 0;       // 信号強度 [dBm]
    uint32_t      localIp     = 0;       // ローカルIPアドレス（uint32_t形式）
    bool          requestConnect    = false; // ロジックフェーズからの接続リクエスト
    bool          requestDisconnect = false; // ロジックフェーズからの切断リクエスト
};

// --- モジュール実装 ---
struct SystemData;

class WifiModule : public IModule {
private:
    WifiConfig  _config;
    ModuleTimer _stateTimer;    // 状態遷移用タイマー
    WifiState   _state = WifiState::DISCONNECTED;
    uint8_t     _retryCount = 0;

    void _startConnect();
    void _disconnect();

public:
    WifiModule(const WifiConfig& config);
    bool init() override;
    void update(SystemData& data) override;
    void deinit() override;
};
