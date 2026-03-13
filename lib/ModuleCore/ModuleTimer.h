// ModuleTimer.h — プロジェクト非依存のコアヘッダー
#pragma once
#include <Arduino.h>

class ModuleTimer {
private:
    unsigned long _startTime;

public:
    // コンストラクタ
    ModuleTimer() : _startTime(0) {}

    // 基準時刻をセット
    // 引数なし: getNowTime()が0から開始
    // 引数あり: getNowTime()が指定値から開始
    void setTime(unsigned long offsetMs = 0) {
        _startTime = millis() - offsetMs;
    }

    // 基準時刻からの経過時間(ms)を取得
    unsigned long getNowTime() const {
        return millis() - _startTime;
    }
};
