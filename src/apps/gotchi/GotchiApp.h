#pragma once
#include "../../core/AppBase.h"
#include "../../core/DisplayManager.h"
#include "GotchiPet.h"
#include "GotchiRenderer.h"
#include "MiniGames.h"
#include "FlipCoinGame.h"
#include "Magic8BallGame.h"

enum class GotchiAction : uint8_t {
    FEED = 0, PLAY, MEDICINE, LIGHT, CLEAN, COUNT
};

class GotchiApp : public AppBase {
public:
    void init()                       override;
    void update(uint32_t deltaMs)     override;
    void suspend()                    override;
    void resume()                     override;
    void destroy()                    override;
    bool onInput(const InputEvent& e) override;
    const char* getName() const       override { return "Stick Gotchi"; }

    void injectRenderer(DisplayManager* display) { _renderer.inject(display); }
    void startMiniGame(MiniGameId id);

private:
    GotchiPet      _pet;
    GotchiRenderer _renderer;
    FlipCoinGame   _flipCoin;
    Magic8BallGame _magic8Ball;
    MiniGameId     _activeMiniGame = MiniGameId::NONE;

    // Action bar state
    GotchiAction _selectedAction  = GotchiAction::FEED;
    bool         _actionBarVisible = true;
    uint32_t     _actionBarHideMs  = 0;  // hide after 10s of inactivity

    // IMU polling
    uint32_t _imuPollAccum  = 0;
    float    _prevAccMag    = 1.0f;
    int      _shakeCount    = 0;
    uint32_t _shakeWindowMs = 0;

    // Mic polling
    uint32_t _micPollAccum  = 0;

    void _cycleAction();
    void _executeAction();
    void _pollImu(uint32_t deltaMs);
    void _pollImuForBall(uint32_t deltaMs);
    void _pollMic(uint32_t deltaMs);
};
