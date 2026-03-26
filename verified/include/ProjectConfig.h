// ProjectConfig.h — モジュール設定の集約
// ボード: ESP32-S3 N16R8 CAM (OV5640)
#pragma once
#include "SystemData.h"

// ===== 共有バス ピン定義 =====
// バスピンは特定モジュールに属さない共有インフラのため、単体定数として定義する。
// モジュール固有のピンは各Configインスタンスのリテラルに直書きすること。

// 共有SPIバス（TFT + Touch + SD の3デバイスがCSピンで排他制御）
constexpr int SPI_MOSI_PIN = 38;
constexpr int SPI_MISO_PIN = 40;
constexpr int SPI_SCK_PIN  = 39;

// I2Cバス（MPU-6500）
constexpr int I2C_SDA_PIN = 2;
constexpr int I2C_SCL_PIN = 3;

// ===== Configインスタンス =====
// モジュール固有のピン番号はConfigリテラルに直書きする。
// 外部からピンを参照する場合は SERVO_CONFIG.pin のように構造体経由でアクセスする。

// TFT LCD (ILI9341, 2.8インチ)
// LovyanGFXの組み立て（バス・パネル・タッチ）はmain.cppのsetupLcd()で実行
const TftConfig TFT_CONFIG = {
    .csPin           = 41,   // TFT CSピン
    .dcPin           = 42,   // TFT DCピン
    .rstPin          = 48,   // TFT RSTピン
    .rotation        = 1,    // 1=横向き (landscape, 320x240)
    .updateIntervalMs = 100, // 画面更新周期 100ms (10fps)
};

// タッチパネル (XPT2046)
const TouchConfig TOUCH_CONFIG = {
    .csPin  = 47,  // タッチCSピン
    .irqPin = 14,  // タッチIRQピン
};

// MPU-6500 (I2C)
// I2Cバス(Wire)はmain.cppのsetup()でI2C_SDA_PIN/I2C_SCL_PINを指定して初期化
const Mpu6500Config MPU6500_CONFIG = {
    .address          = 0x68,  // AD0=LOW: 0x68 / AD0=HIGH: 0x69
    .sampleIntervalMs = 20,    // サンプリング周期 20ms (50Hz)
};

// サーボ SG90 (PWM)
const ServoConfig SERVO_CONFIG = {
    .pin          = 1,     // サーボ信号ピン（GPIO1）
    .pwmChannel   = 0,     // LEDCチャネル0
    .minPulseUs   = 500,   // SG90: 500μs
    .maxPulseUs   = 2500,  // SG90: 2500μs
    .defaultAngle = 90,    // 初期角度 90度（中央）
};

// SDカード (SPI, ボード裏面スロット)
// SPIバスはTFT/Touchと共有（main.cppで同一SPIインスタンスを渡す）
const SdConfig SD_CONFIG = {
    .csPin = 21,  // SD CSピン（GPIO21）
};

// カメラ OV5640 (DVP, ボード固定配線)
const CameraConfig CAMERA_CONFIG = {
    .pwdnPin    = -1,   // 電源制御なし
    .resetPin   = -1,   // ハードウェアリセットなし
    .xclkPin    = 15,   // クロック出力
    .siodPin    = 4,    // SCCB SDA
    .siocPin    = 5,    // SCCB SCL
    .d0Pin      = 11,
    .d1Pin      = 9,
    .d2Pin      = 8,
    .d3Pin      = 10,
    .d4Pin      = 12,
    .d5Pin      = 18,
    .d6Pin      = 17,
    .d7Pin      = 16,
    .vsyncPin   = 6,    // 垂直同期
    .hrefPin    = 7,    // 水平参照
    .pclkPin    = 13,   // ピクセルクロック
    .xclkFreqHz  = 20000000,   // 20MHz
    .frameSize   = 5,          // FRAMESIZE_QVGA (320x240)
    .pixFormat   = 4,          // PIXFORMAT_JPEG
    .jpegQuality = 12,         // JPEG品質 (低い値=高画質)
    .fbCount     = 2,          // PSRAMへのダブルバッファ
};

// BLE
const BleConfig BLE_CONFIG = {
    .deviceName  = "ESP32-TestBench",
    .serviceUuid = "12345678-1234-1234-1234-123456789abc",
    .rxCharUuid  = "12345678-1234-1234-1234-123456789abd",
    .txCharUuid  = "12345678-1234-1234-1234-123456789abe",
};
