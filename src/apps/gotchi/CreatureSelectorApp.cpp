#include "CreatureSelectorApp.h"
#include <math.h>

static constexpr uint32_t BG = 0x040F02;

static const uint32_t COLORS_PRI[] = { 0xFFC000, 0x00CC88, 0x888888, 0xFF40FF };
static const uint32_t COLORS_DIM[] = { 0x664800, 0x004433, 0x333333, 0x661166 };
static const char* NAMES[]    = { "Bytee", "Cthulhu", "Jack", "Lumi" };
static const char* TAGLINES[] = {
    "Curious magic robot",
    "Just wants a hug",
    "Basically never reacts",
    "Extremely emotional fae",
};

void CreatureSelectorApp::update(uint32_t deltaMs) {
    _animAccum += deltaMs;
    if (_animAccum >= 600) {
        _animAccum -= 600;
        _animFrame ^= 1;
        _needsDraw = true;
    }
    if (!_needsDraw) return;
    _needsDraw = false;
    _drawFrame();
}

bool CreatureSelectorApp::onInput(const InputEvent& e) {
    if (e.button == ButtonId::B && e.action == ButtonAction::SHORT_PRESS) {
        _cycle();
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
        if (_confirmCb) _confirmCb(_preview);
        return true;
    }
    if (e.button == ButtonId::B && e.action == ButtonAction::LONG_PRESS) {
        if (_menuCallback) _menuCallback();
        return true;
    }
    return false;
}

void CreatureSelectorApp::_cycle() {
    uint8_t next = ((uint8_t)_preview + 1) % (uint8_t)CreatureType::_COUNT;
    _preview   = (CreatureType)next;
    _needsDraw = true;
}

void CreatureSelectorApp::_drawFrame() {
    if (!_display->acquire(100)) return;
    M5Canvas& c = _display->canvas();
    const int W = DisplayManager::W;
    const int H = DisplayManager::H;

    uint8_t  ci  = (uint8_t)_preview;
    uint32_t col = COLORS_PRI[ci];
    uint32_t dim = COLORS_DIM[ci];

    c.fillScreen(BG);

    // ── Creature name (header) ─────────────────────────────────────────────
    c.setTextFont(4);
    c.setTextColor(col, BG);
    c.drawCenterString(NAMES[ci], W / 2, 3);

    // Cycle hints
    c.setTextFont(2);
    c.setTextColor(dim, BG);
    c.setCursor(4, 4);
    c.print("<");
    c.setCursor(W - 12, 4);
    c.print(">");

    // ── Creature placeholder shape ─────────────────────────────────────────
    // (replaced by sprite sheet when SGOTCHI-83/84 art lands)
    int cx = W / 2;
    int cy = 68;
    bool blink = (_animFrame == 1);

    switch (_preview) {

    case CreatureType::BYTEE:
        // Hexagon body + two square eyes
        for (int i = 0; i < 6; i++) {
            float a0 = i * 3.14159f / 3.0f - 3.14159f / 6.0f;
            float a1 = (i + 1) * 3.14159f / 3.0f - 3.14159f / 6.0f;
            c.drawLine(cx + (int)(cosf(a0) * 30), cy + (int)(sinf(a0) * 30),
                       cx + (int)(cosf(a1) * 30), cy + (int)(sinf(a1) * 30), col);
        }
        c.fillCircle(cx, cy, 18, col);
        if (!blink) {
            c.fillRect(cx - 12, cy - 6, 7, 7, BG);
            c.fillRect(cx + 5,  cy - 6, 7, 7, BG);
        } else {
            c.drawFastHLine(cx - 12, cy - 3, 7, BG);
            c.drawFastHLine(cx + 5,  cy - 3, 7, BG);
        }
        // Antenna
        c.drawFastVLine(cx, cy - 32, 12, col);
        c.fillCircle(cx, cy - 32, 3, col);
        break;

    case CreatureType::CTHULHU:
        // Blob body + tentacles
        c.fillCircle(cx, cy - 4, 24, col);
        for (int t = 0; t < 5; t++) {
            float a = 3.14159f / 2.0f + t * 3.14159f / 4.0f;
            int tx = cx + (int)(cosf(a) * 24);
            int ty = cy - 4 + (int)(sinf(a) * 24);
            c.drawLine(tx, ty, tx + (int)(cosf(a) * 14), ty + (int)(sinf(a) * 14), col);
            c.fillCircle(tx + (int)(cosf(a) * 14), ty + (int)(sinf(a) * 14), 3, col);
        }
        if (!blink) {
            c.fillCircle(cx - 9, cy - 8, 5, BG);
            c.fillCircle(cx + 9, cy - 8, 5, BG);
        } else {
            c.drawFastHLine(cx - 14, cy - 8, 10, BG);
            c.drawFastHLine(cx + 4,  cy - 8, 10, BG);
        }
        break;

    case CreatureType::JACK:
        // Stone body (rectangle) + hat (trapezoid)
        c.fillRect(cx - 20, cy - 14, 40, 34, col);
        c.fillTriangle(cx - 26, cy - 14, cx + 26, cy - 14, cx, cy - 36, col);
        c.fillRect(cx - 28, cy - 18, 56, 4, col);  // hat brim
        if (!blink) {
            c.fillRect(cx - 10, cy - 4, 7, 8, BG);
            c.fillRect(cx + 3,  cy - 4, 7, 8, BG);
        }
        // (Jack rarely blinks — just keeps a flat expression)
        break;

    case CreatureType::LUMI:
        // Star/flower petals + circular face
        for (int p = 0; p < 8; p++) {
            float a = p * 3.14159f / 4.0f;
            c.fillCircle(cx + (int)(cosf(a) * 22), cy + (int)(sinf(a) * 22), 8, col);
        }
        c.fillCircle(cx, cy, 20, col);
        if (!blink) {
            c.fillCircle(cx - 7, cy - 4, 4, BG);
            c.fillCircle(cx + 7, cy - 4, 4, BG);
        } else {
            c.drawFastHLine(cx - 11, cy - 4, 8, BG);
            c.drawFastHLine(cx + 3,  cy - 4, 8, BG);
        }
        // Ear tufts / antlers
        c.fillCircle(cx - 20, cy - 26, 5, col);
        c.fillCircle(cx + 20, cy - 26, 5, col);
        break;

    default: break;
    }

    // ── Tagline ───────────────────────────────────────────────────────────
    c.drawFastHLine(0, 102, W, dim);
    c.setTextFont(2);
    c.setTextColor(col, BG);
    c.drawCenterString(TAGLINES[ci], W / 2, 106);

    // ── Button hints ──────────────────────────────────────────────────────
    c.setTextFont(1);
    c.setTextColor(dim, BG);
    c.drawCenterString("B: cambiar   A: elegir", W / 2, H - 9);

    _display->push();
    _display->release();
}
