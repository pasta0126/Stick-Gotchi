#include "MenuOverlay.h"

// Layout constants
static constexpr int  ITEM_H      = 28;
static constexpr int  PADDING     = 8;
static constexpr int  TITLE_H     = 22;
static constexpr uint32_t BG_COLOR  = 0x1A1A2E; // dark navy
static constexpr uint32_t SEL_COLOR = 0xE94560; // accent red
static constexpr uint32_t TXT_COLOR = TFT_WHITE;
static constexpr uint32_t DIM_COLOR = 0xAAAAAA;

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
    _apps->resumeCurrent(); // restores AppManager callback internally
}

void MenuOverlay::update() {
    if (!_dirty) return;
    _dirty = false;
    _render();
}

// ---------------------------------------------------------------------------

void MenuOverlay::_render() {
    if (!_display->acquire(100)) return;

    auto& c = _display->canvas();
    c.fillScreen(BG_COLOR);

    // Title bar
    c.fillRect(0, 0, DisplayManager::W, TITLE_H, SEL_COLOR);
    c.setTextColor(TFT_WHITE, SEL_COLOR);
    c.setTextSize(1);
    c.setTextFont(2);
    c.setCursor(PADDING, 4);
    c.print("MENU");

    // Current app label (right-aligned in title bar)
    if (_apps->current()) {
        const char* name = _apps->current()->getName();
        int tw = c.textWidth(name);
        c.setCursor(DisplayManager::W - tw - PADDING, 4);
        c.print(name);
    }

    // Menu items
    for (int i = 0; i < (int)_items.size(); i++) {
        int y = TITLE_H + i * ITEM_H;
        bool selected = (i == _selectedIdx);

        if (selected) {
            c.fillRect(0, y, DisplayManager::W, ITEM_H, SEL_COLOR);
            c.setTextColor(TFT_WHITE, SEL_COLOR);
        } else {
            c.fillRect(0, y, DisplayManager::W, ITEM_H, BG_COLOR);
            c.setTextColor(DIM_COLOR, BG_COLOR);
        }

        // Arrow indicator for selected item
        if (selected) {
            c.setCursor(PADDING, y + 6);
            c.print("> ");
        } else {
            c.setCursor(PADDING + 14, y + 6);
        }
        c.print(_items[i].label);

        // Type badge
        const char* badge = (_items[i].type == MenuItemType::APP) ? "[APP]" : "[ACT]";
        int bw = c.textWidth(badge);
        c.setCursor(DisplayManager::W - bw - PADDING, y + 6);
        c.setTextColor(selected ? TFT_WHITE : 0x555555,
                       selected ? SEL_COLOR : BG_COLOR);
        c.print(badge);
    }

    // Footer hint
    int footerY = DisplayManager::H - 14;
    c.fillRect(0, footerY, DisplayManager::W, 14, 0x0D0D1A);
    c.setTextColor(0x666666, 0x0D0D1A);
    c.setTextFont(1);
    c.setCursor(PADDING, footerY + 2);
    c.print("A:next  A(hold):select  B:back");

    _display->push();
    _display->release();
}

void MenuOverlay::_handleInput(const InputEvent& e) {
    if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
        _selectedIdx = (_selectedIdx + 1) % (int)_items.size();
        _dirty = true;
    } else if (e.button == ButtonId::A && e.action == ButtonAction::LONG_PRESS) {
        _selectCurrent();
    } else if (e.button == ButtonId::B && e.action == ButtonAction::SHORT_PRESS) {
        close();
    }
}

void MenuOverlay::_selectCurrent() {
    if (_selectedIdx < 0 || _selectedIdx >= (int)_items.size()) return;
    MenuItem& item = _items[_selectedIdx];

    if (item.type == MenuItemType::APP && item.appFactory) {
        AppBase* app = item.appFactory();
        close(); // resume is called here, but launchApp will init the new one
        _apps->launchApp(app);
    } else if (item.type == MenuItemType::ACTION && item.action) {
        close();
        item.action();
    }
}
