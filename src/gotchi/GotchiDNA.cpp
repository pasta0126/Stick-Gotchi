#include "GotchiDNA.h"
#include <Arduino.h>
#include <cmath>

GotchiVisual decodeVisual(uint16_t seed) {
    return {
        .hue_primary   = (uint8_t)((seed >> 11) & 0x1F),
        .hue_secondary = (uint8_t)((seed >> 6)  & 0x1F),
        .eye_type      = (uint8_t)((seed >> 4)  & 0x03),
        .mark_type     = (uint8_t)((seed >> 2)  & 0x03),
        .body_shape    = (uint8_t)(seed         & 0x03),
    };
}

uint16_t mutateSeed(uint16_t parent_seed, uint32_t rng) {
    uint8_t hp = (parent_seed >> 11) & 0x1F;
    uint8_t hs = (parent_seed >> 6)  & 0x1F;

    hp = (hp + (rng & 1 ? 1 : -1)) & 0x1F;
    hs = (hs + ((rng >> 1) & 1 ? 1 : -1)) & 0x1F;

    uint8_t marks = ((rng >> 8) % 10 == 0)
        ? (((parent_seed >> 2) & 0x03) + 1) & 0x03
        : (parent_seed >> 2) & 0x03;

    return (hp << 11) | (hs << 6) | (((parent_seed >> 4) & 0x03) << 4)
         | (marks << 2) | (parent_seed & 0x03);
}

GotchiID createNewID(uint8_t generation, const uint8_t parent_id[3], uint16_t visual_seed) {
    uint16_t birth_epoch = (uint16_t)(millis() / 86400000UL);
    return {
        .generation = generation,
        .parent_id = {parent_id[0], parent_id[1], parent_id[2]},
        .visual_seed = visual_seed,
        .birth_epoch = birth_epoch,
    };
}

uint16_t hsvToRgb565(uint8_t hue32, uint8_t sat, uint8_t val) {
    float h = hue32 * 360.0f / 32.0f;
    float s = sat / 255.0f;
    float v = val / 255.0f;

    float c = v * s;
    float hp = h / 60.0f;
    float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));

    float r1, g1, b1;
    if (hp < 1.0f) {
        r1 = c; g1 = x; b1 = 0.0f;
    } else if (hp < 2.0f) {
        r1 = x; g1 = c; b1 = 0.0f;
    } else if (hp < 3.0f) {
        r1 = 0.0f; g1 = c; b1 = x;
    } else if (hp < 4.0f) {
        r1 = 0.0f; g1 = x; b1 = c;
    } else if (hp < 5.0f) {
        r1 = x; g1 = 0.0f; b1 = c;
    } else {
        r1 = c; g1 = 0.0f; b1 = x;
    }

    float m = v - c;
    uint8_t r = (uint8_t)((r1 + m) * 255.0f);
    uint8_t g = (uint8_t)((g1 + m) * 255.0f);
    uint8_t b = (uint8_t)((b1 + m) * 255.0f);

    uint16_t r5 = (r >> 3) & 0x1F;
    uint16_t g6 = (g >> 2) & 0x3F;
    uint16_t b5 = (b >> 3) & 0x1F;

    return (r5 << 11) | (g6 << 5) | b5;
}
