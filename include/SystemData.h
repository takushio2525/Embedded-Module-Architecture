// SystemData.h — モジュール間共有データ構造体
#pragma once
#include "LedModule.h"
#include "TftModule.h"
#include "Mpu6500Module.h"
#include "CameraModule.h"

// ===== システムデータ =====
struct SystemData {
    LedData    led;
    TftData    tft;
    Mpu6500Data mpu;
    CameraData  camera;
};
