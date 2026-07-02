#pragma once
#include <stdint.h>

enum class ButtonId : uint8_t {
    A = 0,  // Front button — SHORT: select/confirm (positive input). LONG: reserved.
    B = 1,  // Side button  — SHORT: advance/navigate. LONG: cancel/back to CarouselHome.
    C = 2,  // Power button — power off / restart ONLY, never UI. Handled directly in main loop.
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
