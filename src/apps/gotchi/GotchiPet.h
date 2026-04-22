#pragma once
#include <stdint.h>
#include "../../gotchi/GotchiDNA.h"
#include "../../gotchi/GotchiLineage.h"

enum class Mood : uint8_t {
    NEUTRAL   = 0,  // Arduino.h defines DEFAULT as a macro — do not rename
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
    uint8_t hunger;   // 0 (starving) → 100 (full)
    uint8_t energy;   // 0 (exhausted) → 100 (rested)
    uint8_t health;   // 0 (dying) → 100 (healthy) — degrades from neglect
};

class GotchiPet {
public:
    void begin();
    void tick(uint32_t deltaMs);

    // User actions
    void feed();
    void play();
    void pet();

    // Sensor inputs
    void onShake(uint8_t intensity);   // 0=soft 1=medium 2=hard 3=violent
    void onNoiseLevel(uint8_t db);     // ambient dB reading from mic task
    void setHour(uint8_t hour);        // RTC hour — drives sleep schedule

    // Readers
    Mood      mood()        const { return _mood; }
    PetStats  stats()       const { return _stats; }
    bool      moodChanged() const { return _moodChanged; }
    void      clearMoodChanged()  { _moodChanged = false; }
    bool      isTempMood()  const { return _tempMood != Mood::NEUTRAL; }
    bool      isSleeping()  const { return _sleeping; }
    bool      isDead()      const { return _dead; }
    bool      isNewEgg()    const { return _newEggReady; }
    GotchiID  currentID()   const { return _id; }

    // NVS persistence
    void save();
    void load();

private:
    PetStats _stats{};
    Mood     _mood        = Mood::NEUTRAL;
    bool     _moodChanged = false;

    Mood     _tempMood    = Mood::NEUTRAL;
    uint32_t _moodExpiry  = 0;

    bool     _sleeping    = false;
    bool     _dead        = false;
    uint32_t _lowHealthMs = 0;   // time spent at health=0 (death timer)

    uint8_t  _hour        = 12;

    uint32_t _decayAccum  = 0;
    uint32_t _saveAccum   = 0;

    uint32_t _petClicksTs = 0;
    int      _petClicks   = 0;

    GotchiID        _id{};
    GotchiAncestor  _ancestors[5]{};
    GotchiHeritage  _heritage{};
    GotchiLineage   _lineage;
    bool            _newEggReady = false;

    static constexpr uint32_t DECAY_INTERVAL_MS  = 10000;   // 10s per decay tick
    static constexpr uint32_t SAVE_INTERVAL_MS   = 300000;  // 5 min
    static constexpr uint32_t DEATH_DELAY_MS      = 900000; // 15 min at health=0
    static constexpr uint8_t  DECAY_HUNGER        = 1;
    static constexpr uint8_t  DECAY_ENERGY        = 1;

    void _recalcMood();
    void _setTempMood(Mood m, uint32_t durationMs);
    void _updateSleep();
    void _updateHealth(uint32_t deltaMs);
    void _handleDeath();
};
