#include "GotchiApp.h"
#include "../../core/DisplayManager.h"
#include <M5Unified.h>

void GotchiApp::init() {
    _pet.begin();
    _renderer.start(&_pet);
    if (_pendingTransitionColor) {
        _renderer.beginTransition(_pendingTransitionColor);
        _pendingTransitionColor = 0;
    }
    M5.Mic.begin();
}

void GotchiApp::update(uint32_t deltaMs) {
    if (_activeMiniGame == MiniGameId::FLIP_COIN) {
        _flipCoin.update(deltaMs);
        _renderer.setMiniGame(
            static_cast<uint8_t>(_activeMiniGame),
            static_cast<uint8_t>(_flipCoin.state()),
            _flipCoin.coinFrame(),
            _flipCoin.isHeads()
        );
        return;
    }

    if (_activeMiniGame == MiniGameId::MAGIC_8BALL) {
        _pollImuForBall(deltaMs);
        _magic8Ball.update(deltaMs);
        _renderer.setMiniGame(
            static_cast<uint8_t>(_activeMiniGame),
            static_cast<uint8_t>(_magic8Ball.state()),
            _magic8Ball.frame(),
            _magic8Ball.resultId()
        );
        return;
    }

    _pet.tick(deltaMs);
    _pollImu(deltaMs);
    _pollMic(deltaMs);
    _renderer.setMiniGame(0, 0, 0, false);
    _renderer.setActionBarState(0, false);  // action bar removed in v2
}

void GotchiApp::startMiniGame(MiniGameId id) {
    _activeMiniGame = id;
    if (id == MiniGameId::FLIP_COIN)
        _flipCoin.start();
    else if (id == MiniGameId::MAGIC_8BALL)
        _magic8Ball.reset();
}

void GotchiApp::suspend() { _renderer.suspend(); }
void GotchiApp::resume()  { _renderer.resume();  }

void GotchiApp::destroy() {
    _pet.save();
    _renderer.stop();
}

bool GotchiApp::onInput(const InputEvent& e) {
    if (_activeMiniGame == MiniGameId::MAGIC_8BALL) {
        if (e.button == ButtonId::B) {
            _activeMiniGame = MiniGameId::NONE;
            _renderer.setMiniGame(0, 0, 0, false);
        } else if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
            if (_magic8Ball.state() == BallState::RESULT)
                _magic8Ball.reset();
        }
        return true;
    }

    if (_activeMiniGame == MiniGameId::FLIP_COIN) {
        if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
            _flipCoin.onBtnA();
            return true;
        }
        if (e.button == ButtonId::B) {
            _activeMiniGame = MiniGameId::NONE;
            _renderer.setMiniGame(0, 0, 0, false);
            return true;
        }
        return true;
    }

    if (e.button == ButtonId::B && e.action == ButtonAction::LONG_PRESS) {
        _renderer.showMoodPeek();
        return true;
    }
    if (e.button == ButtonId::B && e.action == ButtonAction::SHORT_PRESS) {
        _pet.onBtnB();
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::LONG_PRESS) {
        if (_menuCallback) _menuCallback();
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
        _pet.onBtnA();
        return true;
    }
    return false;
}

// ── IMU polling ───────────────────────────────────────────────────────────────

