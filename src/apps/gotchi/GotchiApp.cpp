#include "GotchiApp.h"
#include <M5Unified.h>

void GotchiApp::init() {
    _pet.begin();

    // Wire BLE callbacks
    _ble->setCommandCallback([this](BleCommand cmd) {
        switch (cmd) {
        case BleCommand::FEED:  _pet.feed();  break;
        case BleCommand::DRINK: _pet.drink(); break;
        case BleCommand::PET:   _pet.pet();   break;
        case BleCommand::PLAY:  _pet.play();  break;
        }
    });
    _ble->setBatteryCallback([this](uint8_t /*level*/, bool charging) {
        _pet.setPhoneBatteryLow(!charging);
    });
    _ble->setContextCallback([this](uint8_t hour, int8_t tempC) {
        _pet.setContext(hour, tempC);
    });

    _ble->start("StickGotchi");
    _renderer.start(&_pet);

    M5.Mic.begin();
    Serial.println("[GotchiApp] init");
}

void GotchiApp::update(uint32_t deltaMs) {
    _pet.tick(deltaMs);
    _pollImu(deltaMs);
    _pollMic(deltaMs);
    _renderer.updateExpression();
    _syncBle();
}

void GotchiApp::suspend() {
    _renderer.suspend();
    // Keep BLE running so the Android app stays aware the gotchi is paused.
}

void GotchiApp::resume() {
    _renderer.resume();
}

void GotchiApp::destroy() {
    _pet.save(); // persist stats before shutting down
    _renderer.stop();
    _ble->stop();
}

bool GotchiApp::onInput(const InputEvent& e) {
    if (e.button == ButtonId::A) {
        _pet.pet();
        return true;
    }
    if (e.button == ButtonId::B && e.action == ButtonAction::SHORT_PRESS) {
        _pet.play();
        return true;
    }
    if (e.button == ButtonId::B && e.action == ButtonAction::LONG_PRESS) {
        if (_menuCallback) _menuCallback();
        return true;
    }
    return false;
}

// ── IMU polling ───────────────────────────────────────────────────────────────

void GotchiApp::_pollImu(uint32_t deltaMs) {
    _imuPollAccum += deltaMs;
    if (_imuPollAccum < 50) return; // poll at ~20 Hz
    _imuPollAccum = 0;

    float ax, ay, az;
    M5.Imu.getAccel(&ax, &ay, &az);

    // Step detection — peak-valley on acceleration magnitude
    float mag = sqrtf(ax * ax + ay * ay + az * az);
    uint32_t now = millis();

    if (!_stepHigh && mag > 1.25f && (now - _lastStepMs) > 300) {
        _stepHigh = true;
    }
    if (_stepHigh && mag < 0.85f) {
        _stepHigh   = false;
        _lastStepMs = now;
        _pet.onStep();
    }

    // Shake detection — large rapid changes
    float delta = fabsf(mag - _prevAccMag);
    if (delta > 0.8f) {
        if (_shakeCount == 0) _shakeWindowMs = now;
        _shakeCount++;
        if (_shakeCount >= 4 && (now - _shakeWindowMs) < 600) {
            _pet.onShake();
            _shakeCount = 0;
        }
    } else if ((now - _shakeWindowMs) > 800) {
        _shakeCount = 0;
    }
    _prevAccMag = mag;

    // Orientation: face down = az strongly negative
    _pet.onFaceDown(az < -0.8f);

    // Tilt → gaze: ax drives left/right eye movement in landscape orientation
    static constexpr float TILT_DEAD = 0.15f;
    static constexpr float TILT_MAX  = 0.80f;
    float gazeH = 0.0f;
    if (fabsf(ax) > TILT_DEAD) {
        float norm = (fabsf(ax) - TILT_DEAD) / (TILT_MAX - TILT_DEAD);
        gazeH = copysignf(min(1.0f, norm), ax); // negative=left, positive=right
    }
    _renderer.setGaze(gazeH, 0.0f);
}

// ── Microphone polling ────────────────────────────────────────────────────────

void GotchiApp::_pollMic(uint32_t deltaMs) {
    _micPollAccum += deltaMs;
    if (_micPollAccum < 100) return; // sample at ~10 Hz
    _micPollAccum = 0;

    static int16_t buf[32];
    if (!M5.Mic.record(buf, 32, 8000)) return;

    int16_t peak = 0;
    for (auto& s : buf) {
        int16_t v = abs(s);
        if (v > peak) peak = v;
    }
    // ~3000 ≈ 9 % of int16 max; covers a sharp clap without false-triggering on speech.
    if (peak > 3000) _pet.onSoundEvent();
}

// ── BLE state sync ────────────────────────────────────────────────────────────

void GotchiApp::_syncBle() {
    uint32_t now = millis();
    bool shouldNotify = _pet.moodChanged() || (now - _lastBleNotify) >= 3000;
    if (!shouldNotify) return;

    _pet.clearMoodChanged();
    _lastBleNotify = now;

    PetStats    s = _pet.stats();
    BleGotchiState state{};
    state.mood   = (uint8_t)_pet.mood();
    state.hunger = s.hunger;
    state.thirst = s.thirst;
    state.energy = s.energy;
    state.steps  = s.steps;
    state.flags  = _pet.phoneBatLow() ? 0x01 : 0x00;
    _ble->notifyState(state);
}
