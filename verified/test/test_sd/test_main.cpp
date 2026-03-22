// test_sd — SDカードモジュールのテスト
// SDカード挿入 + SPI接続が必要
#include <Arduino.h>
#include <unity.h>
#include <SPI.h>
#include <SD.h>
#include "SdModule.h"
#include "ProjectConfig.h"

static SPIClass testSpi = SPIClass(FSPI);
static SdModule* sdModule = nullptr;
static SystemData systemData;

// init()が成功すること（SDカード検出）
void test_sd_init() {
    bool result = sdModule->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "SDカードのinit()が失敗（カード未挿入？）");
}

// カード種別が有効であること
void test_sd_card_type() {
    uint8_t cardType = SD.cardType();
    TEST_ASSERT_NOT_EQUAL_MESSAGE(CARD_NONE, cardType,
        "カード種別がCARD_NONE");
    // SD / SDHC / SDXC のいずれか
    TEST_ASSERT_TRUE_MESSAGE(
        cardType == CARD_SD || cardType == CARD_SDHC,
        "カード種別が不明");
    Serial.printf("[SD] カード種別: %s\n",
        cardType == CARD_SD ? "SD" : "SDHC");
}

// カード容量が取得できること
void test_sd_capacity() {
    systemData.sd = SdData{};
    sdModule->update(systemData);
    TEST_ASSERT_TRUE(systemData.sd.isValid);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, systemData.sd.totalBytes,
        "カード容量が0");
    Serial.printf("[SD] 容量: %lluMB, 使用: %lluMB\n",
        systemData.sd.totalBytes / (1024 * 1024),
        systemData.sd.usedBytes / (1024 * 1024));
}

// ファイル書き込みテスト
void test_sd_write_file() {
    const char* path = "/test_write.txt";
    File file = SD.open(path, FILE_WRITE);
    TEST_ASSERT_TRUE_MESSAGE(file, "テストファイルのオープンに失敗");
    size_t written = file.println("Hello from Module Test Bench");
    file.close();
    TEST_ASSERT_GREATER_THAN(0, written);

    // クリーンアップ
    SD.remove(path);
}

// ファイル読み書き整合性テスト
void test_sd_read_write_consistency() {
    const char* path = "/test_rw.txt";
    const char* testStr = "TestData12345";

    // 書き込み
    File wf = SD.open(path, FILE_WRITE);
    TEST_ASSERT_TRUE(wf);
    wf.print(testStr);
    wf.close();

    // 読み込み
    File rf = SD.open(path, FILE_READ);
    TEST_ASSERT_TRUE(rf);
    String readBack = rf.readString();
    rf.close();

    TEST_ASSERT_EQUAL_STRING(testStr, readBack.c_str());

    SD.remove(path);
}

// 大きなファイルの書き込みテスト（バス共有の安定性確認）
void test_sd_large_write() {
    const char* path = "/test_large.bin";
    File file = SD.open(path, FILE_WRITE);
    TEST_ASSERT_TRUE(file);

    // 4KBのデータを書き込み
    uint8_t buf[256];
    for (int i = 0; i < 256; i++) buf[i] = (uint8_t)i;

    size_t totalWritten = 0;
    for (int i = 0; i < 16; i++) {  // 256 * 16 = 4096 bytes
        totalWritten += file.write(buf, sizeof(buf));
    }
    file.close();
    TEST_ASSERT_EQUAL(4096, totalWritten);

    // 読み戻して先頭を検証
    file = SD.open(path, FILE_READ);
    TEST_ASSERT_TRUE(file);
    uint8_t readBuf[256];
    file.read(readBuf, sizeof(readBuf));
    file.close();

    for (int i = 0; i < 256; i++) {
        TEST_ASSERT_EQUAL_UINT8(i, readBuf[i]);
    }

    SD.remove(path);
}

// モジュールの読み書きテスト（SdModule内蔵テスト）
void test_sd_module_rw_test() {
    // updateでtestCompletedがtrueになり、testPassedがtrueになること
    systemData.sd = SdData{};
    systemData.sd.isValid = true;
    sdModule->update(systemData);
    TEST_ASSERT_TRUE_MESSAGE(systemData.sd.testCompleted,
        "SdModuleの内蔵テストが完了しなかった");
    TEST_ASSERT_TRUE_MESSAGE(systemData.sd.testPassed,
        "SdModuleの内蔵R/Wテストが失敗");
}

// deinit()が正常に完了すること
void test_sd_deinit() {
    sdModule->deinit();
    // deinit後は特にクラッシュしないことを確認
    TEST_ASSERT_TRUE(true);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }  // USB-CDC接続待ち
    delay(500);

    // 共有SPIバス初期化
    testSpi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);

    sdModule = new SdModule(SD_CONFIG, &testSpi);

    UNITY_BEGIN();

    RUN_TEST(test_sd_init);
    RUN_TEST(test_sd_card_type);
    RUN_TEST(test_sd_capacity);
    RUN_TEST(test_sd_write_file);
    RUN_TEST(test_sd_read_write_consistency);
    RUN_TEST(test_sd_large_write);
    RUN_TEST(test_sd_module_rw_test);
    RUN_TEST(test_sd_deinit);

    UNITY_END();

    delete sdModule;
}

void loop() {}
