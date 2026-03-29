// CameraModule.h — OV5640カメラモジュール（ESP32-S3 DVP接続）
// esp_camera ライブラリを使用
#pragma once
#include <Arduino.h>
#include "IModule.h"

// --- Config構造体 ---
// 注意: ピン番号はボードの配線に合わせて変更すること
struct CameraConfig {
    // DVPピンアサイン
    int8_t pwdnPin;    // POWER DOWN (-1 で未使用)
    int8_t resetPin;   // RESET      (-1 で未使用)
    int8_t xclkPin;    // クロック出力
    int8_t siodPin;    // SCCB SDA (カメラ設定用I2C)
    int8_t siocPin;    // SCCB SCL
    int8_t d0Pin;      // データバス D0-D7
    int8_t d1Pin;
    int8_t d2Pin;
    int8_t d3Pin;
    int8_t d4Pin;
    int8_t d5Pin;
    int8_t d6Pin;
    int8_t d7Pin;
    int8_t vsyncPin;   // 垂直同期
    int8_t hrefPin;    // 水平参照
    int8_t pclkPin;    // ピクセルクロック
    // カメラ動作設定
    int     xclkFreqHz;    // XCLKクロック周波数（通常20MHz）
    uint8_t frameSize;     // フレームサイズ (framesize_t の値)
    uint8_t pixFormat;     // ピクセルフォーマット (pixformat_t の値)
    int     jpegQuality;   // JPEG品質 0-63（低いほど高画質）
    size_t  fbCount;       // フレームバッファ数（PSRAMを使う場合は2以上推奨）
};

// --- Data構造体 ---
// 注意: フレームの生データはSystemDataに格納しない
//       CameraModule::getFrameBuffer() でアクセスすること
struct CameraData {
    bool     isValid    = false;
    bool     frameReady = false;  // 新しいフレームが取得済みであればtrue
    uint16_t width      = 0;
    uint16_t height     = 0;
    size_t   frameSize  = 0;      // バイト単位のフレームサイズ
};

// --- モジュール実装 ---
struct SystemData;

class CameraModule : public IModule {
public:
    explicit CameraModule(const CameraConfig& config);
    bool init()                         override;
    void updateInput(SystemData& data)  override;
    void deinit()                  override;

    // フレームバッファへのアクセス（logicフェーズでのみ使用）
    // 戻り値: フレームバッファポインタ（次のupdate()呼び出しまで有効）
    // 使用後は releaseFrame() を呼ぶこと
    const uint8_t* getFrameBuffer() const;
    void           releaseFrame();

private:
    CameraConfig _config;
    void*        _frameBuffer = nullptr;  // camera_fb_t* を void* で保持（ヘッダ依存回避）
    bool         _frameHeld   = false;
};
