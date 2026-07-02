#include "CarouselHome.h"
#include "../core/AppManager.h"

void CarouselHome::init() {
    _dirty = true;
}

bool CarouselHome::onInput(const InputEvent& e) {
    if (_tiles.empty()) return false;

    if (e.button == ButtonId::B && e.action == ButtonAction::SHORT_PRESS) {
        _index = (_index + 1) % _tiles.size();
        _dirty = true;
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
        if (_apps) _apps->launchApp(_tiles[_index].launch());
        return true;
    }
    // BtnB long / BtnA long: no function at Home.
    return true;
}

void CarouselHome::update(uint32_t /*deltaMs*/) {
    if (!_dirty) return;
    _dirty = false;
    _draw();
}

void CarouselHome::_draw() {
    if (!_display->acquire(50)) return;
    auto& c = _display->canvas();
    c.fillScreen(0x000000);

    if (_tiles.empty()) {
        c.setTextColor(0x4208);
        c.drawCenterString("(sin apps)", 120, 65);
        _display->push();
        _display->release();
        return;
    }

    constexpr int CENTER_X = 120, CENTER_Y = 58;
    constexpr int SLOT_W   = 78;
    constexpr int SIDE_SZ  = 52, SIDE_SCALE = 1;
    constexpr int MID_SZ   = 76, MID_SCALE  = 2;

    size_t n = _tiles.size();
    for (int off = -1; off <= 1; off++) {
        size_t idx   = (_index + n + (size_t)(off + (int)n)) % n;
        int    cx    = CENTER_X + off * SLOT_W;
        bool   sel   = (off == 0);
        int    sz    = sel ? MID_SZ    : SIDE_SZ;
        int    scale = sel ? MID_SCALE : SIDE_SCALE;
        uint32_t box = sel ? 0x40FF20 : 0x2A5020;

        c.drawRoundRect(cx - sz/2, CENTER_Y - sz/2, sz, sz, 6, box);
        if (sel) c.drawRoundRect(cx - sz/2 - 2, CENTER_Y - sz/2 - 2, sz + 4, sz + 4, 8, box);

        _tiles[idx].iconFn(c, cx, CENTER_Y - (sel ? 6 : 4), scale, sel);

        if (sel) {
            c.setTextSize(1);
            c.setTextColor(0xFFFFFF);
            c.drawCenterString(_tiles[idx].name, cx, CENTER_Y + sz/2 - 13);
        }
    }

    int dotsW = (int)n * 10;
    int dx0   = CENTER_X - dotsW/2 + 5;
    for (size_t i = 0; i < n; i++) {
        uint32_t col = (i == _index) ? 0x40FF20 : 0x1A3010;
        c.fillCircle(dx0 + (int)i * 10, 124, 3, col);
    }

    _display->push();
    _display->release();
}
