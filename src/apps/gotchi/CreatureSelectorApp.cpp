#include "CreatureSelectorApp.h"
#include "GotchiFaceSprites.h"

static const uint16_t COLORS_PRI[] = { 0xFFC0, 0x0CC8, 0x8C71, 0xF81F };
static const uint16_t COLORS_DIM[] = { 0x4940, 0x0220, 0x2104, 0x4008 };
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
    uint16_t col = COLORS_PRI[ci];
    uint16_t dim = COLORS_DIM[ci];

    // ── Face portrait (full-screen background) ─────────────────────────────
    {
        const uint8_t*  rle = getFaceFrameData(ci, 0, _animFrame);  // mood 0 = NEUTRAL
        const uint16_t* pal = getFacePalette(ci);
        int px = 0, py = 0, drawn = 0;
        const int total = FACE_W * FACE_H;
        while (drawn < total) {
            uint8_t  count = *rle++;
            uint16_t color = pal[*rle++];
            int rem = count;
            while (rem > 0) {
                int line_rem = FACE_W - px;
                int take = rem < line_rem ? rem : line_rem;
                c.fillRect(px * 2, py * 2, take * 2, 2, color);
                px += take; rem -= take; drawn += take;
                if (px >= FACE_W) { px = 0; py++; }
            }
        }
    }

    // ── Navigation arrows (over portrait, mid-height) ─────────────────────
    c.setTextFont(2);
    c.setTextColor(col, 0x0000);
    c.setCursor(2, 48);   c.print("<");
    c.setCursor(W - 12, 48); c.print(">");

    // ── Bottom strip (name + hints) ────────────────────────────────────────
    c.fillRect(0, 107, W, H - 107, 0x0000);
    c.drawFastHLine(0, 107, W, col);

    c.setTextFont(2);
    c.setTextColor(col, 0x0000);
    c.drawCenterString(NAMES[ci], W / 2, 109);

    c.setTextFont(1);
    c.setTextColor(dim, 0x0000);
    c.drawCenterString(TAGLINES[ci], W / 2, 126);

    _display->push();
    _display->release();
}
