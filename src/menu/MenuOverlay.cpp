#include "MenuOverlay.h"

// ── Solarpunk palette ─────────────────────────────────────────────────────────
static constexpr uint32_t SP_BG     = 0x040F02; // near-black forest
static constexpr uint32_t SP_MOSS   = 0x183010; // dark moss (header/footer)
static constexpr uint32_t SP_GREEN  = 0x40FF20; // acid lime
static constexpr uint32_t SP_AMBER  = 0xFFCC00; // warm amber
static constexpr uint32_t SP_WHITE  = 0xDCF5D0; // off-white with green tint
static constexpr uint32_t SP_DIM    = 0x2A5020; // dim green (inactive)
static constexpr uint32_t SP_RULE   = 0x1A3A12; // subtle separator

static constexpr int HEADER_H = 18;
static constexpr int FOOTER_H = 20;

void MenuOverlay::begin(ButtonManager& buttons, DisplayManager& display, AppManager& apps) {
    _buttons = &buttons;
    _display = &display;
    _apps    = &apps;
}

void MenuOverlay::addItem(MenuItem item) {
    _items.push_back(std::move(item));
}

void MenuOverlay::open() {
    if (_open) return;
    _open        = true;
    _dirty       = true;
    _selectedIdx = 0;
    _apps->suspendCurrent();
    _buttons->setCallback([this](const InputEvent& e) { _handleInput(e); });
}

void MenuOverlay::close() {
    if (!_open) return;
    _open = false;
    _apps->resumeCurrent();
}

void MenuOverlay::nextItem() {
    if (!_open || _items.empty()) return;
    _selectedIdx = (_selectedIdx + 1) % (int)_items.size();
    _dirty = true;
}

void MenuOverlay::prevItem() {
    if (!_open || _items.empty()) return;
    _selectedIdx = (_selectedIdx - 1 + (int)_items.size()) % (int)_items.size();
    _dirty = true;
}

void MenuOverlay::update() {
    if (!_dirty) return;
    _dirty = false;
    _render();
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void MenuOverlay::_render() {
    if (!_display->acquire(100)) return;

    auto& c = _display->canvas();
    const int W = DisplayManager::W; // 240
    const int H = DisplayManager::H; // 135

    c.fillScreen(SP_BG);

    // ── Header ────────────────────────────────────────────────────────────────
    c.fillRect(0, 0, W, HEADER_H, SP_MOSS);

    c.setTextFont(2);
    c.setTextColor(SP_GREEN, SP_MOSS);
    c.setCursor(8, 2);
    c.print("MENU");

    // Item counter right-aligned
    char counter[12];
    snprintf(counter, sizeof(counter), "%d/%d", _selectedIdx + 1, (int)_items.size());
    c.setTextFont(1);
    c.setTextColor(SP_DIM, SP_MOSS);
    int tw = c.textWidth(counter);
    c.setCursor(W - tw - 8, 5);
    c.print(counter);

    // ── Content ───────────────────────────────────────────────────────────────
    if (_selectedIdx < (int)_items.size()) {
        MenuItem& item = _items[_selectedIdx];

        // Vertical separator between icon area and text
        c.drawFastVLine(90, HEADER_H + 4, H - HEADER_H - FOOTER_H - 8, SP_RULE);

        // Icon — centered in left column (x=0..90, y=HEADER_H..H-FOOTER_H)
        int iconCx = 45;
        int iconCy = HEADER_H + (H - HEADER_H - FOOTER_H) / 2;
        int iconSz = 44;

        if (item.iconFn) {
            item.iconFn(c, iconCx, iconCy, iconSz, SP_GREEN);
        } else {
            // Fallback: hexagon outline as generic icon
            for (int i = 0; i < 6; i++) {
                float a0 = i       * 3.14159f / 3.0f - 3.14159f / 6.0f;
                float a1 = (i + 1) * 3.14159f / 3.0f - 3.14159f / 6.0f;
                int x0 = iconCx + (int)(cosf(a0) * iconSz / 2);
                int y0 = iconCy + (int)(sinf(a0) * iconSz / 2);
                int x1 = iconCx + (int)(cosf(a1) * iconSz / 2);
                int y1 = iconCy + (int)(sinf(a1) * iconSz / 2);
                c.drawLine(x0, y0, x1, y1, SP_GREEN);
            }
        }

        // Title
        c.setTextFont(2);
        c.setTextColor(SP_WHITE, SP_BG);
        c.setCursor(98, HEADER_H + 12);
        c.print(item.label);

        // Thin rule under title
        c.drawFastHLine(98, HEADER_H + 30, W - 106, SP_RULE);

        // Type badge
        c.setTextFont(1);
        c.setTextColor(SP_DIM, SP_BG);
        c.setCursor(98, HEADER_H + 36);
        c.print(item.type == MenuItemType::APP ? "application" : "action");

        // Select hint
        c.setTextColor(SP_AMBER, SP_BG);
        c.setCursor(98, HEADER_H + 56);
        c.print("A hold  \x10  select"); // \x10 = right-arrow glyph in GFX font
    }

    // ── Footer ────────────────────────────────────────────────────────────────
    int fy = H - FOOTER_H;
    c.fillRect(0, fy, W, FOOTER_H, SP_MOSS);

    // Left arrow (prev = C button) — filled triangle pointing left
    c.fillTriangle(6, fy + 10, 14, fy + 4, 14, fy + 16, SP_AMBER);
    c.setTextFont(1);
    c.setTextColor(SP_DIM, SP_MOSS);
    c.setCursor(18, fy + 6);
    c.print("C");

    // Right arrow (next = B button) — filled triangle pointing right
    c.fillTriangle(W - 6, fy + 10, W - 14, fy + 4, W - 14, fy + 16, SP_AMBER);
    int bw = c.textWidth("B");
    c.setCursor(W - 14 - bw - 3, fy + 6);
    c.print("B");

    // Navigation dots — centred
    int n = (int)_items.size();
    if (n > 0) {
        int dotStep = 10;
        int dotsW   = (n - 1) * dotStep;
        int dotX    = (W - dotsW) / 2;
        int dotY    = fy + 10;
        for (int i = 0; i < n; i++) {
            int dx = dotX + i * dotStep;
            if (i == _selectedIdx) {
                c.fillCircle(dx, dotY, 4, SP_GREEN);
            } else {
                c.fillCircle(dx, dotY, 2, SP_DIM);
            }
        }
    }

    _display->push();
    _display->release();
}

// ── Input ─────────────────────────────────────────────────────────────────────

void MenuOverlay::_handleInput(const InputEvent& e) {
    if (e.button == ButtonId::B && e.action == ButtonAction::SHORT_PRESS) {
        nextItem();
    } else if (e.button == ButtonId::A && e.action == ButtonAction::LONG_PRESS) {
        _selectCurrent();
    } else if (e.button == ButtonId::B && e.action == ButtonAction::LONG_PRESS) {
        close();
    }
    // C (prev) is handled directly in main.cpp via M5.BtnC.wasPressed()
}

void MenuOverlay::_selectCurrent() {
    if (_selectedIdx < 0 || _selectedIdx >= (int)_items.size()) return;
    MenuItem& item = _items[_selectedIdx];

    if (item.type == MenuItemType::APP && item.appFactory) {
        AppBase* app = item.appFactory();
        close();
        _apps->launchApp(app);
    } else if (item.type == MenuItemType::ACTION && item.action) {
        close();
        item.action();
    }
}
