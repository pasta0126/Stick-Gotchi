#pragma once
#include <stdint.h>

enum class BallState : uint8_t { IDLE, SHAKING, EASING, RESULT };

class Magic8BallGame {
public:
    void reset();
    void update(uint32_t deltaMs);
    void onAccelDelta(float delta);

    BallState state()    const { return _state; }
    uint8_t   frame()    const { return _frame; }
    uint8_t   resultId() const { return _resultId; }  // 1-12

private:
    BallState _state       = BallState::IDLE;
    uint8_t   _frame       = 0;
    uint8_t   _resultId    = 0;

    uint32_t  _lastShakeMs  = 0;
    uint32_t  _frameAccumMs = 0;
    uint32_t  _framePeriodMs = 120;

    uint8_t   _easeStep    = 0;
    static constexpr uint8_t  EASE_STEPS      = 20;
    static constexpr uint32_t SHAKE_TIMEOUT_MS = 600;

    uint32_t _easeFramePeriod(uint8_t step) const;
};
