#pragma once
#include <functional>
#include "../../core/AppBase.h"
#include "../../core/DisplayManager.h"
#include "../../gotchi/GotchiBehaviour.h"

class CreatureSelectorApp : public AppBase {
public:
    void inject(DisplayManager* display) { _display = display; }
    void setConfirmCallback(std::function<void(CreatureType)> cb) { _confirmCb = std::move(cb); }
    void openAt(CreatureType initial) { _preview = initial; _needsDraw = true; }

    void init()                       override { _needsDraw = true; }
    void update(uint32_t deltaMs)     override;
    void suspend()                    override {}
    void resume()                     override { _needsDraw = true; }
    void destroy()                    override {}
    bool onInput(const InputEvent& e) override;
    const char* getName() const       override { return "Criatura"; }

private:
    DisplayManager* _display   = nullptr;
    std::function<void(CreatureType)> _confirmCb;

    CreatureType _preview    = CreatureType::BYTEE;
    uint32_t     _animAccum  = 0;
    uint8_t      _animFrame  = 0;
    bool         _needsDraw  = true;

    void _drawFrame();
    void _cycle();
};
