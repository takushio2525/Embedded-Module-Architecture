// test_tft — TFT LCDモジュールのテスト
// ILI9341 2.8インチTFT LCDの実機接続が必要
// 各描画テストでシリアル経由のユーザー確認を待つ
#include <Arduino.h>
#include <unity.h>
#include <TFT_eSPI.h>
#include "TftModule.h"
#include "ProjectConfig.h"
#include "../test_utils.h"

static TFT_eSPI tftDriver;
static TftModule* tft = nullptr;
static SystemData systemData;

// TFT_eSPIドライバの初期化が成功すること
void test_tft_driver_init() {
    tftDriver.init();
    // init()でクラッシュしなければOK
    TEST_ASSERT_TRUE(true);
}

// TftModuleのinit()が成功すること
void test_tft_module_init() {
    bool result = tft->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "TFTのinit()が失敗");
}

// 画面サイズが正しいこと（rotation=1で320x240）
void test_tft_screen_size() {
    int16_t w = tftDriver.width();
    int16_t h = tftDriver.height();
    TEST_ASSERT_EQUAL_INT16_MESSAGE(320, w,
        "画面幅が320でない（rotation設定を確認）");
    TEST_ASSERT_EQUAL_INT16_MESSAGE(240, h,
        "画面高さが240でない（rotation設定を確認）");
}

// 赤で塗りつぶし → ユーザー確認
void test_tft_fill_red() {
    tftDriver.fillScreen(TFT_RED);
    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm("画面全体が「赤」で表示されていますか？"),
        "赤塗りつぶしの目視確認に失敗");
}

// 緑で塗りつぶし → ユーザー確認
void test_tft_fill_green() {
    tftDriver.fillScreen(TFT_GREEN);
    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm("画面全体が「緑」で表示されていますか？"),
        "緑塗りつぶしの目視確認に失敗");
}

// 青で塗りつぶし → ユーザー確認
void test_tft_fill_blue() {
    tftDriver.fillScreen(TFT_BLUE);
    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm("画面全体が「青」で表示されていますか？"),
        "青塗りつぶしの目視確認に失敗");
}

// テキスト描画 → ユーザー確認
void test_tft_draw_text() {
    tftDriver.fillScreen(TFT_BLACK);
    tftDriver.setTextColor(TFT_WHITE, TFT_BLACK);
    tftDriver.drawString("TFT Test OK!", 10, 10, 2);
    tftDriver.drawString("Line 2: ABCDEFG", 10, 34, 2);
    tftDriver.setTextColor(TFT_YELLOW, TFT_BLACK);
    tftDriver.drawString("Yellow Text", 10, 58, 2);
    tftDriver.setTextColor(TFT_CYAN, TFT_BLACK);
    tftDriver.drawString("Cyan Text", 10, 82, 2);

    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm(
            "4行のテキストが表示されていますか？"
            "（白2行, 黄1行, シアン1行）"),
        "テキスト描画の目視確認に失敗");
}

// update()で5行表示 → ユーザー確認
void test_tft_module_update() {
    strncpy(systemData.tft.line1, "Line1: Accel Data     ", sizeof(systemData.tft.line1));
    strncpy(systemData.tft.line2, "Line2: Gyro Data      ", sizeof(systemData.tft.line2));
    strncpy(systemData.tft.line3, "Line3: Touch Info     ", sizeof(systemData.tft.line3));
    strncpy(systemData.tft.line4, "Line4: Status         ", sizeof(systemData.tft.line4));
    strncpy(systemData.tft.line5, "Line5: Memory Info    ", sizeof(systemData.tft.line5));

    // updateIntervalMs経過後に描画されるよう2回呼び出す
    tft->update(systemData);
    delay(TFT_CONFIG.updateIntervalMs + 10);
    tft->update(systemData);

    TEST_ASSERT_TRUE_MESSAGE(
        waitForSerialConfirm(
            "5行のテスト文字列が表示されていますか？"
            "（Line1〜Line5）"),
        "update()による5行表示の目視確認に失敗");
}

// deinit()が正常に完了すること
void test_tft_deinit() {
    tft->deinit();
    TEST_ASSERT_TRUE(true);
}

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    // TFT_eSPIが内部でSPIを管理するため、別途SPI.begin()は不要
    tft = new TftModule(TFT_CONFIG, &tftDriver);

    Serial.println();
    Serial.println("========================================");
    Serial.println(" TFT LCD テスト（対話型）");
    Serial.println(" 各テストで画面表示を確認し、");
    Serial.println(" シリアルに 'y' を送信してください。");
    Serial.println("========================================");

    UNITY_BEGIN();

    RUN_TEST(test_tft_driver_init);
    RUN_TEST(test_tft_module_init);
    RUN_TEST(test_tft_screen_size);
    RUN_TEST(test_tft_fill_red);
    RUN_TEST(test_tft_fill_green);
    RUN_TEST(test_tft_fill_blue);
    RUN_TEST(test_tft_draw_text);
    RUN_TEST(test_tft_module_update);
    RUN_TEST(test_tft_deinit);

    UNITY_END();

    delete tft;
}

void loop() {}
