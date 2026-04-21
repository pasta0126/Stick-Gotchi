#include "MenuOverlay.h"

// ── Solarpunk palette ─────────────────────────────────────────────────────────
static constexpr uint32_t SP_BG    = 0x040F02;
static constexpr uint32_t SP_MOSS  = 0x183010;
static constexpr uint32_t SP_GREEN = 0x40FF20;
static constexpr uint32_t SP_AMBER = 0xFFCC00;
static constexpr uint32_t SP_WHITE = 0xDCF5D0;
static constexpr uint32_t SP_DIM   = 0x2A5020;
static constexpr uint32_t SP_RULE  = 0x1A3A12;

static constexpr int HEADER_H = 20;

void MenuOverlay::begin(ButtonManager& buttons, DisplayManager& display, AppManager& apps) {
    _buttons = &buttons;
    _display = &display;
    _apps    = &apps;
}

void MenuOverlay::addItem(MenuItem item) {
    _rootItems.push_back(std::move(item));
}

void MenuOverlay::open() {
    if (_open) return;
    _open  = true;
    _dirty = true;
    _stack.clear();
    _stack.push_back({ _rootItems, 0 });
    _apps->suspendCurrent();
    _buttons->setCallback([this](const InputEvent& e) { _handleInput(e); });
}

void MenuOverlay::close() {
    if (!_open) return;
    _open = false;
    _stack.clear();
    _apps->resumeCurrent();
}

void MenuOverlay::nextItem() {
    if (!_open || _stack.empty()) return;
    int n = (int)_cur().items.size();
    if (n == 0) return;
    _cur().selectedIdx = (_cur().selectedIdx + 1) % n;
    _dirty = true;
}

void MenuOverlay::prevItem() {
    if (!_open || _stack.empty()) return;
    int n = (int)_cur().items.size();
    if (n == 0) return;
    _cur().selectedIdx = (_cur().selectedIdx - 1 + n) % n;
    _dirty = true;
}

void MenuOverlay::_goBack() {
    if (_stack.size() > 1) {
        _stack.pop_back();
        _dirty = true;
    } else {
        close();
    }
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
    const int W = DisplayManager::W;
    const int H = DisplayManager::H;

    c.fillScreen(SP_BG);

    // ── Header ────────────────────────────────────────────────────────────────
    c.fillRect(0, 0, W, HEADER_H, SP_MOSS);

    // Breadcrumb: show depth indicator
    c.setTextFont(2);
    c.setTextColor(SP_GREEN, SP_MOSS);
    c.setCursor(8, 2);
    if (_stack.size() > 1) {
        c.print("< BACK");
    } else {
        c.print("MENU");
    }

    // Counter
    if (!_stack.empty()) {
        char ctr[10];
        snprintf(ctr, sizeof(ctr), "%d/%d",
                 _cur().selectedIdx + 1, (int)_cur().items.size());
        c.setTextFont(1);
        c.setTextColor(SP_DIM, SP_MOSS);
        int tw = c.textWidth(ctr);
        c.setCursor(W - tw - 8, 6);
        c.print(ctr);
    }

    if (_stack.empty() || _cur().items.empty()) {
        _display->push();
        _display->release();
        return;
    }

    // ── Content ───────────────────────────────────────────────────────────────
    const MenuItem& item = _cur().items[_cur().selectedIdx];

    // Vertical separator
    c.drawFastVLine(88, HEADER_H + 4, H - HEADER_H - 8, SP_RULE);

    // Icon — centered in left column
    int iconCx = 44;
    int iconCy = HEADER_H + (H - HEADER_H) / 2;
    int iconSz = 50;

    if (item.iconFn) {
        item.iconFn(c, iconCx, iconCy, iconSz, SP_GREEN);
    } else {
        // Default: hexagon
        for (int i = 0; i < 6; i++) {
            float a0 = i * 3.14159f / 3.0f - 3.14159f / 6.0f;
            float a1 = (i+1) * 3.14159f / 3.0f - 3.14159f / 6.0f;
            c.drawLine(iconCx + (int)(cosf(a0)*iconSz/2),
                       iconCy + (int)(sinf(a0)*iconSz/2),
                       iconCx + (int)(cosf(a1)*iconSz/2),
                       iconCy + (int)(sinf(a1)*iconSz/2), SP_GREEN);
        }
    }

    // Title — choose largest font that fits
    int textX = 96;
    c.setTextFont(4);
    int titleW = c.textWidth(item.label);
    if (titleW > W - textX - 4) {
        c.setTextFont(2);
    }
    c.setTextColor(SP_WHITE, SP_BG);
    c.setCursor(textX, HEADER_H + 14);
    c.print(item.label);

    // Rule
    c.drawFastHLine(textX, HEADER_H + (c.fontHeight() == 26 ? 44 : 32), W - textX - 4, SP_RULE);

    // Type badge
    c.setTextFont(1);
    c.setTextColor(SP_DIM, SP_BG);
    c.setCursor(textX, HEADER_H + 38);
    switch (item.type) {
    case MenuItemType::APP:     c.print("application"); break;
    case MenuItemType::ACTION:  c.print("action");      break;
    case MenuItemType::SUBMENU: c.print("submenu \x10"); break;
    }

    // Select / enter hint
    c.setTextColor(SP_AMBER, SP_BG);
    c.setCursor(textX, HEADER_H + 58);
    c.print("A hold  select");

    // Dot navigation indicators (bottom-center, no bar)
    int n = (int)_cur().items.size();
    int dotStep = 10;
    int dotsW   = (n - 1) * dotStep;
    int dotX    = (W - dotsW) / 2;
    int dotY    = H - 8;
    for (int i = 0; i < n; i++) {
        int dx = dotX + i * dotStep;
        if (i == _cur().selectedIdx) {
            c.fillCircle(dx, dotY, 4, SP_GREEN);
        } else {
            c.fillCircle(dx, dotY, 2, SP_DIM);
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
        _goBack();
    }
}

void MenuOverlay::_selectCurrent() {
    if (_stack.empty() || _cur().items.empty()) return;
    const MenuItem& item = _cur().items[_cur().selectedIdx];

    if (item.type == MenuItemType::SUBMENU && !item.children.empty()) {
        _stack.push_back({ item.children, 0 });
        _dirty = true;
    } else if (item.type == MenuItemType::APP && item.appFactory) {
        AppBase* app = item.appFactory();
        close();
        _apps->launchApp(app);
    } else if (item.type == MenuItemType::ACTION && item.action) {
        close();
        item.action();
    }
}
