# Embedded Module Architecture

ESP32-S3 N16R8 CAM開発ボードを題材にした、組み込みシステム向けModuleインターフェース設計パターンのリファレンス実装。

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
```
pio run -e esp32-s3-cam-n16r8
```

### 仕様書
DevContainer内で `doc/main.tex` を開き、LaTeX Workshopでコンパイル。

## ドキュメント

詳細な仕様・設計情報は `doc/main.tex`（LaTeX仕様書）を参照してください。
