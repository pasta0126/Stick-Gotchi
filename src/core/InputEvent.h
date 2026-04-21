#pragma once
#include <stdint.h>

enum class ButtonId : uint8_t {
    A = 0,  // M5 front button — navigation / interaction
    B = 1,  // Side button    — back / cancel
    C = 2,  // Power button   — menu toggle (handled in main loop)
};

enum class ButtonAction : uint8_t {
    SHORT_PRESS,
    LONG_PRESS,
};

struct InputEvent {
    ButtonId     button;
    ButtonAction action;
    uint32_t     timestampMs;
};
