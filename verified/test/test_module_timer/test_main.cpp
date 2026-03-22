// test_module_timer — ModuleTimerのユニットテスト
// ハードウェア非依存: millis()のみ使用
#include <Arduino.h>
#include <unity.h>
#include "ModuleTimer.h"

// setTime()直後のgetNowTime()は0に近いこと
void test_timer_initial_value() {
    ModuleTimer timer;
    timer.setTime();
    unsigned long elapsed = timer.getNowTime();
    TEST_ASSERT_LESS_THAN(5, elapsed);  // 5ms以内
}

// delay後にgetNowTime()が経過時間を返すこと
void test_timer_elapsed() {
    ModuleTimer timer;
    timer.setTime();
    delay(100);
    unsigned long elapsed = timer.getNowTime();
    // 100ms±20msの範囲
    TEST_ASSERT_GREATER_OR_EQUAL(80, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(120, elapsed);
}

// setTime()で基準時刻がリセットされること
void test_timer_reset() {
    ModuleTimer timer;
    timer.setTime();
    delay(50);
    timer.setTime();  // リセット
    unsigned long elapsed = timer.getNowTime();
    TEST_ASSERT_LESS_THAN(5, elapsed);
}

// setTime(offset)でオフセット付き初期化ができること
void test_timer_offset() {
    ModuleTimer timer;
    timer.setTime(100);  // 100ms経過済みとして開始
    unsigned long elapsed = timer.getNowTime();
    // 100ms±5msの範囲
    TEST_ASSERT_GREATER_OR_EQUAL(95, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(105, elapsed);
}

// 周期判定パターン（モジュールの典型的な使い方）
void test_timer_interval_pattern() {
    ModuleTimer timer;
    const uint32_t interval = 50;  // 50ms周期

    timer.setTime();

    // 周期未達: まだ実行しない
    delay(30);
    TEST_ASSERT_TRUE(timer.getNowTime() < interval);

    // 周期到達: 実行する
    delay(25);
    TEST_ASSERT_TRUE(timer.getNowTime() >= interval);

    // リセット後、再び周期未達
    timer.setTime();
    TEST_ASSERT_TRUE(timer.getNowTime() < interval);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }  // USB-CDC接続待ち
    delay(500);

    UNITY_BEGIN();

    RUN_TEST(test_timer_initial_value);
    RUN_TEST(test_timer_elapsed);
    RUN_TEST(test_timer_reset);
    RUN_TEST(test_timer_offset);
    RUN_TEST(test_timer_interval_pattern);

    UNITY_END();
}

void loop() {}
