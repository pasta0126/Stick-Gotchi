#pragma once
#include <stdint.h>

struct GotchiID {
    uint8_t  generation;      // 0–255
    uint8_t  parent_id[3];    // 24 bits del ID del padre (truncado)
    uint16_t visual_seed;     // semilla visual determinista
    uint16_t birth_epoch;     // timestamp truncado (días desde epoch fijo)
};

struct GotchiVisual {
    uint8_t hue_primary;      // 0–31  → hue = hue_primary * 11.6°
    uint8_t hue_secondary;    // 0–31
    uint8_t eye_type;         // 0–3
    uint8_t mark_type;        // 0–3
    uint8_t body_shape;       // 0–3
};

GotchiVisual decodeVisual(uint16_t seed);

uint16_t mutateSeed(uint16_t parent_seed, uint32_t rng);

GotchiID createNewID(uint8_t generation, const uint8_t parent_id[3], uint16_t visual_seed);

uint16_t hsvToRgb565(uint8_t hue32, uint8_t sat, uint8_t val);
