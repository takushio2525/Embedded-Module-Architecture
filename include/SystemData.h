// SystemData.h — モジュール間共有データ構造体
#pragma once
#include "LedModule.h"
#include "TouchModule.h"
#include "TftModule.h"
#include "Mpu6500Module.h"
#include "CameraModule.h"
#include "ServoModule.h"
#include "DriveMotorModule.h"
#include "ChassisModule.h"

// ===== システムデータ =====
struct SystemData {
    LedData        led;
    TouchData      touch;
    TftData        tft;
    Mpu6500Data    mpu;
    CameraData     camera;
    ServoData      servo;
    DriveMotorData driveMotor;
    ChassisData    chassis;
};
