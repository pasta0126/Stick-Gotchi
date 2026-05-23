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
    c.setTextFont(2);
    c.setTextColor(_accentColor, SP_MOSS);
    c.setCursor(8, 2);
    c.print(_stack.size() > 1 ? "< BACK" : "MENU");
    // Accent underline
    c.drawFastHLine(0, HEADER_H - 1, W, _accentColor);

    if (_stack.empty() || _cur().items.empty()) {
        _display->push();
        _display->release();
        return;
    }

    // ── 2-column icon grid ────────────────────────────────────────────────────
    int n     = (int)_cur().items.size();
    int rows  = (n + 1) / 2;
    int cellW = W / 2;
    int gridH = H - HEADER_H;
    int cellH = gridH / rows;
    if (cellH > 57) cellH = 57;

    int sel = _cur().selectedIdx;

    for (int i = 0; i < n; i++) {
        int row = i / 2;
        int col = i % 2;
        int x0  = col * cellW;
        int y0  = HEADER_H + row * cellH;
        int cx  = x0 + cellW / 2;
        bool active = (i == sel);

        // Cell background
        if (active) {
            c.fillRect(x0 + 2, y0 + 2, cellW - 4, cellH - 4, SP_MOSS);
            c.drawRect(x0 + 1, y0 + 1, cellW - 2, cellH - 2, _accentColor);
        }

        // Icon — centered in upper portion of cell
        const auto& item = _cur().items[i];
        int iconSz  = cellH - 18;
        int iconCy  = y0 + (cellH - 14) / 2;
        uint32_t icol = active ? _accentColor : SP_DIM;
        if (item.iconFn) {
            item.iconFn(c, cx, iconCy, iconSz, icol);
        } else {
            c.drawCircle(cx, iconCy, iconSz / 3, icol);
        }

        // Label
        c.setTextFont(1);
        c.setTextColor(active ? _accentColor : SP_DIM,
                       active ? SP_MOSS      : SP_BG);
        c.drawCenterString(item.label, cx, y0 + cellH - 11);
    }

    // Grid dividers
    c.drawFastVLine(W / 2, HEADER_H, H - HEADER_H, SP_RULE);
    for (int r = 1; r < rows; r++) {
        c.drawFastHLine(0, HEADER_H + r * cellH, W, SP_RULE);
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
