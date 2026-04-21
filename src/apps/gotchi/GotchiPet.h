#pragma once
#include <stdint.h>

// Moods match the Atom-Gotchi BLE protocol (byte 0 of state payload).
enum class Mood : uint8_t {
    NEUTRAL   = 0,  // renamed: Arduino.h defines DEFAULT as a macro
    HAPPY     = 1,
    SICK      = 2,
    PENSIVE   = 3,
    SAD       = 4,
    SLEEPING  = 5,
    EXCITED   = 6,
    LAUGHING  = 7,
    DIZZY     = 8,
    ANNOYED   = 9,
    ANGRY     = 10,
    STARTLED  = 11,
    SCARED    = 12,
};

struct PetStats {
    uint8_t  hunger;  // 0 (starving) → 100 (full)
    uint8_t  thirst;  // 0 (parched)  → 100 (hydrated)
    uint8_t  energy;  // 0 (exhausted)→ 100 (rested)
    uint16_t steps;
};

// All pet state and logic — no display or hardware dependencies.
// Updated from the main loop thread via tick().
class GotchiPet {
public:
    void begin();

    // Advance simulation by deltaMs milliseconds.
    void tick(uint32_t deltaMs);

    // Commands from BLE / buttons
    void feed();
    void drink();
    void pet();
    void play();

    // Sensor inputs (called by GotchiApp after reading IMU)
    void onStep();
    void onShake();
    void onFaceDown(bool faceDown);
    void onSoundEvent();
    void setContext(uint8_t hour, int8_t tempC);
    void setPhoneBatteryLow(bool low);

    // Readers
    Mood      mood()         const { return _mood; }
    PetStats  stats()        const { return _stats; }
    bool      moodChanged()  const { return _moodChanged; }
    void      clearMoodChanged()   { _moodChanged = false; }
    bool      phoneBatLow()  const { return _phoneBatLow; }

    // NVS persistence
    void save();
    void load();

private:
    PetStats _stats{};
    Mood     _mood        = Mood::NEUTRAL;
    bool     _moodChanged = false;

    // Temporary mood override (expires after _moodExpiry ms)
    Mood     _tempMood    = Mood::NEUTRAL;
    uint32_t _moodExpiry  = 0;

    // Internal counters
    uint32_t _decayAccum   = 0;    // accumulates ms for decay tick
    uint32_t _petClicksTs  = 0;    // timestamp of first pet click
    int      _petClicks    = 0;
    bool     _faceDown     = false;
    uint32_t _faceDownTs   = 0;
    bool     _phoneBatLow  = false;
    uint8_t  _hour         = 12;
    int8_t   _tempC        = 20;

    uint32_t _saveAccum    = 0;

    static constexpr uint32_t DECAY_INTERVAL_MS = 10000; // 10 s per decay tick
    static constexpr uint32_t SAVE_INTERVAL_MS  = 300000; // save every 5 min
    static constexpr uint8_t  DECAY_HUNGER      = 1;
    static constexpr uint8_t  DECAY_THIRST      = 2;
    static constexpr uint8_t  DECAY_ENERGY      = 1;

    void _recalcMood();
    void _setTempMood(Mood m, uint32_t durationMs);
};
