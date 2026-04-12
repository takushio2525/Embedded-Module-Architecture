// display_touch — TFT表示 + タッチ入力テスト
// 3フェーズモデルでタッチ座標を読み取り、画面に表示する
// 書き込み＋シリアルモニター:
//   cd verified && pio run -e test-display -t upload && pio device monitor
#include <Arduino.h>
#include "IModule.h"
#include "ProjectConfig.h"
#include "ModuleTimer.h"

// ===== モジュール・データ =====
DisplayBoardModule display(DISPLAY_BOARD_CONFIG);
SystemData systemData;

// シリアル出力用タイマー
static ModuleTimer printTimer;
static constexpr uint32_t PRINT_INTERVAL_MS = 200;

void setup()
{
    Serial.begin(115200);
    delay(3000); // USB-CDC再接続待ち

    Serial.println("[DisplayTest] 起動");

    // モジュール初期化
    if (!display.init())
    {
        Serial.println("[DisplayTest] init失敗");
        display.enabled = false;
    }

    printTimer.setTime();
    Serial.println("[DisplayTest] 開始（画面をタッチしてください）");
}

void loop()
{
    // 1. 入力フェーズ
    if (display.enabled)
    {
        display.updateInput(systemData);
    }

    // 2. ロジックフェーズ
    if (systemData.display.touchPressed)
    {
        snprintf(systemData.display.line1, sizeof(systemData.display.line1),
                 "Touch: X=%d Y=%d",
                 systemData.display.touchX, systemData.display.touchY);
    }
    else
    {
        strncpy(systemData.display.line1, "Touch: ---",
                sizeof(systemData.display.line1));
    }

    snprintf(systemData.display.line2, sizeof(systemData.display.line2),
             "Heap: %dK  PSRAM: %dK",
             (int)(ESP.getFreeHeap() / 1024),
             (int)(ESP.getFreePsram() / 1024));

    // シリアルにも出力
    if (printTimer.getNowTime() >= PRINT_INTERVAL_MS)
    {
        printTimer.setTime();

        if (systemData.display.touchPressed)
        {
            Serial.printf("Touch: X=%d Y=%d\n",
                          systemData.display.touchX,
                          systemData.display.touchY);
        }
    }

    // 3. 出力フェーズ
    if (display.enabled)
    {
        display.updateOutput(systemData);
    }
}
