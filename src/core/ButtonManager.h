#pragma once
#include <functional>
#include "InputEvent.h"

using InputCallback = std::function<void(const InputEvent&)>;

// Polls M5Unified buttons A and B each loop tick and fires InputEvents.
// Button C (power) is handled directly in main.cpp as a meta-level control.
class ButtonManager {
public:
    // longPressMs: hold duration in ms to trigger LONG_PRESS.
    void begin(uint32_t longPressMs = 700);

    // Call once per loop iteration, after M5.update().
    void update();

    // Replace the current event callback. MenuOverlay temporarily steals this;
    // AppManager restores it when the menu closes.
    void setCallback(InputCallback cb);

private:
    InputCallback _callback;
    uint32_t      _longPressMs = 700;

    struct BtnState {
        bool     wasDown   = false;
        uint32_t pressStart = 0;
        bool     longFired = false;
    } _states[2]; // index 0 = BtnA, 1 = BtnB

    void _process(bool isDown, int idx, ButtonId id, uint32_t now);
};
