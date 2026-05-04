#pragma once
#include <stdint.h>
#include "../../gotchi/GotchiDNA.h"
#include "../../gotchi/GotchiLineage.h"
#include "../../gotchi/GotchiType.h"

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


enum class AdultForm : uint8_t {
    HEALTHY   = 0,  // avg stats >= 70 — vivid, saturated palette
    NORMAL    = 1,  // avg stats 40-69 — standard palette
    NEGLECTED = 2,  // avg stats <  40 — desaturated, dark palette
};

enum class LifeStage : uint8_t {
    EGG   = 0,
    BABY  = 1,
    YOUNG = 2,
    ADULT = 3,
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
    void medicine();     // heals if sick; annoyed if healthy
    void toggleLight();  // ON = bright (prevents night sleep); OFF = dark (allows naps)
    void clean();        // resets dirtyness accumulator

    // Sensor inputs
    void onShake(uint8_t intensity);   // 0=soft 1=medium 2=hard 3=violent
    void onNoiseLevel(uint8_t db);     // ambient dB reading from mic task
    void setHour(uint8_t hour);        // RTC hour — drives sleep schedule

    // Readers
    Mood      mood()        const { return _mood; }
    PetStats  stats()       const { return _stats; }
    bool      moodChanged() const { return _moodChanged; }
    void      clearMoodChanged()  { _moodChanged = false; }
    bool      isTempMood()   const { return _tempMood != Mood::NEUTRAL; }
    bool      isSleeping()   const { return _sleeping; }
    bool      isSick()       const { return _sick || _stats.health < 20; }
    bool      isAgony()      const { return !_dead && _stats.health == 0; }
    bool      isLightOn()    const { return _lightOn; }
    uint8_t   dirtyness()    const { return _dirtyness; }
    uint8_t   moodScore()    const { return _moodScore; }
    uint8_t   avgHealthPct() const { return _avgHealthPct; }
    uint8_t   neglectCount() const { return _neglectCount; }
    bool      isDead()      const { return _dead; }
    bool      isNewEgg()    const { return _newEggReady; }
    bool      isEggHatched() const { return _eggHatched; }
    uint16_t  lastDaysLived()  const { return _lastDaysLived; }
    uint8_t   lastDeathCause() const { return _lastDeathCause; }
    void      restartEgg();
    void      acceptInheritedEgg();
    GotchiID  currentID()   const { return _id; }
    LifeStage    stage()     const { return _stage; }
    GotchiType   gotchiType() const;
    AdultForm    adultForm() const;   // meaningful only at ADULT stage

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
    bool     _sick        = false;
    bool     _dead        = false;
    bool     _lightOn     = true;
    uint8_t  _dirtyness   = 0;
    uint32_t _lowHealthMs    = 0;  // time spent at health=0 (death timer)
    uint32_t _dirtyHighMs    = 0;  // time at dirtyness >= 95; triggers sickness
    uint32_t _sickRecoverMs  = 0;  // time in good conditions while sick; natural recovery
    uint32_t _ageMs          = 0;  // real lived age (non-egg), persisted
    uint16_t _lastDaysLived  = 0;
    uint8_t  _lastDeathCause = 0;  // 0=hunger 1=sick 2=dirty 3=abuse 4=neglect

    uint8_t  _moodScore    = 65;   // EWMA of mood quality, 0-100
    uint8_t  _avgHealthPct = 100;  // EWMA of health, 0-100
    uint8_t  _neglectCount = 0;    // decay ticks where any stat was critically low

    uint8_t  _shakeStress  = 0;   // 0-100, accumulated shake trauma; decays over time
    uint8_t  _noiseAccum   = 0;   // 0-100, sustained noise exposure; decays in quiet
    uint32_t _quietMs      = 0;   // consecutive ms of calm environment (db < 70)
    uint8_t  _lastNoiseDb  = 0;   // most recent mic reading, sampled each decay tick

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
    bool            _eggHatched  = false;

    GotchiType   _resolvedType = GotchiType::ORGANIC;
    bool         _typeLoaded   = false;
    LifeStage    _stage       = LifeStage::EGG;
    uint32_t     _stageAgeMs  = 0;
    uint32_t     _feedCount   = 0;
    uint32_t     _playCount   = 0;

    static constexpr uint32_t DECAY_INTERVAL_MS = 10000;      // 10s per decay tick
    static constexpr uint32_t SAVE_INTERVAL_MS  = 300000;     // 5 min
    static constexpr uint32_t DEATH_DELAY_MS    = 900000;     // 15 min at health=0
    static constexpr uint32_t STAGE_EGG_MS      = 30000;      // 30s demo
    static constexpr uint32_t STAGE_BABY_MS     = 259200000;  // 72 h
    static constexpr uint32_t STAGE_YOUNG_MS    = 604800000;  // 168 h

    void _recalcMood();
    void _setTempMood(Mood m, uint32_t durationMs);
    void _updateSleep();
    void _updateHealth(uint32_t deltaMs);
    void _handleDeath();
    void _selectEvolvedType();
    void _updateCareHistory();
};
