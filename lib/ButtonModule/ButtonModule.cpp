// ButtonModule.cpp — ボタン入力モジュールの実装
#include "ButtonModule.h"
#include "SystemData.h"

ButtonModule::ButtonModule(const ButtonConfig& config) : _config(config) {}

bool ButtonModule::init() {
    // activeLow（プルアップ）の場合は INPUT_PULLUP を使用
    if (_config.activeLow) {
        pinMode(_config.pin, INPUT_PULLUP);
    } else {
        pinMode(_config.pin, INPUT);
    }
    _debounceTimer.setTime();
    Serial.printf("[Button] init OK (pin=%d, activeLow=%d)\n",
                  _config.pin, _config.activeLow);
    return true;
}

void ButtonModule::update(SystemData& data) {
    // 生の入力値を読み取り、activeLowの場合は反転
    bool rawPressed = digitalRead(_config.pin);
    if (_config.activeLow) {
        rawPressed = !rawPressed;
    }

    // 入力値が変化したらデバウンスタイマーをリセット
    if (rawPressed != _lastRaw) {
        _debounceTimer.setTime();
        _lastRaw = rawPressed;
    }

    // デバウンス安定時間を超えたら状態を確定
    if (_debounceTimer.getNowTime() >= _config.debounceMs) {
        _stableState = _lastRaw;
    }

    // エッジ検出
    data.button.pressed      = _stableState;
    data.button.justPressed  = (_stableState && !_prevStable);
    data.button.justReleased = (!_stableState && _prevStable);
    _prevStable = _stableState;
}
