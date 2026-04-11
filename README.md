# Embedded Module Architecture

Arduinoフレームワーク対応マイコン全般で使える、組み込みシステム向けModuleインターフェース設計パターンのリファレンス実装。

## 学習の進め方

```
Step 1: 教科書（doc-textbook/）
  クラスの基礎から3フェーズモデルまで、Q&A形式で段階的に学ぶ
  ※ C++クラスに不安がある人は第1章「前提知識」から
  ※ クラスを理解している人は第2章から

    ↓

Step 2: 実装ガイド（doc-guide/）
  sample/ の実コードを題材に、設計パターンの適用方法を学ぶ
  ※ LedModule（最小構成）→ ButtonModule → ... の順に読む

    ↓

Step 3: 設計仕様書（doc/）
  ルール集・API仕様として随時参照する
  ※ 通読するより、必要なときに該当セクションを引く使い方を推奨
```

各ドキュメントのPDFは、それぞれのディレクトリ内に生成済みです。

## コンセプト

組み込みシステムにおけるハードウェアモジュール（カメラ・センサー・LED・LCD・モーター等）を、統一的なインターフェース（`IModule`）で管理する設計パターンを定義・実証する。コア設計思想はArduinoフレームワークが動く**あらゆるマイコン**（ESP32、STM32、AVR等）に適用可能。タイマークラスや実装例はArduino APIに合わせ、初心者でも扱いやすい形にしている。

主な設計要素:

- **IModuleインターフェース** — `init()` / `updateInput()` / `updateOutput()` / `deinit()` による統一ライフサイクル
- **SystemDataパターン** — モジュール間のデータ連携を共有構造体で一元管理
- **3フェーズ実行モデル** — `loop()` を「入力 → ロジック → 出力」に分離
- **Configパターン** — ピンアサインや設定を構造体で外部注入
- **通信バスパターン** — SPI/I2Cバスを `main.cpp` で一元管理

アーキテクチャの詳細は [ARCHITECTURE.md](ARCHITECTURE.md) を参照。

## リポジトリ構成

```
├── sample/                サンプルプロジェクト（設計パターンの構造説明用）
│   ├── lib/ModuleCore/    コア層（IModule, ModuleTimer）— プロジェクト非依存
│   ├── lib/{ModuleName}/  各モジュール実装
│   ├── include/           SystemData.h, ProjectConfig.h
│   ├── src/main.cpp       エントリーポイント
│   └── platformio.ini     PlatformIO設定
├── verified/              動作確認済みプロジェクト（実機テスト済み）
├── doc/                   設計仕様書（LaTeX）
├── doc-textbook/          教科書（LaTeX）
├── doc-guide/             実装ガイド（LaTeX）
└── ARCHITECTURE.md        アーキテクチャリファレンス
```

### sample/ — サンプルプロジェクト

アーキテクチャの設計パターンを具体的なコードで示すためのサンプル実装。ESP32-S3 N16R8 CAM開発ボードを題材にしている。

> **注意:** 実機での動作検証は行っていない。設計パターンの理解・学習を目的としたコードであり、実プロジェクトに適用する際は十分なテストを行うこと。

### verified/ — 動作確認済みプロジェクト

ESP32-S3 CAM搭載ロボットで実機テスト済みのプロジェクト。フォルダごとダウンロードすればそのまま使える自己完結型の構成。sample/ との技術差異については [verified/README.md](verified/README.md) を参照。

## サンプル実装モジュール一覧

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
| BleModule | 入出力 | BLE通信（volatileフラグ方式） |
| WifiModule | 入出力 | WiFi通信（ステートマシン） |
| EncoderModule | 入力 | ロータリーエンコーダ（割り込み） |

## ドキュメント

| ドキュメント | ディレクトリ | 内容 |
|---|---|---|
| 設計仕様書 | `doc/` | アーキテクチャのルール・API仕様（「何を守るか」） |
| 教科書 | `doc-textbook/` | パターンの段階的な学習（「なぜそうするか」） |
| 実装ガイド | `doc-guide/` | 実コードの解説（「実際にどう書いたか」） |

## 環境構築

### 開発環境（PlatformIO）
1. [VSCode](https://code.visualstudio.com/) をインストール
2. [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) 拡張機能をインストール
3. `sample/` または `verified/` フォルダをPlatformIOで開く

### ドキュメント環境（LaTeX）
1. VSCodeでDevContainerを起動（`.devcontainer/` 使用）
2. LaTeX Workshop拡張機能が自動でインストールされる
3. `doc/main.tex`、`doc-textbook/main.tex`、`doc-guide/main.tex` がそれぞれのエントリーポイント

## ビルド方法

### サンプルプロジェクト
```bash
cd sample/
pio run -e esp32-s3-cam-n16r8
```

### 仕様書
DevContainer内で `doc/main.tex` を開き、LaTeX Workshopでコンパイル。
