// test_tft — TFT LCDモジュールのテスト
// ILI9341 2.8インチTFT LCDの実機接続が必要
// 表示内容の正しさは目視確認
#include <Arduino.h>
#include <unity.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "TftModule.h"
#include "ProjectConfig.h"

static SPIClass sharedSpi = SPIClass(FSPI);
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

// 画面全体を色で塗りつぶせること（目視確認）
void test_tft_fill_colors() {
    // 赤
    tftDriver.fillScreen(TFT_RED);
    delay(300);
    // 緑
    tftDriver.fillScreen(TFT_GREEN);
    delay(300);
    // 青
    tftDriver.fillScreen(TFT_BLUE);
    delay(300);
    // 黒に戻す
    tftDriver.fillScreen(TFT_BLACK);
    delay(200);
    // クラッシュしなければOK（色の正しさは目視確認）
    TEST_ASSERT_TRUE(true);
    Serial.println("[TFT] 画面塗りつぶしテスト完了（赤→緑→青→黒）");
}

// テキスト描画ができること（目視確認）
void test_tft_draw_text() {
    tftDriver.fillScreen(TFT_BLACK);
    tftDriver.setTextColor(TFT_WHITE, TFT_BLACK);
    tftDriver.drawString("TFT Test OK!", 10, 10, 2);
    tftDriver.drawString("Line 2: ABCDEFG", 10, 34, 2);
    tftDriver.setTextColor(TFT_YELLOW, TFT_BLACK);
    tftDriver.drawString("Yellow Text", 10, 58, 2);
    delay(500);
    TEST_ASSERT_TRUE(true);
    Serial.println("[TFT] テキスト描画テスト完了（目視確認してください）");
}

// update()で表示が更新されること
void test_tft_module_update() {
    // テスト用データを設定
    strncpy(systemData.tft.line1, "Test Line 1: Hello", sizeof(systemData.tft.line1));
    strncpy(systemData.tft.line2, "Test Line 2: World", sizeof(systemData.tft.line2));
    strncpy(systemData.tft.line3, "Test Line 3: Module", sizeof(systemData.tft.line3));
    strncpy(systemData.tft.line4, "Test Line 4: Arch", sizeof(systemData.tft.line4));
    strncpy(systemData.tft.line5, "Test Line 5: OK!", sizeof(systemData.tft.line5));

    tft->update(systemData);
    delay(200);
    // updateIntervalMs経過後に再度呼び出し
    delay(TFT_CONFIG.updateIntervalMs);
    tft->update(systemData);
    delay(500);

    TEST_ASSERT_TRUE(true);
    Serial.println("[TFT] update()テスト完了（5行表示を目視確認してください）");
}

// deinit()が正常に完了すること
void test_tft_deinit() {
    tft->deinit();
    TEST_ASSERT_TRUE(true);
}

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    // 共有SPIバス初期化
    sharedSpi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);

    tft = new TftModule(TFT_CONFIG, &tftDriver);

    UNITY_BEGIN();

    RUN_TEST(test_tft_driver_init);
    RUN_TEST(test_tft_module_init);
    RUN_TEST(test_tft_screen_size);
    RUN_TEST(test_tft_fill_colors);
    RUN_TEST(test_tft_draw_text);
    RUN_TEST(test_tft_module_update);
    RUN_TEST(test_tft_deinit);

    UNITY_END();

    delete tft;
}

void loop() {}
