// ProjectConfig.h — プロジェクト固有設定の集約（Configインスタンスのみ）
// 注意: ピン番号はハードウェア配線に合わせて変更すること
#pragma once
#include "SystemData.h"

// ===== LED =====
const LedConfig LED_CONFIG = {
    .ledPin = 2, // GPIO2: オンボードLED
};

// ===== 表示ボード (2.8インチ TFT + タッチパネル統合) =====
// SPIピン (MOSI/MISO/SCK/CS/DC/RST) とタッチCSは platformio.ini の build_flags で設定
// TFT_eSPIが内部でバスを管理するため、外部SPIClassは使用しない
const DisplayBoardConfig DISPLAY_BOARD_CONFIG = {
    .rotation = 1,           // 1=横向き (landscape)
    .blPin = 46,             // バックライト制御ピン（-1で常時ON）
    .updateIntervalMs = 100, // 画面更新周期 100ms (10fps)
};

// ===== 共有I2Cバスピン =====
constexpr int I2C_SDA_PIN = 41;
constexpr int I2C_SCL_PIN = 42;

// ===== MPU-6500 (I2C) =====
const Mpu6500Config MPU6500_CONFIG = {
    .address = 0x68,        // AD0=LOW: 0x68 / AD0=HIGH: 0x69
    .sampleIntervalMs = 20, // サンプリング周期 20ms (50Hz)
};

// ===== OV5640 カメラ (DVP) =====
// 注意: ピン番号はボードのシルクまたは回路図を参照すること
// 以下はAI Thinker ESP32-S3-CAMボードの例
const CameraConfig CAMERA_CONFIG = {
    .pwdnPin = -1,  // 電源制御なし
    .resetPin = -1, // ハードウェアリセットなし
    .xclkPin = 15,
    .siodPin = 4,
    .siocPin = 5,
    .d0Pin = 11,
    .d1Pin = 9,
    .d2Pin = 8,
    .d3Pin = 10,
    .d4Pin = 12,
    .d5Pin = 18,
    .d6Pin = 17,
    .d7Pin = 16,
    .vsyncPin = 6,
    .hrefPin = 7,
    .pclkPin = 13,
    .xclkFreqHz = 20000000, // 20MHz
    .frameSize = 5,         // FRAMESIZE_QVGA (320x240) = 5
    .pixFormat = 4,         // PIXFORMAT_JPEG = 4
    .jpegQuality = 12,      // 画質 (低い値=高画質)
    .fbCount = 2,           // PSRAMへのダブルバッファ
};

// ===== サーボ (ESP32 PWM) =====
const ServoConfig SERVO_CONFIG = {
    .pin = 1,           // GPIO1: サーボ信号
    .pwmChannel = 0,    // LEDCチャネル0
    .minPulseUs = 500,  // SG90: 500μs
    .maxPulseUs = 2500, // SG90: 2500μs
    .defaultAngle = 90, // 初期角度 90度（中央）
};

// ===== シャーシ（4輪オムニ） =====
const ChassisConfig CHASSIS_CONFIG = {
    .motors = {
        // [0] 左前（LEDCチャネル2）
        {.in1Pin = 4, .in2Pin = 16, .pwmPin = 17, .pwmChannel = 2, .pwmFreqHz = 1220, .minPowerThreshold = 5.0f},
        // [1] 右前（LEDCチャネル3）
        {.in1Pin = 26, .in2Pin = 25, .pwmPin = 33, .pwmChannel = 3, .pwmFreqHz = 1220, .minPowerThreshold = 5.0f},
        // [2] 左後（LEDCチャネル4）
        {.in1Pin = 18, .in2Pin = 19, .pwmPin = 23, .pwmChannel = 4, .pwmFreqHz = 1220, .minPowerThreshold = 5.0f},
        // [3] 右後（LEDCチャネル5）
        {.in1Pin = 13, .in2Pin = 14, .pwmPin = 27, .pwmChannel = 5, .pwmFreqHz = 1220, .minPowerThreshold = 5.0f},
    },
    .wheelPattern = WheelPattern::OMNI,
    .maxOutputPercent = 90.0f,
};

// ===== ボタン（プルアップ接続） =====
const ButtonConfig BUTTON_CONFIG = {
    .pin = 0,          // GPIO0: BOOTボタン（多くのESP32ボードに搭載）
    .activeLow = true, // LOW=押下（内蔵プルアップ使用）
    .debounceMs = 30,  // デバウンス30ms
};

// ===== BLE =====
const BleConfig BLE_CONFIG = {
    .deviceName = "ESP32-Module",
    .serviceUuid = "12345678-1234-1234-1234-123456789abc",
    .rxCharUuid = "12345678-1234-1234-1234-123456789abd",
    .txCharUuid = "12345678-1234-1234-1234-123456789abe",
};

// ===== バッテリー電圧監視（ADC） =====
const BatteryConfig BATTERY_CONFIG = {
    .adcPin = 3,                 // GPIO3: ADC1_CH2
    .voltageDivider = 2.0f,      // 1/2分圧回路
    .adcRefVoltage = 3.3f,       // ESP32 ADC基準電圧
    .adcResolution = 4095,       // 12bit ADC
    .sampleIntervalMs = 500,     // 500msごとにサンプリング
    .lowVoltageThreshold = 3.3f, // 3.3V以下で低電圧警告
};

// ===== ブザー（LEDCチャネル6） =====
const BuzzerConfig BUZZER_CONFIG = {
    .pin = 47,       // GPIO47: ブザー出力
    .pwmChannel = 6, // LEDCチャネル6（サーボch0, モーターch2-5と重複しない）
};

// ===== WiFi =====
const WifiConfig WIFI_CONFIG = {
    .ssid = "YourSSID",
    .password = "YourPassword",
    .connectTimeoutMs = 10000,   // 10秒タイムアウト
    .reconnectIntervalMs = 5000, // 5秒後に再接続
    .maxRetries = 5,             // 最大5回リトライ
};

// ===== ロータリーエンコーダ =====
const EncoderConfig ENCODER_CONFIG = {
    .pinA = 39,             // GPIO39: A相
    .pinB = 40,             // GPIO40: B相
    .pulsesPerRev = 20,     // 1回転20パルス
    .sampleIntervalMs = 50, // 50msごとに速度計算
};
