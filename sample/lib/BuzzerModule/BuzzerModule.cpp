// BuzzerModule.cpp — ブザー出力モジュールの実装
#include "BuzzerModule.h"
#include "SystemData.h"

// PWM設定: 8bit分解能、デューティ50%
static constexpr uint8_t  BUZZER_PWM_RES  = 8;
static constexpr uint32_t BUZZER_DUTY_50  = 128;  // 2^7 = 50%

BuzzerModule::BuzzerModule(const BuzzerConfig& config) : _config(config) {}

bool BuzzerModule::init() {
    ledcSetup(_config.pwmChannel, 1000, BUZZER_PWM_RES);  // 初期1kHz
    ledcAttachPin(_config.pin, _config.pwmChannel);
    ledcWrite(_config.pwmChannel, 0);  // 消音状態で開始
    Serial.printf("[Buzzer] init OK (pin=%d, ch=%d)\n",
                  _config.pin, _config.pwmChannel);
    return true;
}

void BuzzerModule::updateOutput(SystemData& data) {
    // 再生リクエスト処理
    if (data.buzzer.requestPlay) {
        data.buzzer.requestPlay = false;

        if (data.buzzer.frequency > 0) {
            _toneOn(data.buzzer.frequency);
            if (data.buzzer.durationMs > 0) {
                _durationTimer.setTime();
                _timed = true;
            } else {
                _timed = false;  // 無制限再生
            }
        } else {
            _toneOff();
        }
    }

    // 時間指定ありの場合、経過したら自動消音
    if (_playing && _timed) {
        if (_durationTimer.getNowTime() >= data.buzzer.durationMs) {
            _toneOff();
        }
    }
}

void BuzzerModule::deinit() {
    // PWMリソースを解放（スリープ前やモジュール停止時に呼ぶ）
    _toneOff();
    ledcDetachPin(_config.pin);
    Serial.println("[Buzzer] deinit OK");
}

void BuzzerModule::_toneOn(uint16_t freq) {
    ledcWriteTone(_config.pwmChannel, freq);
    ledcWrite(_config.pwmChannel, BUZZER_DUTY_50);
    _playing = true;
}

void BuzzerModule::_toneOff() {
    ledcWrite(_config.pwmChannel, 0);
    _playing = false;
    _timed   = false;
}
