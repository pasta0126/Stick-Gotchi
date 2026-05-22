#pragma once
#include <stdint.h>
#include "../../gotchi/GotchiType.h"

// Type identity icons — 8×8 px, displayed at scale 1 inside the stats bar.
// Palette: 0=transparent  1=primary fill  2=highlight  3=shadow

constexpr uint8_t TYPE_ICON_W = 8;
constexpr uint8_t TYPE_ICON_H = 8;

// ORGANIC — leaf with vein
constexpr uint8_t SPR_TYPE_ORGANIC[8*8] = {
    0,0,0,1,1,0,0,0,
    0,0,1,1,1,1,0,0,
    0,1,1,2,1,1,0,0,
    0,1,1,1,1,1,0,0,
    0,0,1,1,1,0,0,0,
    0,0,0,3,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// CRYSTAL — diamond
constexpr uint8_t SPR_TYPE_CRYSTAL[8*8] = {
    0,0,0,1,0,0,0,0,
    0,0,1,1,1,0,0,0,
    0,1,2,1,1,1,0,0,
    1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,0,0,
    0,0,1,1,1,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// ENERGY — lightning bolt
constexpr uint8_t SPR_TYPE_ENERGY[8*8] = {
    0,0,1,1,1,0,0,0,
    0,0,1,1,0,0,0,0,
    0,1,1,1,0,0,0,0,
    0,1,1,1,1,0,0,0,
    0,0,0,1,1,0,0,0,
    0,0,0,0,1,1,0,0,
    0,0,0,0,1,0,0,0,
    0,0,0,0,0,0,0,0,
};

// CYBER — chip / circuit board
constexpr uint8_t SPR_TYPE_CYBER[8*8] = {
    0,1,1,1,1,1,0,0,
    0,1,2,0,2,1,0,0,
    1,1,0,0,0,1,1,0,
    1,0,0,0,0,0,1,0,
    0,1,2,0,2,1,0,0,
    0,1,1,1,1,1,0,0,
    0,0,1,0,1,0,0,0,
    0,0,0,0,0,0,0,0,
};

// ELEMENTAL — 4-point star / compass rose
constexpr uint8_t SPR_TYPE_ELEMENTAL[8*8] = {
    0,0,0,1,0,0,0,0,
    0,0,1,1,1,0,0,0,
    0,1,1,2,1,1,0,0,
    1,1,2,1,2,1,1,0,
    0,1,1,2,1,1,0,0,
    0,0,1,1,1,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// SOUL — crescent moon
constexpr uint8_t SPR_TYPE_SOUL[8*8] = {
    0,0,1,1,1,0,0,0,
    0,1,1,1,1,1,0,0,
    0,1,2,0,0,1,0,0,
    0,1,0,0,0,1,0,0,
    0,1,1,0,0,1,0,0,
    0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

inline const uint8_t* gotchiTypeIcon(GotchiType type) {
    switch (type) {
    case GotchiType::ORGANIC:   return SPR_TYPE_ORGANIC;
    case GotchiType::CRYSTAL:   return SPR_TYPE_CRYSTAL;
    case GotchiType::ENERGY:    return SPR_TYPE_ENERGY;
    case GotchiType::CYBER:     return SPR_TYPE_CYBER;
    case GotchiType::ELEMENTAL: return SPR_TYPE_ELEMENTAL;
    case GotchiType::SOUL:      return SPR_TYPE_SOUL;
    default:                    return SPR_TYPE_ORGANIC;
    }
}

// RGB565 accent color per type
inline uint16_t gotchiTypeColor(GotchiType type) {
    switch (type) {
    case GotchiType::ORGANIC:   return 0x2D40;  // forest green
    case GotchiType::CRYSTAL:   return 0x4DF7;  // ice blue
    case GotchiType::ENERGY:    return 0xFFE0;  // electric yellow
    case GotchiType::CYBER:     return 0x03EF;  // neon cyan
    case GotchiType::ELEMENTAL: return 0xFC00;  // fire orange
    case GotchiType::SOUL:      return 0xA01F;  // violet
    default:                    return 0x8888;
    }
}

inline const char* gotchiTypeShortName(GotchiType type) {
    switch (type) {
    case GotchiType::ORGANIC:   return "ORG";
    case GotchiType::CRYSTAL:   return "CRY";
    case GotchiType::ENERGY:    return "ENE";
    case GotchiType::CYBER:     return "CYB";
    case GotchiType::ELEMENTAL: return "ELE";
    case GotchiType::SOUL:      return "SOL";
    default:                    return "???";
    }
}
