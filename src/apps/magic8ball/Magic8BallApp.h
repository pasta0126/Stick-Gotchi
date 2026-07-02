#pragma once
#include "../../core/AppBase.h"
#include "../../core/DisplayManager.h"
#include "Magic8BallGame.h"

class Magic8BallApp : public AppBase {
public:
    void inject(DisplayManager* display) { _display = display; }

    void init()                       override;
    void update(uint32_t deltaMs)     override;
    void suspend()                    override {}
    void resume()                     override {}
    void destroy()                    override {}
    bool onInput(const InputEvent& e) override;
    const char* getName() const       override { return "Magic 8-Ball"; }

private:
    DisplayManager* _display     = nullptr;
    Magic8BallGame  _game;
    uint32_t        _drawAccum   = 0;
    uint32_t        _imuPollAccum = 0;
    float           _prevAccMag  = 1.0f;

    void _pollImu(uint32_t deltaMs);
    void _draw();
};
