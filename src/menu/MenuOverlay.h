#pragma once
#include <vector>
#include "MenuItem.h"
#include "../core/ButtonManager.h"
#include "../core/DisplayManager.h"
#include "../core/AppManager.h"

// Lightweight menu rendered on the main loop thread.
// When open: steals button input, suspends the current app's render task,
//            and draws a full-screen list over the display.
// When closed: restores input to AppManager, resumes the suspended app.
class MenuOverlay {
public:
    void begin(ButtonManager& buttons, DisplayManager& display, AppManager& apps);

    void addItem(MenuItem item);

    void open();
    void close();
    bool isOpen() const { return _open; }

    // Navigate items (called from main loop: B=next, C=prev via M5.BtnC)
    void nextItem();
    void prevItem();

    // Call from main loop while isOpen() is true.
    void update();

private:
    std::vector<MenuItem> _items;
    int                   _selectedIdx = 0;
    bool                  _open        = false;
    bool                  _dirty       = true; // redraw needed

    ButtonManager*  _buttons = nullptr;
    DisplayManager* _display = nullptr;
    AppManager*     _apps    = nullptr;

    void _render();
    void _handleInput(const InputEvent& e);
    void _selectCurrent();
};
