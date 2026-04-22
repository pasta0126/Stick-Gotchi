#include "GotchiPet.h"
#include <Arduino.h>
#include <Preferences.h>

void GotchiPet::begin() {
    _stats.hunger = 80;
    _stats.energy = 80;
    _stats.health = 100;
    load();
    _mood        = Mood::HAPPY;
    _moodChanged = true;
}

void GotchiPet::save() {
    Preferences prefs;
    prefs.begin("gotchi", false);
    prefs.putUChar("h",  _stats.hunger);
    prefs.putUChar("e",  _stats.energy);
    prefs.putUChar("hp", _stats.health);
    prefs.end();
}

void GotchiPet::load() {
    Preferences prefs;
    prefs.begin("gotchi", true);
    if (!prefs.isKey("h")) { prefs.end(); return; }
    _stats.hunger = prefs.getUChar("h",  _stats.hunger);
    _stats.energy = prefs.getUChar("e",  _stats.energy);
    _stats.health = prefs.getUChar("hp", _stats.health);
    prefs.end();
}

// ── Main tick ─────────────────────────────────────────────────────────────────

void GotchiPet::tick(uint32_t deltaMs) {
    if (_dead) return;

    uint32_t now = millis();

    if (_tempMood != Mood::NEUTRAL && now >= _moodExpiry)
        _tempMood = Mood::NEUTRAL;

    _updateSleep();

    _decayAccum += deltaMs;
    if (_decayAccum >= DECAY_INTERVAL_MS) {
        _decayAccum -= DECAY_INTERVAL_MS;

        if (!_sleeping) {
            _stats.hunger = (_stats.hunger > DECAY_HUNGER) ? _stats.hunger - DECAY_HUNGER : 0;
            _stats.energy = (_stats.energy > DECAY_ENERGY) ? _stats.energy - DECAY_ENERGY : 0;
        } else {
            _stats.energy = min(100, (int)_stats.energy + 3);
        }
    }

    _updateHealth(deltaMs);
    _recalcMood();

    _saveAccum += deltaMs;
    if (_saveAccum >= SAVE_INTERVAL_MS) {
        _saveAccum -= SAVE_INTERVAL_MS;
        save();
    }
}

// ── Actions ───────────────────────────────────────────────────────────────────

void GotchiPet::feed() {
    if (_dead) return;
    _stats.hunger = min(100, (int)_stats.hunger + 25);
    _setTempMood(Mood::HAPPY, 3000);
}

void GotchiPet::play() {
    if (_dead || _sleeping) return;
    _stats.energy = (_stats.energy > 10) ? _stats.energy - 10 : 0;
    _setTempMood(Mood::EXCITED, 5000);
}

void GotchiPet::pet() {
    if (_dead) return;
    uint32_t now = millis();
    if (_petClicks == 0) _petClicksTs = now;
    _petClicks++;

    if (_petClicks >= 5 && (now - _petClicksTs) <= 2000) {
        _setTempMood(Mood::ANGRY, 6000);
        _petClicks = 0;
        return;
    }
    if (_petClicks >= 3 && (now - _petClicksTs) <= 8000) {
        _setTempMood(Mood::LAUGHING, 4000);
        _petClicks = 0;
        return;
    }
    if ((now - _petClicksTs) > 8000) _petClicks = 1;
}

// ── Sensor inputs ─────────────────────────────────────────────────────────────

void GotchiPet::onShake(uint8_t intensity) {
    if (_dead) return;
    switch (intensity) {
    case 0: // soft
        if (_sleeping) {
            _sleeping = false;
            _setTempMood(Mood::HAPPY, 3000);
        } else {
            _setTempMood(Mood::EXCITED, 2000);
        }
        break;
    case 1: // medium
        _setTempMood(Mood::DIZZY, 4000);
        break;
    case 2: // hard
        _setTempMood(Mood::SCARED, 4000);
        if (_stats.health > 2) _stats.health -= 2;
        break;
    case 3: // violent
        _setTempMood(Mood::SCARED, 6000);
        if (_stats.health > 5) _stats.health -= 5;
        break;
    }
}

void GotchiPet::onNoiseLevel(uint8_t db) {
    if (_dead) return;
    if (db > 85) {
        _setTempMood(Mood::STARTLED, 2000);
    } else if (db > 70) {
        if (_sleeping) {
            _sleeping = false;
        }
        // Sustained noise handled in tick via accumulated bad mood
        if (_tempMood == Mood::NEUTRAL && _mood != Mood::ANNOYED) {
            _setTempMood(Mood::ANNOYED, 5000);
        }
    }
}

void GotchiPet::setHour(uint8_t hour) {
    _hour = hour;
}

// ── Internal ──────────────────────────────────────────────────────────────────

void GotchiPet::_updateSleep() {
    bool nightTime = (_hour >= 22 || _hour < 7);
    if (nightTime && !_sleeping) {
        _sleeping = true;
        _setTempMood(Mood::SLEEPING, 0); // persistent until woken
    }
    if (!nightTime && _sleeping && _stats.energy > 30) {
        _sleeping = false;
    }
    // Low energy during day → sleep
    if (!nightTime && !_sleeping && _stats.energy < 15) {
        _sleeping = true;
    }
}

void GotchiPet::_updateHealth(uint32_t deltaMs) {
    // Neglect = any stat below 20 for extended time
    bool neglected = (_stats.hunger < 20 || _stats.energy < 10);
    bool nightProtection = (_hour >= 22 || _hour < 7);

    if (neglected && !nightProtection) {
        if (_stats.health > 0) {
            // Slow health drain — 1 pt per minute of neglect
            static uint32_t healthDecayAccum = 0;
            healthDecayAccum += deltaMs;
            if (healthDecayAccum >= 60000) {
                healthDecayAccum -= 60000;
                _stats.health--;
            }
        }
    }

    if (_stats.health == 0) {
        _lowHealthMs += deltaMs;
        if (_lowHealthMs >= DEATH_DELAY_MS) {
            _dead = true;
            save();
        }
    } else {
        _lowHealthMs = 0;
        // Slow health recovery when well-fed and rested
        if (_stats.hunger > 70 && _stats.energy > 50 && _stats.health < 100) {
            static uint32_t healthRecoverAccum = 0;
            healthRecoverAccum += deltaMs;
            if (healthRecoverAccum >= 120000) { // 1pt per 2 min
                healthRecoverAccum -= 120000;
                _stats.health++;
            }
        }
    }
}

void GotchiPet::_recalcMood() {
    if (_tempMood != Mood::NEUTRAL) {
        if (_mood != _tempMood) { _mood = _tempMood; _moodChanged = true; }
        return;
    }

    Mood next = Mood::NEUTRAL;

    if (_sleeping)              next = Mood::SLEEPING;
    else if (_stats.health < 20) next = Mood::SICK;
    else if (_stats.hunger < 20) next = Mood::SAD;
    else if (_stats.energy < 20) next = Mood::PENSIVE;
    else if (_stats.hunger > 70 && _stats.energy > 70 && _stats.health > 70)
                                next = Mood::HAPPY;

    if (_mood != next) { _mood = next; _moodChanged = true; }
}

void GotchiPet::_setTempMood(Mood m, uint32_t durationMs) {
    _tempMood   = m;
    _moodExpiry = durationMs > 0 ? millis() + durationMs : UINT32_MAX;
    if (_mood != m) { _mood = m; _moodChanged = true; }
}
