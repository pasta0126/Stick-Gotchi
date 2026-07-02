#pragma once
#include "../../core/AppBase.h"
#include "../../core/DisplayManager.h"
#include "FlipCoinGame.h"

class CoinFlipApp : public AppBase {
public:
    void inject(DisplayManager* display) { _display = display; }

    void init()                       override;
    void update(uint32_t deltaMs)     override;
    void suspend()                    override {}
    void resume()                     override {}
    void destroy()                    override {}
    bool onInput(const InputEvent& e) override;
    const char* getName() const       override { return "Coin Flip"; }

private:
    DisplayManager* _display   = nullptr;
    FlipCoinGame    _game;
    uint32_t        _drawAccum = 0;

    void _draw();
};
