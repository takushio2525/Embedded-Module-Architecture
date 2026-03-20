// SystemData.h — モジュール間共有データ構造体
// モジュール追加時に各Dataヘッダーのインクルードとメンバーを追加する
#pragma once
#include "TftModule.h"
#include "TouchModule.h"
#include "Mpu6500Module.h"

// ===== システムデータ =====
struct SystemData {
    TftData      tft;
    TouchData    touch;
    Mpu6500Data  mpu;
};
