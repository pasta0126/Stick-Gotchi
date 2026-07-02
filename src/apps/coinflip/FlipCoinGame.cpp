#include "FlipCoinGame.h"
#include <esp_random.h>

void FlipCoinGame::start() {
    _isHeads      = (esp_random() & 1);
    _coinFrame    = 0;
    _spinStep     = 0;
    _frameAccumMs = 0;
    _resultMs     = 0;
    // alinear total de pasos con el frame de resultado (partiendo siempre de frame 0)
    // CRUZ=frame0: (0+N)%12==0 → N multiplo de 12 → 36
    // CARA=frame6: (0+N)%12==6 → N=12k+6       → 42
    _totalSteps   = _isHeads ? 66 : 60;
    _state        = FlipState::SPINNING;
}

void FlipCoinGame::onBtnA() {
    if (_state == FlipState::IDLE)
        start();
    else if (_state == FlipState::RESULT)
        start();
}

uint32_t FlipCoinGame::_frameMsForStep(uint8_t step) const {
    // ease-in: primeros 4 pasos (150ms→35ms)
    if (step < 4) {
        float u = step / 4.0f;
        return (uint32_t)(150.0f * (1.0f - u) + 35.0f * u);
    }
    // ease-out: ultimos 10 pasos (35ms→200ms cuadratico)
    int easeOutStart = _totalSteps - 10;
    if (step >= easeOutStart) {
        float u = (float)(step - easeOutStart) / 9.0f;
        return (uint32_t)(35.0f + u * u * 165.0f);
    }
    // fase rapida central
    return 35;
}

void FlipCoinGame::update(uint32_t deltaMs) {
    switch (_state) {
    case FlipState::SPINNING: {
        _frameAccumMs += deltaMs;
        uint32_t stepMs = _frameMsForStep(_spinStep);
        if (_frameAccumMs >= stepMs) {
            _frameAccumMs -= stepMs;
            _coinFrame = (_coinFrame + 1) % 12;
            if (++_spinStep >= _totalSteps) {
                _coinFrame    = _isHeads ? 6 : 0;
                _state        = FlipState::RESULT;
                _resultMs     = 0;
                _frameAccumMs = 0;
            }
        }
        break;
    }

    case FlipState::RESULT:
        _resultMs += deltaMs;
        if (_resultMs >= AUTO_RESET_MS)
            _state = FlipState::IDLE;
        break;

    default:
        break;
    }
}
