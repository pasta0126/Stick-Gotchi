#pragma once
#include <Arduino.h>
#include <functional>
#include "InputEvent.h"

// Abstract base class for all apps.
// AppManager calls these lifecycle methods; every app must implement them.
class AppBase {
public:
    virtual ~AppBase() = default;

    // Called once when the app is first launched or re-launched from the menu.
    virtual void init() = 0;

    // Called every loop tick while this app is in the foreground and not suspended.
    virtual void update(uint32_t deltaMs) = 0;

    // Called when the menu opens over this app — pause rendering tasks, stop BLE if needed.
    virtual void suspend() = 0;

    // Called when the menu closes and this app returns to foreground.
    virtual void resume() = 0;

    // Called when another app replaces this one — free resources, stop tasks.
    virtual void destroy() = 0;

    // Called for every button event while this app is foreground and not suspended.
    // Return true to consume the event (prevent further dispatch).
    virtual bool onInput(const InputEvent& /*event*/) { return false; }

    // Label shown in the menu.
    virtual const char* getName() const = 0;

    // Called by main.cpp so apps can request the menu to open (e.g. on BtnB long press).
    void setMenuCallback(std::function<void()> cb) { _menuCallback = std::move(cb); }

protected:
    std::function<void()> _menuCallback;
};
