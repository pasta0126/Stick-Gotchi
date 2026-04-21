#pragma once
#include "../../core/AppBase.h"
#include "../../core/DisplayManager.h"

enum class ImuMode { ACCEL, GYRO, ORIENT };

class ImuDemoApp : public AppBase {
public:
    void inject(DisplayManager* display) { _display = display; }
    void setMode(ImuMode mode)           { _mode = mode; }

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
    const char* getName() const override {
        switch (_mode) {
        case ImuMode::ACCEL:  return "Accelerometer";
        case ImuMode::GYRO:   return "Gyroscope";
        case ImuMode::ORIENT: return "Orientation";
        }
        return "IMU";
    }

private:
    DisplayManager* _display  = nullptr;
    ImuMode         _mode     = ImuMode::ACCEL;
    uint32_t        _drawAccum = 0;

    void _drawAccel(float ax, float ay, float az);
    void _drawGyro(float gx, float gy, float gz);
    void _drawOrient(float ax, float ay, float az);
};
