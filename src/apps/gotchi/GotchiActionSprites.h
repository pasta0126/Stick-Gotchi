#pragma once
#include <stdint.h>

// Action bar icons — 8×8 px, displayed at scale 2 (16×16) inside 40×20 buttons.
// Palette: 0=transparent  1=primary fill  2=highlight  3=shadow/outline

constexpr uint8_t ACTION_ICON_W     = 8;
constexpr uint8_t ACTION_ICON_H     = 8;
constexpr uint8_t ACTION_ICON_SCALE = 2;

// FEED — apple
constexpr uint8_t SPR_ACTION_FEED[8*8] = {
    0,0,0,1,1,0,0,0,
    0,0,0,0,1,0,0,0,
    0,0,1,1,1,1,0,0,
    0,1,1,2,1,1,1,0,
    0,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,0,
    0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,
};

// PLAY — bouncy ball
constexpr uint8_t SPR_ACTION_PLAY[8*8] = {
    0,0,1,1,1,1,0,0,
    0,1,1,2,2,1,1,0,
    0,1,2,1,1,1,1,0,
    0,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,0,
    0,1,1,1,3,1,1,0,
    0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,
};

// MED — medical cross
constexpr uint8_t SPR_ACTION_MED[8*8] = {
    0,0,0,1,1,0,0,0,
    0,0,0,1,1,0,0,0,
    0,1,1,1,1,1,1,0,
    0,1,1,2,2,1,1,0,
    0,1,1,1,1,1,1,0,
    0,0,0,1,1,0,0,0,
    0,0,0,1,1,0,0,0,
    0,0,0,0,0,0,0,0,
};

// LITE — lightbulb
constexpr uint8_t SPR_ACTION_LITE[8*8] = {
    0,0,1,1,1,0,0,0,
    0,1,1,2,1,1,0,0,
    0,1,1,1,1,1,0,0,
    0,1,1,1,1,1,0,0,
    0,0,1,1,1,0,0,0,
    0,0,3,1,3,0,0,0,
    0,0,0,3,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// BATH — water drop
constexpr uint8_t SPR_ACTION_BATH[8*8] = {
    0,0,0,1,0,0,0,0,
    0,0,1,1,1,0,0,0,
    0,1,1,2,1,1,0,0,
    0,1,1,1,1,1,0,0,
    0,1,1,1,1,1,0,0,
    0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

inline const uint8_t* gotchiActionIcon(uint8_t actionIdx) {
    switch (actionIdx) {
    case 0: return SPR_ACTION_FEED;
    case 1: return SPR_ACTION_PLAY;
    case 2: return SPR_ACTION_MED;
    case 3: return SPR_ACTION_LITE;
    case 4: return SPR_ACTION_BATH;
    default: return SPR_ACTION_FEED;
    }
}
