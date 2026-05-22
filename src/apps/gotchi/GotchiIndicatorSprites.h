#pragma once
#include <stdint.h>

// Status indicator icons — 8×8 px, scale 1 — shown in play area corner when conditions trigger.
// Palette: 0=transparent  1=primary  2=highlight/secondary  3=shadow

constexpr uint8_t IND_ICON_W = 8;
constexpr uint8_t IND_ICON_H = 8;

// DIRTY — stink wavy lines
constexpr uint8_t SPR_IND_DIRTY[8*8] = {
    0,1,0,1,0,1,0,0,
    1,0,1,0,1,0,0,0,
    0,0,0,0,0,0,0,0,
    0,1,0,1,0,1,0,0,
    1,0,1,0,1,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// HUNGRY — empty bowl
constexpr uint8_t SPR_IND_HUNGRY[8*8] = {
    0,0,0,0,0,0,0,0,
    0,0,1,1,1,0,0,0,
    0,1,0,0,0,1,0,0,
    0,1,0,0,0,1,0,0,
    0,0,1,1,1,0,0,0,
    0,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// SICK — skull
constexpr uint8_t SPR_IND_SICK[8*8] = {
    0,0,1,1,1,0,0,0,
    0,1,1,1,1,1,0,0,
    0,1,2,1,2,1,0,0,
    0,1,1,1,1,1,0,0,
    0,0,1,1,1,0,0,0,
    0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// ALERT — exclamation mark
constexpr uint8_t SPR_IND_ALERT[8*8] = {
    0,0,0,1,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// SLEEP — Zzz glyph (for small indicator, separate from big sleep Zs)
constexpr uint8_t SPR_IND_SLEEP[8*8] = {
    0,0,0,0,0,0,0,0,
    0,0,1,1,1,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,1,0,0,0,0,0,
    0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};
