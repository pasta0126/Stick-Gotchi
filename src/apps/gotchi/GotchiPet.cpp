#include "GotchiPet.h"
#include <Arduino.h>
#include <Preferences.h>

void GotchiPet::begin() {
    _stats.hunger = 80;
    _stats.thirst = 80;
    _stats.energy = 80;
    _stats.steps  = 0;
    load(); // overwrite defaults with saved values if they exist
    _mood        = Mood::HAPPY;
    _moodChanged = true;
}

void GotchiPet::save() {
    Preferences prefs;
    prefs.begin("gotchi", false);
    prefs.putUChar("h", _stats.hunger);
    prefs.putUChar("t", _stats.thirst);
    prefs.putUChar("e", _stats.energy);
    prefs.putUShort("s", _stats.steps);
    prefs.end();
    Serial.println("[Pet] Stats saved to NVS");
}

void GotchiPet::load() {
    Preferences prefs;
    prefs.begin("gotchi", true);
    if (!prefs.isKey("h")) { prefs.end(); return; } // first boot — keep defaults
    _stats.hunger = prefs.getUChar("h",  _stats.hunger);
    _stats.thirst = prefs.getUChar("t",  _stats.thirst);
    _stats.energy = prefs.getUChar("e",  _stats.energy);
    _stats.steps  = prefs.getUShort("s", _stats.steps);
    prefs.end();
    Serial.println("[Pet] Stats loaded from NVS");
}

// ── Main simulation tick ─────────────────────────────────────────────────────

void GotchiPet::tick(uint32_t deltaMs) {
    uint32_t now = millis();

    // Clear expired temporary mood
    if (_tempMood != Mood::NEUTRAL && now >= _moodExpiry) {
        _tempMood = Mood::NEUTRAL;
    }

    // Face-down anger (30 s)
    if (_faceDown && (now - _faceDownTs) > 30000) {
        _setTempMood(Mood::ANGRY, 6000);
    }

    // Stat decay (every DECAY_INTERVAL_MS)
    _decayAccum += deltaMs;
    if (_decayAccum >= DECAY_INTERVAL_MS) {
        _decayAccum -= DECAY_INTERVAL_MS;

        bool sleeping = (_mood == Mood::SLEEPING);

        if (!sleeping) {
            _stats.hunger = (_stats.hunger > DECAY_HUNGER) ? _stats.hunger - DECAY_HUNGER : 0;
            _stats.thirst = (_stats.thirst > DECAY_THIRST) ? _stats.thirst - DECAY_THIRST : 0;
            _stats.energy = (_stats.energy > DECAY_ENERGY) ? _stats.energy - DECAY_ENERGY : 0;
        } else {
            // Recover energy while sleeping
            _stats.energy = min(100, (int)_stats.energy + 3);
        }
    }

    _recalcMood();

    // Periodic NVS save (every SAVE_INTERVAL_MS)
    _saveAccum += deltaMs;
    if (_saveAccum >= SAVE_INTERVAL_MS) {
        _saveAccum -= SAVE_INTERVAL_MS;
        save();
    }
}

// ── Commands ─────────────────────────────────────────────────────────────────

void GotchiPet::feed() {
    _stats.hunger = min(100, (int)_stats.hunger + 25);
    _setTempMood(Mood::HAPPY, 3000);
}

void GotchiPet::drink() {
    _stats.thirst = min(100, (int)_stats.thirst + 25);
    _setTempMood(Mood::HAPPY, 3000);
}

void GotchiPet::pet() {
    uint32_t now = millis();
    if (_petClicks == 0) _petClicksTs = now;
    _petClicks++;

    // Rage: 5+ clicks in 2 s
    if (_petClicks >= 5 && (now - _petClicksTs) <= 2000) {
        _setTempMood(Mood::ANGRY, 6000);
        _petClicks = 0;
        return;
    }
    // Happy petting: 5+ clicks in 8 s
    if (_petClicks >= 5 && (now - _petClicksTs) <= 8000) {
        _setTempMood(Mood::HAPPY, 4000);
        _petClicks = 0;
        return;
    }
    // Reset window if too slow
    if ((now - _petClicksTs) > 8000) _petClicks = 1;
}

void GotchiPet::play() {
    _stats.energy = (_stats.energy > 10) ? _stats.energy - 10 : 0;
    _setTempMood(Mood::EXCITED, 5000);
}

// ── Sensor inputs ─────────────────────────────────────────────────────────────

void GotchiPet::onStep() {
    if (_stats.steps < 65535) _stats.steps++;
    // Walking costs a little hunger/thirst
    if (_stats.steps % 100 == 0) {
        if (_stats.hunger > 1) _stats.hunger--;
        if (_stats.thirst > 1) _stats.thirst--;
    }
}

void GotchiPet::onShake() {
    _setTempMood(Mood::DIZZY, 4000);
}

void GotchiPet::onFaceDown(bool faceDown) {
    if (faceDown && !_faceDown) {
        _faceDownTs = millis();
    }
    _faceDown = faceDown;
}

void GotchiPet::onSoundEvent() {
    _setTempMood(Mood::STARTLED, 2000);
}

void GotchiPet::setContext(uint8_t hour, int8_t tempC) {
    _hour  = hour;
    _tempC = tempC;
}

void GotchiPet::setPhoneBatteryLow(bool low) {
    _phoneBatLow = low;
}

// ── Internal ─────────────────────────────────────────────────────────────────

void GotchiPet::_recalcMood() {
    // Temporary mood takes priority
    if (_tempMood != Mood::NEUTRAL) {
        if (_mood != _tempMood) {
            _mood        = _tempMood;
            _moodChanged = true;
        }
        return;
    }

    Mood next = Mood::NEUTRAL;

    // Priority order (highest first):  SCARED > ANGRY > SAD > SICK > SLEEPING > PENSIVE > HAPPY
    if (_phoneBatLow) {
        next = Mood::SCARED;
    } else if (_stats.hunger == 0 || _stats.thirst == 0) {
        next = Mood::SICK;
    } else if (_stats.hunger < 20 || _stats.thirst < 20) {
        next = Mood::SAD;
    } else if (_stats.energy < 15) {
        // Night hours trigger sleep
        bool nightTime = (_hour >= 22 || _hour < 6);
        next = nightTime ? Mood::SLEEPING : Mood::PENSIVE;
    } else if (_stats.hunger > 70 && _stats.thirst > 70 && _stats.energy > 70) {
        next = Mood::HAPPY;
    } else {
        next = Mood::NEUTRAL;
    }

    if (_mood != next) {
        _mood        = next;
        _moodChanged = true;
    }
}

void GotchiPet::_setTempMood(Mood m, uint32_t durationMs) {
    _tempMood   = m;
    _moodExpiry = millis() + durationMs;
    if (_mood != m) {
        _mood        = m;
        _moodChanged = true;
    }
}
