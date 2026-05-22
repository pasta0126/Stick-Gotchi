#pragma once
#include <stdint.h>

// HUD stat indicator icons — 6×6 px, scale 1 — placed before each gauge bar.
// Palette: 0=transparent  1=primary fill  2=highlight

constexpr uint8_t HUD_ICON_W = 6;
constexpr uint8_t HUD_ICON_H = 6;

// HP — heart
constexpr uint8_t SPR_HUD_HP[6*6] = {
    0,1,0,0,1,0,
    1,1,1,1,1,1,
    1,2,1,1,1,1,
    0,1,1,1,1,0,
    0,0,1,1,0,0,
    0,0,0,0,0,0,
};

// HUNGER — apple
constexpr uint8_t SPR_HUD_HUNGER[6*6] = {
    0,0,1,1,0,0,
    0,1,1,1,1,0,
    0,1,2,1,1,0,
    0,1,1,1,1,0,
    0,0,1,1,0,0,
    0,0,0,0,0,0,
};

// ENERGY — lightning bolt
constexpr uint8_t SPR_HUD_ENERGY[6*6] = {
    0,1,1,1,0,0,
    0,0,1,0,0,0,
    0,1,1,0,0,0,
    0,0,1,0,0,0,
    0,0,1,1,0,0,
    0,0,0,0,0,0,
};
