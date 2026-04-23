#pragma once
#include <stdint.h>

enum class FlipState : uint8_t { IDLE, SPINNING, RESULT };

class FlipCoinGame {
public:
    void start();
    void onBtnA();
    void update(uint32_t deltaMs);

    FlipState state()     const { return _state; }
    bool      isHeads()   const { return _isHeads; }
    uint8_t   coinFrame() const { return _coinFrame; }

private:
    FlipState _state        = FlipState::IDLE;
    bool      _isHeads      = true;
    uint8_t   _coinFrame    = 0;
    uint8_t   _spinStep     = 0;
    uint8_t   _totalSteps   = 36;
    uint32_t  _frameAccumMs = 0;
    uint32_t  _resultMs     = 0;

    static constexpr uint32_t AUTO_RESET_MS = 3000;

    // CRUZ=frame0: 36 pasos (3 rot).  CARA=frame6: 42 pasos (3.5 rot)
    // ease-out siempre en los ultimos 10 pasos
    uint32_t _frameMsForStep(uint8_t step) const;
};