void GotchiApp::_pollImu(uint32_t deltaMs) {
    static constexpr uint32_t POLL_MS        = 50;
    static constexpr float    STILL_DEV      = 0.10f;  // |mag-1g| < this → near-gravity
    static constexpr float    TAP_DELTA      = 0.65f;  // single spike → tap
    static constexpr float    SHAKE_DELTA    = 0.50f;  // rapid spikes → shake
    static constexpr float    WALK_DELTA_MIN = 0.05f;  // walking oscillation range
    static constexpr float    WALK_DELTA_MAX = 0.38f;
    static constexpr uint32_t PICKUP_STILL   = 3000;   // must be still this long for pickup
    static constexpr uint32_t PUTDOWN_ACTIVE = 1500;   // must be active this long for putdown
    static constexpr uint32_t WALK_CONFIRM   = 600;    // ms of walk-like motion to fire event

    _imuPollAccum += deltaMs;
    if (_imuPollAccum < POLL_MS) return;
    _imuPollAccum = 0;

    // Tick event cooldowns
    auto tickCd = [&](uint32_t& cd) { cd = (cd > POLL_MS) ? cd - POLL_MS : 0; };
    tickCd(_pickupCooldown);
    tickCd(_putdownCooldown);
    tickCd(_walkingCooldown);

    float ax, ay, az;
    M5.Imu.getAccel(&ax, &ay, &az);
    float mag   = sqrtf(ax*ax + ay*ay + az*az);
    float delta = fabsf(mag - _prevAccMag);
    float dev   = fabsf(mag - 1.0f);
    _prevAccMag = mag;

    uint32_t now = millis();

    // ── TAP: single sharp spike (not already in a shake sequence) ─────
    if (delta > TAP_DELTA && _shakeCount <= 1) {
        _pet.onImuTap();
        _shakeCount = 0;
        goto gaze;  // skip shake/motion logic this sample
    }

    // ── SHAKE: rapid multi-spike ───────────────────────────────────────
    if (delta > SHAKE_DELTA) {
        if (_shakeCount == 0) _shakeWindowMs = now;
        _shakeCount++;
        if (_shakeCount >= 3 && (now - _shakeWindowMs) < 600) {
            _pet.onImuShake();
            _shakeCount = 0;
        }
    } else if ((now - _shakeWindowMs) > 800) {
        _shakeCount = 0;
    }

    {
        // ── PICKUP / PUTDOWN (motion state transitions) ────────────────
        bool nearGravity = (dev < STILL_DEV);

        // Save previous accumulated values before update
        uint32_t prevStill  = _stillMs;
        uint32_t prevActive = _activeMs;

        if (nearGravity) {
            _stillMs  = (_stillMs  < 60000u) ? _stillMs  + POLL_MS : 60000u;
            _activeMs = 0;
            // Transition ACTIVE→STILL: putdown
            if (!_wasNearGravity && prevActive >= PUTDOWN_ACTIVE && _putdownCooldown == 0) {
                _pet.onImuPutdown();
                _putdownCooldown = 3000;
            }
        } else {
            _activeMs = (_activeMs < 60000u) ? _activeMs + POLL_MS : 60000u;
            _stillMs  = 0;
            // Transition STILL→ACTIVE: pickup (only if was still long enough)
            if (_wasNearGravity && prevStill >= PICKUP_STILL && _pickupCooldown == 0) {
                _pet.onImuPickup();
                _pickupCooldown = 5000;
            }
        }
        _wasNearGravity = nearGravity;

        // ── WALKING: sustained low-amplitude oscillation ───────────────
        bool walkLike = (delta >= WALK_DELTA_MIN && delta <= WALK_DELTA_MAX && dev < 0.30f);
        if (walkLike) {
            _walkMs = (_walkMs < 10000u) ? _walkMs + POLL_MS : 10000u;
        } else {
            _walkMs = (_walkMs > POLL_MS) ? _walkMs - POLL_MS : 0;
        }
        if (_walkMs >= WALK_CONFIRM && _walkingCooldown == 0) {
            _pet.onImuWalking();
            _walkingCooldown = 2000;
        }
    }

    gaze:
    // ── Tilt → renderer gaze ──────────────────────────────────────────
    {
        static constexpr float TILT_DEAD = 0.15f;
        static constexpr float TILT_MAX  = 0.80f;
        float gazeH = 0.0f;
        if (fabsf(ax) > TILT_DEAD) {
            float norm = (fabsf(ax) - TILT_DEAD) / (TILT_MAX - TILT_DEAD);
            gazeH = copysignf(min(1.0f, norm), ax);
        }
        _renderer.setGaze(gazeH, 0.0f);
    }
}

void GotchiApp::_pollImuForBall(uint32_t deltaMs) {
    _imuPollAccum += deltaMs;
    if (_imuPollAccum < 50) return;
    _imuPollAccum = 0;

    float ax, ay, az;
    M5.Imu.getAccel(&ax, &ay, &az);
    float mag = sqrtf(ax * ax + ay * ay + az * az);
    float delta = fabsf(mag - _prevAccMag);
    _prevAccMag = mag;
    _magic8Ball.onAccelDelta(delta);
}

// ── Mic polling ───────────────────────────────────────────────────────────────

void GotchiApp::_pollMic(uint32_t deltaMs) {
    _micPollAccum += deltaMs;
    if (_micPollAccum < 100) return;
    _micPollAccum = 0;

    static int16_t buf[32];
    if (!M5.Mic.record(buf, 32, 8000)) return;

    int32_t sumSq = 0;
    for (auto& s : buf) sumSq += (int32_t)s * s;
    float rms = sqrtf((float)sumSq / 32);
    _pet.onNoiseLevel(rms);
}
