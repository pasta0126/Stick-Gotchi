#pragma once
#include "../../core/AppBase.h"
#include "../../ble/BleService.h"
#include "GotchiPet.h"
#include "GotchiRenderer.h"

class GotchiApp : public AppBase {
public:
    void init()                       override;
    void update(uint32_t deltaMs)     override;
    void suspend()                    override;
    void resume()                     override;
    void destroy()                    override;
    bool onInput(const InputEvent& e) override;
    const char* getName() const       override { return "Stick Gotchi"; }

    void inject(BleService* ble) { _ble = ble; }

private:
    GotchiPet      _pet;
    GotchiRenderer _renderer;
    BleService*    _ble = nullptr;

    uint32_t _lastBleNotify = 0;
    uint32_t _imuPollAccum  = 0;
    uint32_t _micPollAccum  = 0;

    // Step detection state
    float    _prevAccMag    = 1.0f;
    bool     _stepHigh      = false;
    uint32_t _lastStepMs    = 0;
    int      _shakeCount    = 0;
    uint32_t _shakeWindowMs = 0;

    void _pollImu(uint32_t deltaMs);
    void _pollMic(uint32_t deltaMs);
    void _syncBle();
};
