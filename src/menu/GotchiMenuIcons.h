#pragma once
#include <M5Unified.h>
#include <math.h>

// Menu item icon functions — signature: void(M5Canvas&, int cx, int cy, int sz, uint32_t col)
// All drawing is relative to center (cx, cy) with rough bounding radius sz/2.

inline void menuIconGotchi(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz / 2;
    c.drawCircle(cx, cy, r, col);
    c.fillCircle(cx - r/3, cy - r/5, 2, col);
    c.fillCircle(cx + r/3, cy - r/5, 2, col);
    c.drawLine(cx - r/3, cy + r/5, cx - r/6, cy + r/3, col);
    c.drawLine(cx - r/6, cy + r/3, cx + r/6, cy + r/3, col);
    c.drawLine(cx + r/6, cy + r/3, cx + r/3, cy + r/5, col);
}

inline void menuIconStats(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int w = sz - 6, h = sz - 6;
    int x0 = cx - w/2, yb = cy + h/2;
    c.drawFastHLine(x0, yb, w, col);
    c.drawFastVLine(x0, yb - h, h, col);
    int xs[] = {x0 + 2, x0 + w/3, x0 + 2*w/3, x0 + w - 2};
    int ys[] = {yb - h/4, yb - h/2, yb - h*2/5, yb - h*4/5};
    for (int i = 0; i < 3; i++) {
        c.drawLine(xs[i], ys[i], xs[i+1], ys[i+1], col);
        c.fillCircle(xs[i], ys[i], 2, col);
    }
    c.fillCircle(xs[3], ys[3], 2, col);
}

inline void menuIconBars(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int bw = max(4, sz/5), base = cy + sz/2 - 2;
    int x0 = cx - sz/2 + 2, gap = max(2, (sz - 3*bw) / 2);
    c.fillRect(x0,              base - sz*2/4, bw, sz*2/4, col);
    c.fillRect(x0 + bw + gap,   base - sz*3/4, bw, sz*3/4, col);
    c.fillRect(x0 + 2*(bw+gap), base - sz*1/4, bw, sz*1/4, col);
    c.drawFastHLine(cx - sz/2, base+1, sz, col);
}

inline void menuIconCompass(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz/2 - 2;
    c.drawCircle(cx, cy, r, col);
    c.fillTriangle(cx, cy - r + 4, cx - 5, cy + 4, cx + 5, cy + 4, col);
    c.fillCircle(cx, cy + r - 4, 2, col);
    c.drawFastHLine(cx - r, cy, r*2, col);
    c.drawFastVLine(cx, cy - r, r*2, col);
}

inline void menuIconWave(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int pts = 24, w = sz - 4;
    for (int i = 0; i < pts - 1; i++) {
        float t0 = (float)i     / (pts-1) * 2 * 3.14159f;
        float t1 = (float)(i+1) / (pts-1) * 2 * 3.14159f;
        int x0 = cx - w/2 + i     * w / (pts-1);
        int x1 = cx - w/2 + (i+1) * w / (pts-1);
        int y0 = cy - (int)(sinf(t0) * sz/3);
        int y1 = cy - (int)(sinf(t1) * sz/3);
        c.drawLine(x0, y0, x1, y1, col);
    }
}

inline void menuIconCoin(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz / 2 - 2;
    c.fillCircle(cx, cy, r, col);
    c.fillCircle(cx, cy, r * 5 / 8, (uint32_t)0x000000);
    c.drawCircle(cx, cy, r, col);
}

inline void menuIcon8Ball(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz / 2 - 2;
    c.fillCircle(cx, cy, r, col);
    c.fillCircle(cx, cy, r / 2, (uint32_t)0x000000);
    c.setTextSize(1);
    c.setTextColor(col);
    c.drawCenterString("8", cx, cy - 3);
}

inline void menuIconReboot(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz/2 - 2;
    for (int a = 40; a <= 320; a += 6) {
        float rad = a * 3.14159f / 180.0f;
        c.fillCircle(cx + (int)(cosf(rad)*r), cy + (int)(sinf(rad)*r), 1, col);
    }
    float rad = 320.0f * 3.14159f / 180.0f;
    int ax = cx + (int)(cosf(rad)*r), ay = cy + (int)(sinf(rad)*r);
    c.fillTriangle(ax, ay-6, ax+6, ay+1, ax-4, ay+4, col);
}
