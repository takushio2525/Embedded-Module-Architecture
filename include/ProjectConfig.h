// ProjectConfig.h — プロジェクト固有設定の集約（Configインスタンスのみ）
// 注意: ピン番号はハードウェア配線に合わせて変更すること
#pragma once
#include "SystemData.h"

// ===== LED =====
const LedConfig LED_CONFIG = {
    .ledPin = 2,  // GPIO2: オンボードLED
};

// ===== TFT LCD (2.8インチ, SPI, ST7789) =====
// SPIピン (MOSI/MISO/SCK/CS/DC/RST/TouchCS) は platformio.ini の build_flags で設定
const TftConfig TFT_CONFIG = {
    .rotation        = 1,    // 1=横向き (landscape)
    .blPin           = 46,   // バックライト制御ピン（-1で常時ON）
    .updateIntervalMs = 100, // 画面更新周期 100ms (10fps)
};

// ===== MPU-6500 (I2C) =====
const Mpu6500Config MPU6500_CONFIG = {
    .address          = 0x68,  // AD0=LOW: 0x68 / AD0=HIGH: 0x69
    .sdaPin           = 41,    // I2C SDA
    .sclPin           = 42,    // I2C SCL
    .sampleIntervalMs = 20,    // サンプリング周期 20ms (50Hz)
};

// ===== OV5640 カメラ (DVP) =====
// 注意: ピン番号はボードのシルクまたは回路図を参照すること
// 以下はAI Thinker ESP32-S3-CAMボードの例
const CameraConfig CAMERA_CONFIG = {
    .pwdnPin    = -1,  // 電源制御なし
    .resetPin   = -1,  // ハードウェアリセットなし
    .xclkPin    = 15,
    .siodPin    = 4,
    .siocPin    = 5,
    .d0Pin      = 11,
    .d1Pin      = 9,
    .d2Pin      = 8,
    .d3Pin      = 10,
    .d4Pin      = 12,
    .d5Pin      = 18,
    .d6Pin      = 17,
    .d7Pin      = 16,
    .vsyncPin   = 6,
    .hrefPin    = 7,
    .pclkPin    = 13,
    .xclkFreqHz  = 20000000,   // 20MHz
    .frameSize   = 5,          // FRAMESIZE_QVGA (320x240) = 5
    .pixFormat   = 4,          // PIXFORMAT_JPEG = 4
    .jpegQuality = 12,         // 画質 (低い値=高画質)
    .fbCount     = 2,          // PSRAMへのダブルバッファ
};
