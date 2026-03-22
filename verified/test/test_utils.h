// test_utils.h — テスト用ユーティリティ
// シリアル経由でユーザー確認を待つヘルパー関数
#pragma once
#include <Arduino.h>

// シリアル入力バッファをクリアする
static inline void clearSerialBuffer() {
    while (Serial.available()) Serial.read();
}

// ユーザーにシリアル経由で確認を求める（y/n）
// prompt: 表示するメッセージ
// timeoutMs: タイムアウト [ms]（0=無制限）
// 戻り値: true='y'が入力された / false='n'またはタイムアウト
static bool waitForSerialConfirm(const char* prompt, uint32_t timeoutMs = 30000) {
    clearSerialBuffer();
    Serial.println();
    Serial.printf(">>> %s\n", prompt);
    Serial.println(">>> シリアルに 'y' を送信してください（'n' でスキップ）");

    unsigned long start = millis();
    while (timeoutMs == 0 || (millis() - start < timeoutMs)) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == 'y' || c == 'Y') {
                Serial.println(">>> 確認OK");
                return true;
            }
            if (c == 'n' || c == 'N') {
                Serial.println(">>> スキップ");
                return false;
            }
        }
        delay(10);
    }
    Serial.println(">>> タイムアウト（FAIL扱い）");
    return false;
}

// ユーザーのアクション（タッチ等）を待つ
// prompt: 表示するメッセージ
// timeoutMs: タイムアウト [ms]（0=無制限）
// checkFunc: 毎ループ呼ばれる関数。trueを返したら成功
// 戻り値: checkFuncがtrueを返したらtrue、タイムアウトでfalse
static bool waitForAction(const char* prompt, uint32_t timeoutMs,
                          bool (*checkFunc)()) {
    Serial.println();
    Serial.printf(">>> %s\n", prompt);

    unsigned long start = millis();
    while (timeoutMs == 0 || (millis() - start < timeoutMs)) {
        if (checkFunc()) {
            return true;
        }
        delay(20);
    }
    Serial.println(">>> タイムアウト");
    return false;
}
