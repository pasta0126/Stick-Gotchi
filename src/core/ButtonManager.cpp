#include "ButtonManager.h"
#include <M5Unified.h>

void ButtonManager::begin(uint32_t longPressMs) {
    _longPressMs = longPressMs;
}

void ButtonManager::update() {
    uint32_t now = millis();
    _process(M5.BtnA.isPressed(), 0, ButtonId::A, now);
    _process(M5.BtnB.isPressed(), 1, ButtonId::B, now);
}

void ButtonManager::setCallback(InputCallback cb) {
    _callback = cb;
}

void ButtonManager::_process(bool isDown, int idx, ButtonId id, uint32_t now) {
    BtnState& s = _states[idx];

    if (isDown && !s.wasDown) {
        // Button just pressed
        s.pressStart = now;
        s.longFired  = false;
    } else if (isDown && s.wasDown) {
        // Button held — check for long press threshold
        if (!s.longFired && (now - s.pressStart) >= _longPressMs) {
            s.longFired = true;
            if (_callback) {
                InputEvent e{id, ButtonAction::LONG_PRESS, now};
                _callback(e);
            }
        }
    } else if (!isDown && s.wasDown) {
        // Button just released
        if (!s.longFired) {
            // Was a short press
            if (_callback) {
                InputEvent e{id, ButtonAction::SHORT_PRESS, now};
                _callback(e);
            }
        }
    }

    s.wasDown = isDown;
}
