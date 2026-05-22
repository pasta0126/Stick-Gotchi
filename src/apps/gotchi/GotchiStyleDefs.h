#pragma once
#include <stdint.h>
#include "../../gotchi/GotchiType.h"

// ── Global gotchi art style rules ─────────────────────────────────────────────
//
// All sprites use a 5-slot palette (indices 0-4, index 5 available for extras):
//   0 = transparent (always skipped by _drawSprite)
//   1 = primary fill      — main body / dominant color
//   2 = highlight         — specular or inner lighter tone
//   3 = shadow / outline  — dark edge or ground shadow
//   4 = accent            — eye color, mark, small feature
//   5 = special           — background match or unique per asset
//
// RGB565 values are expressed as 0xRRGG (16-bit, big-endian as M5Canvas expects).

// ── Type accent colors (authoritative — matches GotchiTypeIdentitySprites.h) ──
constexpr uint16_t STYLE_COLOR_ORGANIC   = 0x2D40;  // forest green
constexpr uint16_t STYLE_COLOR_CRYSTAL   = 0x4DF7;  // ice blue
constexpr uint16_t STYLE_COLOR_ENERGY    = 0xFFE0;  // electric yellow
constexpr uint16_t STYLE_COLOR_CYBER     = 0x03EF;  // neon cyan
constexpr uint16_t STYLE_COLOR_ELEMENTAL = 0xFC00;  // fire orange
constexpr uint16_t STYLE_COLOR_SOUL      = 0xA01F;  // violet

// ── AdultForm palette modifiers ───────────────────────────────────────────────
//  Applied as offsets to HSV saturation/value in _buildPalette():
//    HEALTHY:   sat+40, val+35, accent=0xFFE0 (golden glow)
//    NORMAL:    sat+20, val+20
//    NEGLECTED: sat-120, val-80, accent=0x4208 (dim grey)

// ── Sprite size table (w × h, scale, displayed px) ───────────────────────────
//  EGG:   12 × 14, scale 3 → 36 × 42 px displayed
//  BABY:  16 × 16, scale 3 → 48 × 48 px displayed
//  YOUNG: 20 × 22, scale 2 → 40 × 44 px displayed
//  ADULT: 24 × 26, scale 2 → 48 × 52 px displayed

// ── Glow / visual effect rules ────────────────────────────────────────────────
//  ELEMENTAL types: primary hue cycles slowly (10 s period)
//  ENERGY / SOUL types: primary brightness boosted +20 val
//  HEALTHY adults: accent renders as 0xFFE0 (warm golden sparkle)
//  NEGLECTED adults: all colors desaturated; dark accent (0x4208)
//
//  Sleep state: no visual palette override — motion stops, Zs drawn instead.
//  Sick state: renderer does NOT change palette; SICK emote/indicator conveys it.

// ── HUD color conventions ─────────────────────────────────────────────────────
//  Stat bars: green (>60%), yellow (>30%), red (≤30%)
//  Type stripe + bottom line in stats bar: gotchiTypeColor(type)
//  Action bar active button: 0x07E0 (green), icon color 0x0000 (black)
//  Action bar inactive button: 0x2104 (dark), icon color 0x8888 (grey)
//  Menu: solarpunk palette (SP_BG=0x040F02, SP_GREEN=0x40FF20, SP_AMBER=0xFFCC00)

// ── Background color anchors per habitat ─────────────────────────────────────
//  ORGANIC:   sky 0x4419→0x75DC, grass 0x5D08, dirt 0x33C5
//  CRYSTAL:   cave 0x0010, ice 0x2D7F, stalactite 0x4DF7
//  ENERGY:    void 0x080C, grid 0x2104, glow 0xFFE0
//  CYBER:     room 0x0821, panel 0x0842, neon-cyan 0x03EF, neon-red 0xF800
//  ELEMENTAL: sky dark 0x8000→bright 0xFC40, rock 0x3186, lava 0xFC00
//  SOUL:      space 0x0801, platform 0x5820, star 0xFFFF / 0xA01F

// ── Emote bubble style ────────────────────────────────────────────────────────
//  Bubble fill: 0xFFFF (white), border: 0xC618 (light grey)
//  Bubble size: 22 × 20 px, corner radius 3, tail 4 px
//  Emote icon: 8 × 8 px at scale 2, palette: dark 0x2104, blue 0x001F, yellow 0xFFE0
