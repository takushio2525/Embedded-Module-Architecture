// camera_capture — OV5640カメラ キャプチャテスト
// 3フェーズモデルでフレーム取得し、情報をシリアル出力する
#include <Arduino.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== モジュール・データ =====
CameraModule camera(CAMERA_CONFIG);
SystemData systemData;

// シリアル出力用タイマー
static ModuleTimer printTimer;
static constexpr uint32_t PRINT_INTERVAL_MS = 1000;

// フレームカウンタ
static uint32_t frameCount = 0;

void setup() {
    Serial.begin(115200);
    delay(3000);  // USB-CDC再接続待ち

    Serial.println("[CameraTest] 起動");

    // モジュール初期化
    if (!camera.init()) {
        Serial.println("[CameraTest] init失敗");
        camera.enabled = false;
    }

    printTimer.setTime();
    Serial.println("[CameraTest] キャプチャ開始");
}

void loop() {
    // 1. 入力フェーズ
    if (camera.enabled) {
        camera.update(systemData);
    }

    // 2. ロジックフェーズ
    if (systemData.camera.frameReady) {
        frameCount++;
    }

    if (printTimer.getNowTime() >= PRINT_INTERVAL_MS) {
        printTimer.setTime();

        if (systemData.camera.isValid) {
            Serial.printf("Camera: %dx%d  frame=%zu bytes  total=%lu frames  Heap=%dK PSRAM=%dK\n",
                          systemData.camera.width,
                          systemData.camera.height,
                          systemData.camera.frameSize,
                          frameCount,
                          (int)(ESP.getFreeHeap() / 1024),
                          (int)(ESP.getFreePsram() / 1024));
        } else {
            Serial.println("Camera: 無効");
        }
    }

    // フレームバッファ解放（ロジックフェーズ終了時）
    camera.releaseFrame();

    // 3. 出力フェーズ（なし）
}
