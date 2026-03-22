// test_touch — XPT2046タッチパネルモジュールのテスト
// TFT LCD + タッチパネルの実機接続が必要
// タッチ検出テストはユーザーの操作が必要
#include <Arduino.h>
#include <unity.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "TouchModule.h"
#include "ProjectConfig.h"

static SPIClass sharedSpi = SPIClass(FSPI);
static TFT_eSPI tftDriver;
static TouchModule* touch = nullptr;
static SystemData systemData;

// init()が成功すること
void test_touch_init() {
    bool result = touch->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "タッチのinit()が失敗");
}

// タッチなしの状態でtouchPressed=falseであること
void test_touch_no_press() {
    systemData.touch = TouchData{};
    touch->update(systemData);
    // タッチしていなければfalse
    // ※ テスト中にタッチしていないことが前提
    TEST_ASSERT_FALSE_MESSAGE(systemData.touch.touchPressed,
        "タッチしていないのにtouchPressed=true"
        "（テスト中は画面に触れないでください）");
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

// タッチ検出の対話テスト
// ※ 5秒以内に画面をタッチする必要がある
void test_touch_detection_interactive() {
    // TFT画面にガイド表示
    tftDriver.fillScreen(TFT_BLACK);
    tftDriver.setTextColor(TFT_YELLOW, TFT_BLACK);
    tftDriver.drawString(">>> TOUCH TEST <<<", 40, 80, 2);
    tftDriver.drawString("Touch the screen!", 50, 110, 2);
    tftDriver.drawString("(within 5 sec)", 60, 140, 2);

    Serial.println("[Touch] 5秒以内に画面をタッチしてください...");

    bool detected = false;
    unsigned long start = millis();
    while (millis() - start < 5000) {
        systemData.touch = TouchData{};
        touch->update(systemData);
        if (systemData.touch.touchPressed) {
            detected = true;
            Serial.printf("[Touch] 検出! X=%d, Y=%d\n",
                systemData.touch.touchX, systemData.touch.touchY);

            // タッチ位置を画面に表示
            tftDriver.fillScreen(TFT_BLACK);
            char buf[48];
            snprintf(buf, sizeof(buf), "Touch: X=%d Y=%d",
                systemData.touch.touchX, systemData.touch.touchY);
            tftDriver.setTextColor(TFT_GREEN, TFT_BLACK);
            tftDriver.drawString(buf, 20, 100, 2);
            tftDriver.drawString("PASS!", 120, 130, 4);
            delay(1000);
            break;
        }
        delay(20);
    }

    TEST_ASSERT_TRUE_MESSAGE(detected,
        "5秒以内にタッチが検出されなかった");
}

// タッチ座標が画面範囲内であること
void test_touch_coordinate_range() {
    // 前のテストでタッチ検出済みの場合、その座標を検証
    // タッチしていなければスキップ
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

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    // 共有SPIバス・TFTドライバ初期化
    sharedSpi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);
    tftDriver.init();
    tftDriver.setRotation(TFT_CONFIG.rotation);

    touch = new TouchModule(TOUCH_CONFIG, &tftDriver);

    UNITY_BEGIN();

    RUN_TEST(test_touch_init);
    RUN_TEST(test_touch_no_press);
    RUN_TEST(test_touch_continuous_update);
    RUN_TEST(test_touch_detection_interactive);
    RUN_TEST(test_touch_coordinate_range);

    UNITY_END();

    delete touch;
}

void loop() {}
