#pragma once
#include <vector>
#include "MenuItem.h"
#include "../core/ButtonManager.h"
#include "../core/DisplayManager.h"
#include "../core/AppManager.h"

class MenuOverlay {
public:
    void begin(ButtonManager& buttons, DisplayManager& display, AppManager& apps);

    void addItem(MenuItem item); // adds to root level

    void open();
    void close();
    bool isOpen() const { return _open; }

    void nextItem();
    void prevItem();

    void update();

private:
    // Navigation stack — index 0 = root, back() = current level
    struct Level {
        std::vector<MenuItem> items;
        int selectedIdx = 0;
        Level() = default;
        Level(std::vector<MenuItem> i, int s = 0)
            : items(std::move(i)), selectedIdx(s) {}
    };
    std::vector<Level> _stack;
    std::vector<MenuItem> _rootItems; // source for stack[0] on open()

    bool _open  = false;
    bool _dirty = true;

    ButtonManager*  _buttons = nullptr;
    DisplayManager* _display = nullptr;
    AppManager*     _apps    = nullptr;

    Level&       _cur()       { return _stack.back(); }
    const Level& _cur() const { return _stack.back(); }

    void _render();
    void _handleInput(const InputEvent& e);
    void _selectCurrent();
    void _goBack(); // pop level or close
};
