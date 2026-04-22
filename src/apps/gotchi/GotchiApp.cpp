#include "GotchiApp.h"
#include "../../core/DisplayManager.h"
#include <M5Unified.h>

void GotchiApp::init() {
    _pet.begin();
    _renderer.start(&_pet);
    M5.Mic.begin();
    _actionBarVisible = true;
    _actionBarHideMs  = millis() + 10000;
}

void GotchiApp::update(uint32_t deltaMs) {
    auto dt = M5.Rtc.getDateTime();
    _pet.setHour(dt.time.hours);

    _pet.tick(deltaMs);
    _pollImu(deltaMs);
    _pollMic(deltaMs);
    _renderer.setActionBarState(static_cast<uint8_t>(_selectedAction), _actionBarVisible);

    if (_actionBarVisible && millis() > _actionBarHideMs) {
        _actionBarVisible = false;
    }
}

void GotchiApp::suspend() {
    _renderer.suspend();
}

void GotchiApp::resume() {
    _renderer.resume();
}

void GotchiApp::destroy() {
    _pet.save();
    _renderer.stop();
}

bool GotchiApp::onInput(const InputEvent& e) {
    if (e.button == ButtonId::B && e.action == ButtonAction::SHORT_PRESS) {
        _cycleAction();
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
        _executeAction();
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::LONG_PRESS) {
        _actionBarVisible = !_actionBarVisible;
        if (_actionBarVisible) _actionBarHideMs = millis() + 10000;
        return true;
    }
    return false;
}

// ── Action bar ────────────────────────────────────────────────────────────────

void GotchiApp::_cycleAction() {
    _selectedAction = static_cast<GotchiAction>(
        (static_cast<uint8_t>(_selectedAction) + 1) %
        static_cast<uint8_t>(GotchiAction::COUNT)
    );
    _actionBarVisible = true;
    _actionBarHideMs  = millis() + 10000;
}

void GotchiApp::_executeAction() {
    _actionBarHideMs = millis() + 10000;
    switch (_selectedAction) {
    case GotchiAction::FEED:     _pet.feed();  break;
    case GotchiAction::PLAY:     _pet.play();  break;
    case GotchiAction::MEDICINE: /* TODO: Phase MVP */ break;
    case GotchiAction::LIGHT:    /* TODO: Phase MVP */ break;
    case GotchiAction::CLEAN:    /* TODO: Phase MVP */ break;
    default: break;
    }
}

// ── IMU polling ───────────────────────────────────────────────────────────────

void GotchiApp::_pollImu(uint32_t deltaMs) {
    _imuPollAccum += deltaMs;
    if (_imuPollAccum < 50) return;
    _imuPollAccum = 0;

    float ax, ay, az;
    M5.Imu.getAccel(&ax, &ay, &az);
    float mag = sqrtf(ax * ax + ay * ay + az * az);

    // Shake detection
    float delta = fabsf(mag - _prevAccMag);
    uint32_t now = millis();
    if (delta > 0.5f) {
        if (_shakeCount == 0) _shakeWindowMs = now;
        _shakeCount++;
        if (_shakeCount >= 3 && (now - _shakeWindowMs) < 600) {
            // Map intensity: 3-4 hits = soft, 5-6 = medium, 7+ = hard
            uint8_t intensity = (_shakeCount <= 4) ? 0 :
                                (_shakeCount <= 6) ? 1 : 2;
            if (delta > 2.0f) intensity = 3; // violent single spike
            _pet.onShake(intensity);
            _shakeCount = 0;
        }
    } else if ((now - _shakeWindowMs) > 800) {
        _shakeCount = 0;
    }
    _prevAccMag = mag;

    // Tilt → gaze
    static constexpr float TILT_DEAD = 0.15f;
    static constexpr float TILT_MAX  = 0.80f;
    float gazeH = 0.0f;
    if (fabsf(ax) > TILT_DEAD) {
        float norm = (fabsf(ax) - TILT_DEAD) / (TILT_MAX - TILT_DEAD);
        gazeH = copysignf(min(1.0f, norm), ax);
    }
    _renderer.setGaze(gazeH, 0.0f);
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

    // Rough dB estimate relative to int16 max (32768)
    uint8_t db = (rms > 0) ? (uint8_t)min(100.0f, 20.0f * log10f(rms / 32768.0f) + 100.0f) : 0;
    _pet.onNoiseLevel(db);
}
