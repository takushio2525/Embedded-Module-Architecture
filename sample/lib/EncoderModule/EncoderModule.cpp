// EncoderModule.cpp — ロータリーエンコーダ入力モジュールの実装
#include "EncoderModule.h"
#include "SystemData.h"

// 割り込み用シングルインスタンス参照
// 注意: 複数インスタンスを使う場合はインスタンス配列に拡張すること
EncoderModule* EncoderModule::_instance = nullptr;

EncoderModule::EncoderModule(const EncoderConfig& config) : _config(config) {}

bool EncoderModule::init() {
    // インスタンス参照を登録（割り込みハンドラから使用）
    _instance = this;

    // エンコーダピンをプルアップ付き入力に設定
    pinMode(_config.pinA, INPUT_PULLUP);
    pinMode(_config.pinB, INPUT_PULLUP);

    // A相の立ち上がりエッジで割り込み
    attachInterrupt(digitalPinToInterrupt(_config.pinA), _isrHandlerA, RISING);

    _sampleTimer.setTime();
    Serial.printf("[Encoder] init OK (A=%d, B=%d, PPR=%d)\n",
                  _config.pinA, _config.pinB, _config.pulsesPerRev);
    return true;
}

void EncoderModule::update(SystemData& data) {
    // volatile変数を一度だけ読み取り（割り込みとの競合を最小化）
    int32_t currentCount = _isrCount;

    // 累積カウント値をSystemDataにコピー
    data.encoder.count = currentCount;

    // 速度計算（一定間隔で実行）
    if (_sampleTimer.getNowTime() >= _config.sampleIntervalMs) {
        unsigned long elapsed = _sampleTimer.getNowTime();
        _sampleTimer.setTime();

        int32_t delta = currentCount - _prevCount;
        _prevCount = currentCount;

        // rpm = (パルス差分 / PPR) / (経過秒) * 60
        float revolutions = (float)delta / _config.pulsesPerRev;
        float seconds     = (float)elapsed / 1000.0f;
        data.encoder.rpm     = (seconds > 0) ? (revolutions / seconds * 60.0f) : 0.0f;
        data.encoder.isValid = true;
    }
}

void EncoderModule::deinit() {
    detachInterrupt(digitalPinToInterrupt(_config.pinA));
    _instance = nullptr;
    Serial.println("[Encoder] deinit OK");
}

// --- 割り込みハンドラ ---

void IRAM_ATTR EncoderModule::_isrHandlerA() {
    if (_instance) {
        _instance->_onPulseA();
    }
}

void IRAM_ATTR EncoderModule::_onPulseA() {
    // A相の立ち上がり時にB相を読み取って回転方向を判定
    if (digitalRead(_config.pinB) == HIGH) {
        _isrCount++;   // CW（時計回り）
    } else {
        _isrCount--;   // CCW（反時計回り）
    }
}
