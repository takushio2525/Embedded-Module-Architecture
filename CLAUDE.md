# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 概要

ESP32-S3 N16R8 CAM開発ボードを題材にした、組み込みシステム向けModuleインターフェース設計パターンのリファレンス実装。PlatformIO + Arduino フレームワークを使用。

## ビルドコマンド

```bash
# ファームウェアビルド
pio run -e esp32-s3-cam-n16r8

# ビルド＋書き込み
pio run -e esp32-s3-cam-n16r8 -t upload

# シリアルモニター（115200bps）
pio device monitor
```

## ドキュメント（LaTeX仕様書）

DevContainer内でVSCode + LaTeX Workshopを使いコンパイル。

- エントリーポイント: `doc/main.tex`
- セクションファイル: `doc/sections/`

## アーキテクチャ

### コア層（プロジェクト非依存）

`lib/ModuleCore/` 以下の2ファイルで構成。他プロジェクトにそのまま流用可能。

| ファイル | 役割 |
|---|---|
| `lib/ModuleCore/IModule.h` | `IModule<T>` テンプレートインターフェース |
| `lib/ModuleCore/ModuleTimer.h` | `millis()` ベースのノンブロッキングタイマー |

`IModule<T>` はテンプレートパラメータ `T` を使うことで `SystemData` など任意の型に依存せず定義される。メソッドは `init()` と `update(T& data)` の2つのみ。

### プロジェクト層（プロジェクト固有）

| ファイル | 役割 |
|---|---|
| `include/ProjectConfig.h` | 全モジュールのConfigインスタンス（ピンアサイン）+ `SystemData` 構造体 |
| `src/main.cpp` | モジュール配列の定義・ループ制御 |
| `lib/{ModuleName}/` | 個別ハードウェアモジュールの実装 |

各モジュールのヘッダーファイル内で `{Module}Config` と `{Module}Data` の型を定義し、`ProjectConfig.h` でConfigインスタンスの値を定義する。

### 3フェーズ実行モデル

`loop()` は必ず以下の順序で実行する：

1. **入力フェーズ**: 全 `inputModules[]` の `update(systemData)` を順次呼び出し、センサー値を `SystemData` へ書き込む
2. **ロジックフェーズ**: `applyPattern(systemData)` 等の関数で入力データを読み、出力データを書き換える
3. **出力フェーズ**: 全 `outputModules[]` の `update(systemData)` を順次呼び出し、ハードウェアを制御する

入力モジュールと出力モジュールはクロス参照しない（出力モジュールが入力のデータを読む等は行わない）。

### 命名規則

| 要素 | 規則 | 例 |
|---|---|---|
| Config構造体の型名 | `{Module}Config` | `LedConfig`, `TempSensorConfig` |
| Configインスタンス名 | `{MODULE}_CONFIG` (大文字) | `LED_CONFIG`, `TEMP_CONFIG` |
| Data構造体の型名 | `{Module}Data` | `LedData`, `TempData` |

### 通信バスパターン（SPI / I2C）

- バスインスタンス（`SPIClass`, `TwoWire`）は `main.cpp` のグローバルスコープで生成する
- `bus.begin()` は `setup()` 内で、全モジュールの `init()` より前に呼ぶ
- バスポインタはConfig構造体に含めず、コンストラクタの引数でモジュールに渡す
- モジュールはバスポインタをメンバ変数（`_spi`, `_wire`等）として保持する
- モジュールの `init()` で `bus.begin()` を呼ばない（CSピン設定等のみ）
- SPI使用時は `beginTransaction()` / `endTransaction()` で排他制御する

### モジュール実装の要点

- コンストラクタは `const {Module}Config&` でConfigを受け取り、`_config` としてコピー保持する
- ピン番号はハードコードせず `_config` 経由でアクセスする
- `init()` は `bool` を返す（失敗時は `false`）
- `update()` でエラーが発生した場合は `SystemData` のサブ構造体にエラーフラグを設定する
- 周期実行にはメンバ変数として `ModuleTimer` を使い `delay()` を使用しない
