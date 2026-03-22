// test_touch — XPT2046タッチパネルモジュールのテスト
// TFT LCD + タッチパネルの実機接続が必要
// タッチ検出テストはユーザーの操作が必要
#include <Arduino.h>
#include <unity.h>
#include <TFT_eSPI.h>
#include "TouchModule.h"
#include "ProjectConfig.h"
#include "../test_utils.h"

static TFT_eSPI tftDriver;
static TouchModule* touch = nullptr;
static SystemData systemData;

// TFT画面にガイドメッセージを表示するヘルパー
static void showGuide(const char* line1, const char* line2 = nullptr,
                      uint16_t color = TFT_YELLOW) {
    tftDriver.fillScreen(TFT_BLACK);
    tftDriver.setTextColor(color, TFT_BLACK);
    tftDriver.drawString(line1, 20, 100, 2);
    if (line2) {
        tftDriver.drawString(line2, 20, 130, 2);
    }
}

// init()が成功すること
void test_touch_init() {
    bool result = touch->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "タッチのinit()が失敗");
}

// タッチなしの状態でtouchPressed=falseであること
void test_touch_no_press() {
    showGuide(">>> DO NOT TOUCH <<<", "Testing no-press state...");
    Serial.println("[Touch] 3秒間タッチしないでください...");
    delay(3000);

    systemData.touch = TouchData{};
    touch->update(systemData);
    delay(10);  // SPI安定待ち
    TEST_ASSERT_FALSE_MESSAGE(systemData.touch.touchPressed,
        "タッチしていないのにtouchPressed=true");
}

// タッチ検出テスト — タッチされるまで待つ（タイムアウト30秒）
void test_touch_detection() {
    showGuide(">>> TOUCH TEST <<<", "Touch the screen!");
    Serial.println("[Touch] 画面をタッチしてください（30秒以内）...");

    bool detected = false;
    unsigned long start = millis();
    while (millis() - start < 30000) {
        systemData.touch = TouchData{};
        touch->update(systemData);
        if (systemData.touch.touchPressed) {
            detected = true;
            Serial.printf("[Touch] 検出! X=%d, Y=%d\n",
                systemData.touch.touchX, systemData.touch.touchY);

            // タッチ位置を画面に表示
            char buf[48];
            snprintf(buf, sizeof(buf), "Touch: X=%d Y=%d",
                systemData.touch.touchX, systemData.touch.touchY);
            showGuide(buf, "DETECTED!", TFT_GREEN);
            delay(500);
            break;
        }
        delay(20);
    }

    TEST_ASSERT_TRUE_MESSAGE(detected,
        "30秒以内にタッチが検出されなかった");
}

// タッチ座標が画面範囲内であること
void test_touch_coordinate_range() {
    if (!systemData.touch.touchPressed) {
        Serial.println("[Touch] 座標範囲テスト: タッチ未検出のためスキップ");
        TEST_ASSERT_TRUE(true);
        return;
    }

    // rotation=1 (landscape) → 320x240
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(320, systemData.touch.touchX,
        "タッチX座標が画面幅を超えている");
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(240, systemData.touch.touchY,
        "タッチY座標が画面高さを超えている");
}

// 複数回タッチの安定性テスト — 3回タッチを要求
void test_touch_multi_press() {
    const int REQUIRED = 3;
    int count = 0;

    for (int i = 0; i < REQUIRED; i++) {
        char msg1[48], msg2[48];
        snprintf(msg1, sizeof(msg1), "Touch %d/%d", i + 1, REQUIRED);
        snprintf(msg2, sizeof(msg2), "Touch the screen!");
        showGuide(msg1, msg2);
        Serial.printf("[Touch] %d/%d 回目: 画面をタッチしてください...\n",
            i + 1, REQUIRED);

        // タッチ離し待ち
        unsigned long releaseStart = millis();
        while (millis() - releaseStart < 5000) {
            systemData.touch = TouchData{};
            touch->update(systemData);
            if (!systemData.touch.touchPressed) break;
            delay(20);
        }

        // タッチ待ち（15秒）
        bool detected = false;
        unsigned long start = millis();
        while (millis() - start < 15000) {
            systemData.touch = TouchData{};
            touch->update(systemData);
            if (systemData.touch.touchPressed) {
                detected = true;
                count++;
                Serial.printf("[Touch] %d回目 検出: X=%d, Y=%d\n",
                    i + 1, systemData.touch.touchX, systemData.touch.touchY);

                char result[48];
                snprintf(result, sizeof(result), "OK! X=%d Y=%d",
                    systemData.touch.touchX, systemData.touch.touchY);
                showGuide(result, nullptr, TFT_GREEN);
                delay(500);
                break;
            }
            delay(20);
        }

        if (!detected) {
            Serial.printf("[Touch] %d回目: タイムアウト\n", i + 1);
        }
    }

    Serial.printf("[Touch] 複数回タッチテスト: %d/%d 成功\n", count, REQUIRED);
    TEST_ASSERT_EQUAL_MESSAGE(REQUIRED, count,
        "すべてのタッチが検出されなかった");
}

// 連続update()でクラッシュしないこと
void test_touch_continuous_update() {
    for (int i = 0; i < 100; i++) {
        systemData.touch = TouchData{};
        touch->update(systemData);
    }
    TEST_ASSERT_TRUE(true);
    Serial.println("[Touch] 連続update()100回: OK");
}

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    // TFTドライバ初期化（TFT_eSPIが内部でSPIを管理するため、別途SPI.begin()は不要）
    tftDriver.init();
    tftDriver.setRotation(TFT_CONFIG.rotation);

    touch = new TouchModule(TOUCH_CONFIG, &tftDriver);

    Serial.println();
    Serial.println("========================================");
    Serial.println(" タッチパネル テスト（対話型）");
    Serial.println(" 画面の指示に従って操作してください。");
    Serial.println("========================================");

    UNITY_BEGIN();

    RUN_TEST(test_touch_init);
    RUN_TEST(test_touch_no_press);
    RUN_TEST(test_touch_detection);
    RUN_TEST(test_touch_coordinate_range);
    RUN_TEST(test_touch_multi_press);
    RUN_TEST(test_touch_continuous_update);

    UNITY_END();

    delete touch;
}

void loop() {}
