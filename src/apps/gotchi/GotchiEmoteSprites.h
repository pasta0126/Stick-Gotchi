#pragma once
#include <stdint.h>
#include "../../apps/gotchi/GotchiPet.h"

// Emote icon sprites (8×8 px, scale 2×) — one per Mood value.
// Rendered inside the speech bubble above the gotchi.
// Palette: 0=transparent 1=primary mark 2=accent/color mark 3=shadow

constexpr uint8_t EMOTE_W     = 8;
constexpr uint8_t EMOTE_H     = 8;
constexpr uint8_t EMOTE_SCALE = 2;

// NEUTRAL — plain eyes, flat mouth
constexpr uint8_t SPR_EMOTE_NEUTRAL[8*8] = {
    0,0,0,0,0,0,0,0,
    0,0,1,0,0,1,0,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// HAPPY — dot eyes, curved smile
constexpr uint8_t SPR_EMOTE_HAPPY[8*8] = {
    0,0,0,0,0,0,0,0,
    0,0,1,0,0,1,0,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,1,0,0,0,0,1,0,
    0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// SICK — X eyes, wavy mouth
constexpr uint8_t SPR_EMOTE_SICK[8*8] = {
    0,0,0,0,0,0,0,0,
    0,1,0,1,1,0,1,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,1,0,1,0,1,0,
    0,1,0,1,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// PENSIVE — half-lid eye, three dots
constexpr uint8_t SPR_EMOTE_PENSIVE[8*8] = {
    0,0,0,0,0,0,0,0,
    0,0,1,0,0,0,0,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,1,0,1,0,1,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// SAD — teary eyes, frown
constexpr uint8_t SPR_EMOTE_SAD[8*8] = {
    0,0,0,0,0,0,0,0,
    0,0,1,0,0,1,0,0,
    0,0,1,2,0,1,2,0,
    0,0,0,2,0,0,2,0,
    0,0,0,1,1,0,0,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// SLEEPING — Z glyph
constexpr uint8_t SPR_EMOTE_SLEEPING[8*8] = {
    0,0,0,0,0,0,0,0,
    0,1,1,1,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,1,0,0,0,0,0,
    0,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// EXCITED — starburst
constexpr uint8_t SPR_EMOTE_EXCITED[8*8] = {
    0,0,0,1,0,0,0,0,
    0,1,0,1,0,1,0,0,
    0,0,1,1,1,0,0,0,
    1,1,1,1,1,1,1,0,
    0,0,1,1,1,0,0,0,
    0,1,0,1,0,1,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// LAUGHING — arc eyes, wide open grin
constexpr uint8_t SPR_EMOTE_LAUGHING[8*8] = {
    0,0,0,0,0,0,0,0,
    0,1,1,0,1,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,1,0,0,0,0,1,0,
    0,1,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// DIZZY — cross-hatch spiral
constexpr uint8_t SPR_EMOTE_DIZZY[8*8] = {
    0,0,0,0,0,0,0,0,
    0,1,0,1,0,1,0,0,
    0,0,1,0,1,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,1,0,1,0,0,0,
    0,1,0,1,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// ANNOYED — eyes + sweat drop + flat mouth
constexpr uint8_t SPR_EMOTE_ANNOYED[8*8] = {
    0,0,0,0,0,2,0,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,2,0,0,0,0,0,0,
    0,2,0,0,0,0,0,0,
    0,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// ANGRY — angled brows, frown
constexpr uint8_t SPR_EMOTE_ANGRY[8*8] = {
    0,1,0,0,0,0,1,0,
    0,0,1,0,0,1,0,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,1,1,0,0,0,
    0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// STARTLED — exclamation mark
constexpr uint8_t SPR_EMOTE_STARTLED[8*8] = {
    0,0,0,1,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// SCARED — wide circle eyes, zig-zag mouth
constexpr uint8_t SPR_EMOTE_SCARED[8*8] = {
    0,1,1,0,0,1,1,0,
    1,0,0,1,1,0,0,1,
    1,0,0,1,1,0,0,1,
    0,1,1,0,0,1,1,0,
    0,0,0,0,0,0,0,0,
    0,1,0,1,0,1,0,0,
    0,0,1,0,1,0,0,0,
    0,0,0,0,0,0,0,0,
};

// ─────────────────────────────────────────────────────────────────────────────
// Lookup
// ─────────────────────────────────────────────────────────────────────────────

inline const uint8_t* gotchiEmoteSprite(Mood mood) {
    switch (mood) {
    case Mood::HAPPY:     return SPR_EMOTE_HAPPY;
    case Mood::SICK:      return SPR_EMOTE_SICK;
    case Mood::PENSIVE:   return SPR_EMOTE_PENSIVE;
    case Mood::SAD:       return SPR_EMOTE_SAD;
    case Mood::SLEEPING:  return SPR_EMOTE_SLEEPING;
    case Mood::EXCITED:   return SPR_EMOTE_EXCITED;
    case Mood::LAUGHING:  return SPR_EMOTE_LAUGHING;
    case Mood::DIZZY:     return SPR_EMOTE_DIZZY;
    case Mood::ANNOYED:   return SPR_EMOTE_ANNOYED;
    case Mood::ANGRY:     return SPR_EMOTE_ANGRY;
    case Mood::STARTLED:  return SPR_EMOTE_STARTLED;
    case Mood::SCARED:    return SPR_EMOTE_SCARED;
    default:              return SPR_EMOTE_NEUTRAL;
    }
}
