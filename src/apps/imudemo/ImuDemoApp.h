#pragma once
#include "../../core/AppBase.h"
#include "../../core/DisplayManager.h"

// Visualises raw MPU6886 accelerometer + gyroscope data in real time.
class ImuDemoApp : public AppBase {
public:
    void inject(DisplayManager* display) { _display = display; }

    void init()                       override;
    void update(uint32_t deltaMs)     override;
    void suspend()                    override {}
    void resume()                     override {}
    void destroy()                    override {}
    bool onInput(const InputEvent& e) override {
        if (e.button == ButtonId::B && e.action == ButtonAction::LONG_PRESS) {
            if (_menuCallback) _menuCallback();
            return true;
        }
        return false;
    }
    const char* getName() const       override { return "IMU Demo"; }

private:
    DisplayManager* _display = nullptr;
    uint32_t        _drawAccum = 0;

    void _drawFrame(float ax, float ay, float az,
                    float gx, float gy, float gz);
    void _drawBar(int x, int y, int w, int h,
                  float val, float minV, float maxV,
                  uint32_t color, const char* label);
};
