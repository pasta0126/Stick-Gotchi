#pragma once
#include "AppBase.h"
#include "ButtonManager.h"

// Manages the currently running app: launching, suspending, resuming, destroying.
// There is always exactly one active app.
class AppManager {
public:
    void begin(ButtonManager& buttons);

    // Replace the current app.
    // Calls destroy() on the old app, then init() on the new one.
    // Does NOT take ownership — caller manages app object lifetime.
    void launchApp(AppBase* app);

    // Called by MenuOverlay when the menu opens/closes.
    void suspendCurrent();
    void resumeCurrent();

    // Call once per loop iteration (only when menu is closed).
    void update(uint32_t deltaMs);

    AppBase* current() const { return _current; }

private:
    AppBase*       _current = nullptr;
    ButtonManager* _buttons = nullptr;

    void _registerInputCallback();
};
