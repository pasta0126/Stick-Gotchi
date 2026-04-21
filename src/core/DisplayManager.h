#pragma once
#include <M5Unified.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Wraps the M5Unified display with a FreeRTOS mutex so the main loop
// and FreeRTOS render tasks can safely share the screen.
// All draw code must bracket with acquire/release.
class DisplayManager {
public:
    static constexpr int W = 240;
    static constexpr int H = 135;

    void begin();

    // Acquire the display mutex before drawing. Blocks until available.
    // Returns false only if timeoutMs elapses without acquiring.
    bool acquire(uint32_t timeoutMs = portMAX_DELAY);
    void release();

    // Push the canvas contents to the physical display.
    // Must be called while mutex is held.
    void push();

    M5Canvas& canvas() { return _canvas; }

private:
    SemaphoreHandle_t _mutex  = nullptr;
    M5Canvas          _canvas{&M5.Display};
};
