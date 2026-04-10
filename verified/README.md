# Verified Project

ESP32-S3 CAM搭載ロボットで実機テスト済みのプロジェクト。

## ハードウェア構成

- **マイコン**: ESP32-S3 N16R8（PSRAM付き）
- **ディスプレイ**: ILI9341 TFT + XPT2046タッチパネル（SPI共有）
- **IMU**: MPU-6500（I2C）
- **カメラ**: OV5640（DVPインターフェース）
- **通信**: BLE 5.0
- **その他**: サーボ、SDカード（SPI共有）

## 実装済みモジュール

| モジュール | 種別 | 概要 |
|---|---|---|
| DisplayBoardModule | 入出力 | LovyanGFXによるTFT描画+タッチ入力の統合モジュール |
| Mpu6500Module | 入力 | ESP-IDF I2C APIによるIMU読み取り |
| CameraModule | 入力 | OV5640カメラ + PSRAMフレームバッファ |
| BleModule | 入出力 | BLE通信（volatileフラグ方式） |
| ServoModule | 出力 | PWMサーボ制御 |
| SdModule | 入力 | SPI SDカード（TFTとバス共有） |

## sample/ との技術差異

| 項目 | sample/ | verified/ | 理由 |
|---|---|---|---|
| I2C | Arduino Wire (`TwoWire*`) | ESP-IDF レガシー I2C API | esp_camera の SCCB が Wire と共存不可 |
| LCD | TFT_eSPI | LovyanGFX | タッチ統合 + SPI共有排他制御 |
| Arduino Core | v2.x (`ledcSetup`/`ledcAttachPin`) | v3.x (`ledcAttach`) | 最新プラットフォームで検証 |
| TFT+タッチ | TftModule + TouchModule に分離 | DisplayBoardModule に統合 | LovyanGFXが一元管理 |

## ビルド

```bash
cd verified/
pio run -e esp32-s3-cam-n16r8

# ビルド + 書き込み
pio run -e esp32-s3-cam-n16r8 -t upload

# シリアルモニター
pio device monitor
```
