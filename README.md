# Embedded Module Architecture

ESP32-S3 N16R8 CAM開発ボードを題材にした、組み込みシステム向けModuleインターフェース設計パターンのリファレンス実装。

> **注意: 本リポジトリのコードは実機での動作を保証するものではありません。**
> 実装されているモジュール群は、あくまでアーキテクチャの設計パターンを具体的なコードで示すためのサンプルです。
> 実機での動作検証は行っていないため、実プロジェクトに適用する際は十分なテストを行ってください。

## コンセプト

組み込みシステムにおけるハードウェアモジュール（カメラ・センサー・LED・LCD・モーター等）を、統一的なインターフェース（`IModule`）で管理する設計パターンを定義・実証する。

主な設計要素:

- **IModuleインターフェース** — `init()` / `update()` / `deinit()` による統一ライフサイクル
- **SystemDataパターン** — モジュール間のデータ連携を共有構造体で一元管理
- **3フェーズ実行モデル** — `loop()` を「入力 → ロジック → 出力」に分離
- **Configパターン** — ピンアサインや設定を構造体で外部注入
- **通信バスパターン** — SPI/I2Cバスを `main.cpp` で一元管理

## リポジトリ構成

```
├── lib/ModuleCore/        コア層（IModule, ModuleTimer）— プロジェクト非依存
├── lib/{ModuleName}/      各モジュール実装
├── include/               SystemData.h, ProjectConfig.h
├── src/main.cpp           エントリーポイント
├── doc/                   設計仕様書（LaTeX）
├── doc-textbook/          教科書（LaTeX）
├── doc-guide/             実装ガイド（LaTeX）
└── platformio.ini         PlatformIO設定
```

## 実装モジュール一覧

| モジュール | 種別 | 概要 |
|---|---|---|
| LedModule | 出力 | GPIO LED制御 |
| ButtonModule | 入力 | デバウンス付きボタン入力 |
| BatteryModule | 入力 | ADC読み取り + 移動平均フィルタ |
| ServoModule | 出力 | PWMサーボ制御 |
| BuzzerModule | 出力 | PWMブザー制御 + deinit() |
| Mpu6500Module | 入力 | I2C IMU（加速度・ジャイロ） |
| TouchModule | 入力 | タッチパネル入力（TFT_eSPI） |
| TftModule | 出力 | SPI TFTディスプレイ |
| CameraModule | 入力 | OV5640カメラ + PSRAM |
| DriveMotorModule | 出力 | DCモーターPWM制御 |
| ChassisModule | 出力 | 4輪統合足回り制御 |
| BleModule | 入力 | BLE通信（volatileフラグ方式） |
| WifiModule | 入力 | WiFi通信（ステートマシン） |
| EncoderModule | 入力 | ロータリーエンコーダ（割り込み） |

## ドキュメント

| ドキュメント | ディレクトリ | 内容 |
|---|---|---|
| 設計仕様書 | `doc/` | アーキテクチャのルール・API仕様 |
| 教科書 | `doc-textbook/` | パターンの段階的な学習 |
| 実装ガイド | `doc-guide/` | 実コードの解説 |

PDF は各ディレクトリ内に生成されます。詳細な仕様・設計情報は `doc/main.tex`（設計仕様書）を参照してください。

## 環境構築

### 開発環境（PlatformIO）
1. [VSCode](https://code.visualstudio.com/) をインストール
2. [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) 拡張機能をインストール
3. プロジェクトを開く

### ドキュメント環境（LaTeX）
1. VSCodeでDevContainerを起動（`.devcontainer/` 使用）
2. LaTeX Workshop拡張機能が自動でインストールされる

## ビルド方法

### ファームウェア
```bash
pio run -e esp32-s3-cam-n16r8
```

### 仕様書
DevContainer内で `doc/main.tex` を開き、LaTeX Workshopでコンパイル。
