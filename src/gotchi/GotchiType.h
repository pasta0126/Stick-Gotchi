#pragma once
#include <stdint.h>

// Six gotchi types — determine habitat, affinity bonuses, and visual identity.
// Assigned at birth from the visual seed; replaces the old GotchiBranch system.
enum class GotchiType : uint8_t {
    ORGANIC   = 0,  // blob-like, fleshy, rounded — organic spots & bumps
    CRYSTAL   = 1,  // geometric, faceted, angular — gem crown, tapered base
    ENERGY    = 2,  // plasma/electric, spiky aura, floating — energy tail
    CYBER     = 3,  // mechanical, blocky — circuit antenna, screen face
    ELEMENTAL = 4,  // nature magic — leaf crown, vine tendrils
    SOUL      = 5,  // ghostly, ethereal — hollow eyes, wispy underbody
};

constexpr uint8_t GOTCHI_TYPE_COUNT = 6;

struct GotchiTypeInfo {
    const char* name;
    const char* habitat;
};

constexpr GotchiTypeInfo GOTCHI_TYPE_INFO[GOTCHI_TYPE_COUNT] = {
    { "Organic",   "organic"   },
    { "Crystal",   "crystal"   },
    { "Energy",    "energy"    },
    { "Cyber",     "cyber"     },
    { "Elemental", "elemental" },
    { "Soul",      "soul"      },
};

// Derive type from the lower 3 bits of body_shape + mark_type fields of GotchiVisual.
// Keeps the mapping deterministic and tied to the DNA seed.
inline GotchiType gotchiTypeFromSeed(uint8_t body_shape, uint8_t mark_type) {
    return static_cast<GotchiType>((body_shape * 2 + mark_type) % GOTCHI_TYPE_COUNT);
}
