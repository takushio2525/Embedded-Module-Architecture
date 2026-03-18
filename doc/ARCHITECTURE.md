# Embedded Module Architecture — AI リファレンス

ESP32-S3向け組み込みモジュール設計パターンのリファレンス実装。
PlatformIO + Arduino フレームワーク使用。

---

## 設計思想

**問題意識**: 組み込みの `loop()` にロジック・センサー読み込み・出力制御が混在すると、テストも移植も困難になる。

**解決策**: ハードウェアアクセスを「モジュール」に閉じ込め、モジュール間のデータ共有を `SystemData` 構造体（共有メモリ）に一本化する。ループは「入力 → ロジック → 出力」の3フェーズに固定し、データフローを単方向にする。

**移植性**: コア層（`IModule`, `ModuleTimer`）はプロジェクト非依存。`SystemData` の前方宣言のみを使用し、具体的な型定義を持たない。

---

## レイヤー構成

```
コア層（プロジェクト非依存）
  lib/ModuleCore/IModule.h        ← インターフェース定義
  lib/ModuleCore/ModuleTimer.h    ← millis()ベースノンブロッキングタイマー

プロジェクト層（プロジェクト固有）
  include/SystemData.h            ← 全モジュールのDataを集約
  include/ProjectConfig.h         ← 全モジュールのConfigインスタンス（ピンアサイン）
  src/main.cpp                    ← モジュール配列・3フェーズループ
  lib/{ModuleName}/               ← 個別モジュール実装
```

---

## IModule インターフェース

```cpp
struct SystemData;  // 前方宣言のみ（プロジェクト依存を避ける）

class IModule {
public:
    virtual bool init() = 0;               // 初期化。失敗時はfalse
    virtual void update(SystemData& data) = 0; // 毎ループ呼び出し
    virtual void deinit() {}               // リソース解放（デフォルト空実装）
    bool enabled = true;                   // false でupdate()スキップ
};
```

---

## 3フェーズ実行モデル

```cpp
void loop() {
    // 1. 入力フェーズ: センサー → SystemData
    for (auto* m : inputModules)  if (m->enabled) m->update(systemData);

    // 2. ロジックフェーズ: SystemDataを読み書き（判断・変換）
    applyPattern(systemData);

    // 3. 出力フェーズ: SystemData → ハードウェア
    for (auto* m : outputModules) if (m->enabled) m->update(systemData);
}
```

**制約**:
- 入力モジュールは `Data` を書き込む。出力モジュールは `Data` を読み込む。クロス参照禁止。
- `SystemData` への書き込みはフェーズ内のみ。割り込み・別タスクから直接触らない。
- Config変更（セッター等）はロジックフェーズ内のみ。

---

## モジュールの構造（.h/.cpp 分離）

### ヘッダー（宣言）

```cpp
// FooModule.h
#pragma once
#include <Arduino.h>
#include "IModule.h"
#include "ModuleTimer.h"

struct FooConfig {
    uint8_t pin;
    uint32_t intervalMs;
};

struct FooData {
    float value = 0.0f;   // デフォルト値必須
    bool isValid = false; // デフォルト値必須
};

// SystemData の前方宣言は IModule.h で済んでいる

class FooModule : public IModule {
public:
    explicit FooModule(const FooConfig& config);
    bool init() override;
    void update(SystemData& data) override;
    void deinit() override; // リソース解放が必要な場合のみ
private:
    FooConfig   _config;    // コピー保持
    ModuleTimer _timer;
};
```

**ルール**:
- `.h` で `SystemData.h` をインクルード**しない**（循環依存防止）
- バスポインタ（`TwoWire*`, `SPIClass*`）はコンストラクタ引数で受け取りメンバ保持

### 実装（.cpp）

```cpp
// FooModule.cpp
#include "FooModule.h"
#include "SystemData.h"  // ここでインクルード
// ProjectConfig.h はインクルードしない

bool FooModule::init() {
    pinMode(_config.pin, OUTPUT);
    _timer.setTime();
    return true;
}

void FooModule::update(SystemData& data) {
    if (_timer.getNowTime() < _config.intervalMs) return;
    _timer.setTime();
    data.foo.value = readHardware();
    data.foo.isValid = true;
}
```

