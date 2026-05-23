#pragma once
#include <stdint.h>

enum class GotchiMood : uint8_t {
    NEUTRAL  = 0,
    HAPPY    = 1,
    SICK     = 2,
    PENSIVE  = 3,
    SAD      = 4,
    SLEEPING = 5,
    EXCITED  = 6,
    LAUGHING = 7,
    DIZZY    = 8,
    ANNOYED  = 9,
    ANGRY    = 10,
    STARTLED = 11,
    SCARED   = 12,
    _KEEP    = 0xFF,  // sentinel: do not change current mood when reaction fires
};
