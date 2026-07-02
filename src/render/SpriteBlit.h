#pragma once
#include <M5Unified.h>

// Shared palette-indexed sprite blitter, used by any app that draws
// palette-index sprite data (0=transparent, 1..5=palette slots).
struct SpritePalette {
    uint16_t transparent;
    uint16_t primary;
    uint16_t secondary;
    uint16_t dark;
    uint16_t accent;
    uint16_t color5 = 0;
};

inline void drawPaletteSprite(M5Canvas& canvas, const uint8_t* data, uint8_t w, uint8_t h,
                               int x, int y, uint8_t scale, const SpritePalette& pal) {
    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            uint8_t idx = data[py * w + px];
            if (idx == 0) continue;

            uint16_t color;
            switch (idx) {
            case 1: color = pal.primary; break;
            case 2: color = pal.secondary; break;
            case 3: color = pal.dark; break;
            case 4: color = pal.accent; break;
            case 5: color = pal.color5; break;
            default: color = pal.transparent; break;
            }

            if (color != pal.transparent) {
                int sx = x + px * scale;
                int sy = y + py * scale;
                canvas.fillRect(sx, sy, scale, scale, color);
            }
        }
    }
}
