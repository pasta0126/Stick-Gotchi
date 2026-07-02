#pragma once
#include <M5Unified.h>
#include "../render/SpriteBlit.h"
#include "../generated/sprites_coin.h"
#include "../generated/sprites_8magicball.h"
#include "../generated/sprites_accel.h"
#include "../generated/sprites_gyro.h"
#include "../generated/sprites_orient.h"

// Carousel tile icons — pixel-art sprites (16x16, palette-indexed) drawn via
// drawPaletteSprite. `selected` picks a dim (side tile) or bright (centered
// tile) palette; signature matches CarouselTile::iconFn.

inline SpritePalette tileDimPalette() {
    SpritePalette p;
    p.transparent = 0x0000;
    p.primary     = 0x2A5020;
    p.secondary   = 0x1A3010;
    p.dark        = 0x0D2008;
    p.accent      = 0x60A050;
    return p;
}

inline SpritePalette tileBrightPalette() {
    SpritePalette p;
    p.transparent = 0x0000;
    p.primary     = 0x40FF20;
    p.secondary   = 0x2ADD10;
    p.dark        = 0x155008;
    p.accent      = 0xC8FFA0;
    return p;
}

inline void drawTileSprite(M5Canvas& c, const uint8_t* data, uint8_t w, uint8_t h,
                            int cx, int cy, int scale, bool selected) {
    SpritePalette pal = selected ? tileBrightPalette() : tileDimPalette();
    int x = cx - (w * scale) / 2;
    int y = cy - (h * scale) / 2;
    drawPaletteSprite(c, data, w, h, x, y, scale, pal);
}

inline void tileIconCoin(M5Canvas& c, int cx, int cy, int scale, bool selected) {
    drawTileSprite(c, SPR_COIN_F6, COIN_W, COIN_H, cx, cy, scale, selected);
}
inline void tileIcon8Ball(M5Canvas& c, int cx, int cy, int scale, bool selected) {
    drawTileSprite(c, SPR_8BALL_F0, BALL8_W, BALL8_H, cx, cy, scale, selected);
}
inline void tileIconAccel(M5Canvas& c, int cx, int cy, int scale, bool selected) {
    drawTileSprite(c, SPR_ACCEL_DEFAULT_F0, ACCEL_W, ACCEL_H, cx, cy, scale, selected);
}
inline void tileIconGyro(M5Canvas& c, int cx, int cy, int scale, bool selected) {
    drawTileSprite(c, SPR_GYRO_DEFAULT_F0, GYRO_W, GYRO_H, cx, cy, scale, selected);
}
inline void tileIconOrient(M5Canvas& c, int cx, int cy, int scale, bool selected) {
    drawTileSprite(c, SPR_ORIENT_DEFAULT_F0, ORIENT_W, ORIENT_H, cx, cy, scale, selected);
}