---

## SystemData の集約

```cpp
// include/SystemData.h
#pragma once
#include "LedModule.h"
#include "TouchModule.h"
#include "TftModule.h"
#include "Mpu6500Module.h"
#include "CameraModule.h"

struct SystemData {
    LedData     led;
    TouchData   touch;
    TftData     tft;
    Mpu6500Data mpu;
    CameraData  camera;
};
```

モジュールを追加するたびに `SystemData.h` に `XxxData` フィールドを追加する。

---

## ProjectConfig.h の役割

全モジュールの Config インスタンスをここで定義する。モジュール `.cpp` からはインクルードしない（値は `main.cpp` 経由でコンストラクタに渡る）。

```cpp
// include/ProjectConfig.h
#pragma once
#include "SystemData.h"

const LedConfig LED_CONFIG = { .ledPin = 2 };
const Mpu6500Config MPU6500_CONFIG = { .address = 0x68, .sdaPin = 41, ... };
```

---

## 通信バスパターン（SPI / I2C）

```cpp
// main.cpp
static TwoWire  mpuWire   = TwoWire(0); // グローバルスコープで生成
static TFT_eSPI tftDriver;              // ライブラリ提供ドライバも同様

void setup() {
    mpuWire.begin(SDA, SCL);  // bus.begin() は全 init() より前
    tftDriver.init();          // ライブラリ提供ドライバの初期化も同じタイミング
    initModules(...);
}

// モジュール側
Mpu6500Module::Mpu6500Module(const Mpu6500Config& cfg, TwoWire* wire)
    : _config(cfg), _wire(wire) {}
// init() 内で bus.begin() / tftDriver.init() は呼ばない（設定・表示処理のみ）
```

ライブラリが提供するドライバオブジェクト（`TFT_eSPI` 等）も `bus.begin()` と同様に `setup()` で初期化し、ポインタをコンストラクタ引数でモジュールに渡す。

SPI使用時は `beginTransaction()` / `endTransaction()` で排他制御する。

---

## スレッドセーフティ

割り込み・別FreeRTOSタスク・BLEコールバックからの `SystemData` 直接アクセスは禁止。

**パターン**:
```cpp
// 割り込み/別タスク側
volatile bool _hasNewData = false;
SensorRaw _buffer;

void ISR_or_Task() {
    _buffer = readRaw();
    _hasNewData = true;
}

// update() 内
void FooModule::update(SystemData& data) {
    if (_hasNewData) {
        _hasNewData = false;
        data.foo.value = convert(_buffer);
    }
}
```

---

## init失敗とリカバリ

```cpp
// setup() での初期化
for (int r = 0; r < MAX_RETRY; r++) {
    if (module.init()) break;
    delay(100);
}
if (!success) module.enabled = false;

// ロジックフェーズでの定期再試行
if (!module.enabled && retryTimer.getNowTime() > 5000) {
    retryTimer.setTime();
    if (module.init()) module.enabled = true;
}
```

---

## 命名規則

| 要素 | 規則 | 例 |
|---|---|---|
| Config型 | `{Module}Config` | `LedConfig`, `Mpu6500Config` |
| Configインスタンス | `{MODULE}_CONFIG` | `LED_CONFIG`, `MPU6500_CONFIG` |
| Data型 | `{Module}Data` | `LedData`, `Mpu6500Data` |
| ログ | `[ModuleName] msg` | `[Camera] init failed` |

---

## platformio.ini の必須設定

```ini
build_flags =
    -I include   ; lib/ 内から include/ を参照するために必須
```

---

## 新規モジュール追加チェックリスト

1. `lib/{Name}Module/{Name}Module.h` — Config/Data構造体 + クラス宣言
2. `lib/{Name}Module/{Name}Module.cpp` — init()/update() 実装
3. `include/SystemData.h` — `{Name}Data` フィールドを追加
4. `include/ProjectConfig.h` — `{NAME}_CONFIG` インスタンスを追加
5. `src/main.cpp` — インスタンス生成 → `inputModules[]` or `outputModules[]` に追加
