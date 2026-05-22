#pragma once
#include <stdint.h>

// Animation set definitions for all gotchi life stages.
// Each AnimDef describes frame count, hold duration per frame, and loop behaviour.

struct AnimDef {
    uint8_t  frameCount;      // number of sprite frames in the sequence
    uint16_t msPerFrame;      // ms to hold each frame before advancing
    bool     loop;            // true = restart from frame 0 after last frame
};

// ── Per-stage idle animations ──────────────────────────────────────────────────
//  EGG: 2-frame rock (left/right) at 600 ms/frame
//  BABY: 2-frame bob (up/down) at 500 ms/frame
//  YOUNG: 2-frame step cycle at 450 ms/frame
//  ADULT: 2-frame idle breathe at 600 ms/frame

constexpr AnimDef ANIM_EGG_IDLE   = { 2, 600, true  };
constexpr AnimDef ANIM_BABY_IDLE  = { 2, 500, true  };
constexpr AnimDef ANIM_YOUNG_IDLE = { 2, 450, true  };
constexpr AnimDef ANIM_ADULT_IDLE = { 2, 600, true  };

// ── Hatch sequence ─────────────────────────────────────────────────────────────
//  4-frame crack-and-burst played once; HATCH_FRAME_MS already in GotchiRenderer

constexpr AnimDef ANIM_EGG_HATCH  = { 4, 250, false };

// ── Future / reserved animations ──────────────────────────────────────────────
//  These are referenced by the AnimTag enum and will map to sprite data when
//  additional frames are authored. Currently they fall back to the idle sequence.

constexpr AnimDef ANIM_EAT   = { 4, 120, false };  // eat action burst
constexpr AnimDef ANIM_PLAY  = { 4, 100, true  };  // mini-game active loop
constexpr AnimDef ANIM_SLEEP = { 2, 800, true  };  // slow breathing while asleep
constexpr AnimDef ANIM_DIE   = { 4, 300, false };  // death sequence (played once)

// ── Mood modifier: speed multiplier table ──────────────────────────────────────
//  Used to scale msPerFrame based on current mood.
//  Values are in 1/100ths (100 = normal speed, 50 = twice as fast).

struct MoodAnimSpeed {
    uint8_t moodId;   // matches Mood enum value
    uint8_t factor;   // applied as msPerFrame * factor / 100
};

constexpr MoodAnimSpeed MOOD_ANIM_SPEEDS[] = {
    { 0,  100 },  // NEUTRAL   — normal
    { 1,   90 },  // HAPPY     — slightly faster
    { 2,  140 },  // SICK      — sluggish
    { 3,  120 },  // PENSIVE   — slow
    { 4,  110 },  // SAD       — slightly slow
    { 5,  160 },  // SLEEPING  — very slow
    { 6,   60 },  // EXCITED   — fast
    { 7,   70 },  // LAUGHING  — fast
    { 8,   80 },  // DIZZY     — erratic (random handled in renderer)
    { 9,  110 },  // ANNOYED   — slightly slow
    { 10, 130 },  // ANGRY     — stiff
    { 11,  50 },  // STARTLED  — burst-fast
    { 12,  85 },  // SCARED    — tense-fast
};
