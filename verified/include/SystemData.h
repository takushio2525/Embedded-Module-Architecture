// SystemData.h — モジュール間共有データ構造体
// モジュール追加時に各Dataヘッダーのインクルードとメンバーを追加する
#pragma once
#include "DisplayBoardModule.h"
#include "Mpu6500Module.h"
#include "ServoModule.h"
#include "SdModule.h"
#include "CameraModule.h"
#include "BleModule.h"

// ===== システムデータ =====
struct SystemData {
    DisplayBoardData display;
    Mpu6500Data      mpu;
    ServoData        servo;
    SdData           sd;
    CameraData       camera;
    BleData          ble;
};
