#include "Magic8BallGame.h"
#include <Arduino.h>
#include <esp_random.h>

void Magic8BallGame::reset() {
    _state         = BallState::IDLE;
    _frame         = 0;
    _resultId      = 0;
    _lastShakeMs   = 0;
    _frameAccumMs  = 0;
    _framePeriodMs = 120;
    _easeStep      = 0;
}

void Magic8BallGame::onAccelDelta(float delta) {
    if (delta < 0.35f) return;

    _lastShakeMs = millis();

    if (_state == BallState::IDLE || _state == BallState::RESULT) {
        _state         = BallState::SHAKING;
        _framePeriodMs = 120;
        _frameAccumMs  = 0;
        _resultId      = 0;
    }
}

uint32_t Magic8BallGame::_easeFramePeriod(uint8_t step) const {
    float u = (float)step / (EASE_STEPS - 1);
    return (uint32_t)(60.0f + u * u * 300.0f);
}

void Magic8BallGame::update(uint32_t deltaMs) {
    switch (_state) {

    case BallState::SHAKING: {
        // ease-in towards 60ms/frame
        if (_framePeriodMs > 60)
            _framePeriodMs = (_framePeriodMs > 65) ? _framePeriodMs - 5 : 60;

        _frameAccumMs += deltaMs;
        if (_frameAccumMs >= _framePeriodMs) {
            _frameAccumMs -= _framePeriodMs;
            _frame = (_frame + 1) % 4;
        }

        if (millis() - _lastShakeMs >= SHAKE_TIMEOUT_MS) {
            _resultId     = (uint8_t)((esp_random() % 12) + 1);
            _easeStep     = 0;
            _framePeriodMs = 60;
            _frameAccumMs = 0;
            _state        = BallState::EASING;
        }
        break;
    }

    case BallState::EASING: {
        _frameAccumMs += deltaMs;
        uint32_t period = _easeFramePeriod(_easeStep);
        if (_frameAccumMs >= period) {
            _frameAccumMs -= period;
            _frame = (_frame + 1) % 4;
            if (++_easeStep >= EASE_STEPS)
                _state = BallState::RESULT;
        }
        break;
    }

    default: break;
    }
}
