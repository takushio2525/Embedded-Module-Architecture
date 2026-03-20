// CameraModule.cpp — OV5640カメラモジュール実装
#include "CameraModule.h"
#include "SystemData.h"
#include "esp_camera.h"

CameraModule::CameraModule(const CameraConfig& config) : _config(config) {}

bool CameraModule::init() {
    // camera_config_t を CameraConfig から構築
    camera_config_t cfg;
    cfg.pin_pwdn      = _config.pwdnPin;
    cfg.pin_reset     = _config.resetPin;
    cfg.pin_xclk      = _config.xclkPin;
    cfg.pin_sccb_sda  = _config.siodPin;
    cfg.pin_sccb_scl  = _config.siocPin;
    cfg.pin_d0        = _config.d0Pin;
    cfg.pin_d1        = _config.d1Pin;
    cfg.pin_d2        = _config.d2Pin;
    cfg.pin_d3        = _config.d3Pin;
    cfg.pin_d4        = _config.d4Pin;
    cfg.pin_d5        = _config.d5Pin;
    cfg.pin_d6        = _config.d6Pin;
    cfg.pin_d7        = _config.d7Pin;
    cfg.pin_vsync     = _config.vsyncPin;
    cfg.pin_href      = _config.hrefPin;
    cfg.pin_pclk      = _config.pclkPin;

    cfg.xclk_freq_hz  = _config.xclkFreqHz;
    cfg.ledc_timer    = LEDC_TIMER_1;    // TIMER_0はサーボPWMが使用
    cfg.ledc_channel  = LEDC_CHANNEL_1;  // CHANNEL_0はサーボPWMが使用

    cfg.pixel_format  = (pixformat_t)_config.pixFormat;
    cfg.frame_size    = (framesize_t)_config.frameSize;
    cfg.jpeg_quality  = _config.jpegQuality;
    cfg.fb_count      = _config.fbCount;

    // PSRAMが利用可能な場合はフレームバッファをPSRAMに配置
    cfg.fb_location   = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
    cfg.grab_mode     = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        Serial.printf("[Camera] esp_camera_init failed: 0x%x\n", err);
        return false;
    }

    // センサー設定（OV5640向けの初期調整）
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor) {
        sensor->set_vflip(sensor, 0);
        sensor->set_hmirror(sensor, 0);
    }

    Serial.printf("[Camera] init OK (PSRAM: %s)\n", psramFound() ? "yes" : "no");
    return true;
}

void CameraModule::deinit() {
    releaseFrame();
    esp_camera_deinit();
    Serial.println("[Camera] deinit");
}

void CameraModule::update(SystemData& data) {
    // 前フレームを解放
    releaseFrame();

    // 新しいフレームを取得
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        data.camera.frameReady = false;
        data.camera.isValid    = false;
        Serial.println("[Camera] esp_camera_fb_get failed");
        return;
    }

    _frameBuffer = fb;
    _frameHeld   = true;

    // メタデータのみSystemDataへ書き込む
    data.camera.frameReady = true;
    data.camera.isValid    = true;
    data.camera.width      = fb->width;
    data.camera.height     = fb->height;
    data.camera.frameSize  = fb->len;
}

const uint8_t* CameraModule::getFrameBuffer() const {
    if (!_frameHeld || !_frameBuffer) return nullptr;
    return reinterpret_cast<camera_fb_t*>(_frameBuffer)->buf;
}

void CameraModule::releaseFrame() {
    if (_frameHeld && _frameBuffer) {
        esp_camera_fb_return(reinterpret_cast<camera_fb_t*>(_frameBuffer));
        _frameBuffer = nullptr;
        _frameHeld   = false;
    }
}
