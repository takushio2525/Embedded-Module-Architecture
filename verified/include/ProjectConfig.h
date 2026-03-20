// ProjectConfig.h — ピン定義とモジュール設定の集約
// ボード: ESP32-S3 N16R8 CAM (OV5640)
#pragma once
#include "SystemData.h"

// ===== 共有SPIバス ピン定義（TFT + Touch + SD） =====
// 3デバイスがSPIバスを共有し、CSピンで排他制御する
constexpr int SPI_MOSI = 38;
constexpr int SPI_MISO = 40;
constexpr int SPI_SCK  = 39;

// ===== TFT LCD (ILI9341, 2.8インチ, 320x240) =====
// SPIピンは共有バスを使用（platformio.iniのbuild_flagsで設定）
constexpr int TFT_CS_PIN  = 41;
constexpr int TFT_DC_PIN  = 42;
constexpr int TFT_RST_PIN = 48;
// バックライト: 3.3Vに直結（GPIO不使用、常時点灯）

// ===== タッチパネル (XPT2046, SPI) =====
constexpr int TOUCH_CS_PIN  = 47;
constexpr int TOUCH_IRQ_PIN = 14;

// ===== SDカード (SPI, ボード裏面スロット) =====
constexpr int SD_CS_PIN = 21;

// ===== MPU-6500 (I2C) =====
constexpr int I2C_SDA_PIN = 2;
constexpr int I2C_SCL_PIN = 3;

// ===== サーボ SG90 (PWM) =====
constexpr int SERVO_PIN = 1;

// ===== カメラ OV5640 (DVP, ボード固定配線) =====
// カメラピンはボード上で固定されているため変更不可
constexpr int CAM_PWDN_PIN  = -1;  // 電源制御なし
constexpr int CAM_RESET_PIN = -1;  // ハードウェアリセットなし
constexpr int CAM_XCLK_PIN  = 15;
constexpr int CAM_SIOD_PIN  = 4;   // SCCB SDA
constexpr int CAM_SIOC_PIN  = 5;   // SCCB SCL
constexpr int CAM_D0_PIN    = 11;
constexpr int CAM_D1_PIN    = 9;
constexpr int CAM_D2_PIN    = 8;
constexpr int CAM_D3_PIN    = 10;
constexpr int CAM_D4_PIN    = 12;
constexpr int CAM_D5_PIN    = 18;
constexpr int CAM_D6_PIN    = 17;
constexpr int CAM_D7_PIN    = 16;
constexpr int CAM_VSYNC_PIN = 6;
constexpr int CAM_HREF_PIN  = 7;
constexpr int CAM_PCLK_PIN  = 13;

// ===== Configインスタンス =====
// モジュール実装時に各Config構造体をここに定義していく
