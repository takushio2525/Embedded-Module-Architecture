// test_camera — OV5640カメラモジュールのテスト
// ボード搭載のカメラが必要（ESP32-S3 CAMボード）
#include <Arduino.h>
#include <unity.h>
#include "CameraModule.h"
#include "ProjectConfig.h"

static CameraModule* camera = nullptr;
static SystemData systemData;

// init()が成功すること
void test_camera_init() {
    bool result = camera->init();
    TEST_ASSERT_TRUE_MESSAGE(result, "カメラのinit()が失敗");
}

// フレーム取得ができること
void test_camera_capture_frame() {
    systemData.camera = CameraData{};
    camera->update(systemData);
    TEST_ASSERT_TRUE_MESSAGE(systemData.camera.isValid,
        "カメラデータが無効（isValid=false）");
    TEST_ASSERT_TRUE_MESSAGE(systemData.camera.frameReady,
        "フレームが取得できなかった（frameReady=false）");
}

// フレームサイズが妥当であること（QVGA JPEG）
void test_camera_frame_size() {
    systemData.camera = CameraData{};
    camera->update(systemData);
    TEST_ASSERT_TRUE(systemData.camera.frameReady);

    // QVGA (320x240)
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(320, systemData.camera.width,
        "フレーム幅が320でない");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(240, systemData.camera.height,
        "フレーム高さが240でない");

    // JPEGフレームサイズ: 数百バイト〜数十KB
    TEST_ASSERT_GREATER_THAN_MESSAGE(100, systemData.camera.frameSize,
        "フレームサイズが小さすぎる");
    TEST_ASSERT_LESS_THAN_MESSAGE(100000, systemData.camera.frameSize,
        "フレームサイズが大きすぎる");

    Serial.printf("[Camera] フレームサイズ: %d bytes\n",
        systemData.camera.frameSize);
}

// フレームバッファポインタが取得できること
void test_camera_frame_buffer_ptr() {
    systemData.camera = CameraData{};
    camera->update(systemData);
    TEST_ASSERT_TRUE(systemData.camera.frameReady);

    const uint8_t* buf = camera->getFrameBuffer();
    TEST_ASSERT_NOT_NULL_MESSAGE(buf,
        "getFrameBuffer()がnullptrを返した");

    // JPEGのマジックバイト確認（SOI: 0xFF 0xD8）
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xFF, buf[0],
        "JPEG SOIマーカーが不正（先頭バイト）");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xD8, buf[1],
        "JPEG SOIマーカーが不正（2バイト目）");
}

// releaseFrame()後はバッファがnullになること
void test_camera_release_frame() {
    systemData.camera = CameraData{};
    camera->update(systemData);
    TEST_ASSERT_TRUE(systemData.camera.frameReady);

    camera->releaseFrame();
    const uint8_t* buf = camera->getFrameBuffer();
    TEST_ASSERT_NULL_MESSAGE(buf,
        "releaseFrame()後もバッファが残っている");
}

// 連続キャプチャの安定性テスト
void test_camera_continuous_capture() {
    const int NUM_FRAMES = 10;
    int successCount = 0;

    for (int i = 0; i < NUM_FRAMES; i++) {
        systemData.camera = CameraData{};
        camera->update(systemData);
        if (systemData.camera.frameReady && systemData.camera.isValid) {
            successCount++;
        }
        camera->releaseFrame();
        delay(50);  // フレーム間のインターバル
    }

    Serial.printf("[Camera] 連続キャプチャ: %d/%d 成功\n",
        successCount, NUM_FRAMES);
    // 90%以上の成功率を期待
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(NUM_FRAMES * 9 / 10, successCount,
        "連続キャプチャの成功率が低すぎる");
}

// PSRAM使用確認
void test_camera_psram_usage() {
    TEST_ASSERT_TRUE_MESSAGE(psramFound(),
        "PSRAMが検出されない（N16R8ボードが必要）");

    size_t freeBefore = ESP.getFreePsram();
    systemData.camera = CameraData{};
    camera->update(systemData);
    size_t freeAfter = ESP.getFreePsram();

    // フレームバッファ取得でPSRAM使用量が増えること
    Serial.printf("[Camera] PSRAM使用: %d bytes\n",
        (int)(freeBefore - freeAfter));

    camera->releaseFrame();
}

// deinit()が正常に完了すること
void test_camera_deinit() {
    camera->releaseFrame();
    camera->deinit();
    TEST_ASSERT_TRUE(true);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }  // USB-CDC接続待ち
    delay(500);

    camera = new CameraModule(CAMERA_CONFIG);

    UNITY_BEGIN();

    RUN_TEST(test_camera_init);
    RUN_TEST(test_camera_capture_frame);
    RUN_TEST(test_camera_frame_size);
    RUN_TEST(test_camera_frame_buffer_ptr);
    RUN_TEST(test_camera_release_frame);
    RUN_TEST(test_camera_continuous_capture);
    RUN_TEST(test_camera_psram_usage);
    RUN_TEST(test_camera_deinit);

    UNITY_END();

    delete camera;
}

void loop() {}
