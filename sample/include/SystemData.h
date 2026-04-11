// SystemData.h — モジュール間共有データ構造体
#pragma once
#include "LedModule.h"
#include "DisplayBoardModule.h"
#include "Mpu6500Module.h"
#include "CameraModule.h"
#include "ServoModule.h"
#include "ChassisModule.h"
#include "ButtonModule.h"
#include "BleModule.h"
#include "BatteryModule.h"
#include "BuzzerModule.h"
#include "WifiModule.h"
#include "EncoderModule.h"

// ===== システムデータ =====
struct SystemData {
    LedData            led;
    DisplayBoardData   display;
    Mpu6500Data        mpu;
    CameraData         camera;
    ServoData          servo;
    ChassisData        chassis;
    ButtonData         button;
    BleData            ble;
    BatteryData        battery;
    BuzzerData         buzzer;
    WifiData           wifi;
    EncoderData        encoder;
};
