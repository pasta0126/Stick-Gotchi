#include "GotchiPet.h"
#include <Arduino.h>
#include <Preferences.h>

// Per-stage decay rates applied every DECAY_INTERVAL_MS (10 s).
// Indexed by LifeStage (EGG=0, BABY=1, YOUNG=2, ADULT=3).
namespace {
struct StageDecay {
    uint8_t hungerAwake;   // hunger  -X when awake
    uint8_t hungerSleep;   // hunger  -X while sleeping  (0 = no drain)
    uint8_t energyDrain;   // energy  -X when awake
    uint8_t energyGain;    // energy  +X while sleeping
    uint8_t dirtyAwake;    // dirty   +X when awake
    uint8_t dirtySleep;    // dirty   +X while sleeping
};
constexpr StageDecay DECAY_TABLE[4] = {
    //  hA  hS  eD  eR  dA  dS
    {    0,  0,  0,  0,  0,  0 },  // EGG   — no decay, egg just incubates
    {    2,  0,  2,  5,  2,  1 },  // BABY  — vulnerable; fast drain, fast sleep recovery
    {    1,  0,  1,  3,  1,  1 },  // YOUNG — baseline rates
    {    1,  1,  1,  3,  1,  0 },  // ADULT — hunger drains even at night; no dirty during sleep
};
}

GotchiType GotchiPet::gotchiType() const {
    return _resolvedType;
}

void GotchiPet::begin() {
    _stats.hunger = 80;
    _stats.energy = 80;
    _stats.health = 100;
    _typeLoaded = false;
    load();
    _lineage.load(_id, _ancestors, _heritage);

    if (_id.generation == 0 && _id.visual_seed == 0) {
        uint16_t initialSeed = (uint16_t)millis();
        _id = createNewID(0, (const uint8_t[]){0, 0, 0}, initialSeed);
        _lineage.save(_id, _ancestors, _heritage);
    }

    if (!_typeLoaded) {
        GotchiVisual vis = decodeVisual(_id.visual_seed);
        _resolvedType = gotchiTypeFromSeed(vis.body_shape, vis.mark_type);
    }

    _mood        = Mood::HAPPY;
    _moodChanged = true;

    _stage = LifeStage::EGG;
    _stageAgeMs = 0;
    _feedCount = 0;
    _playCount = 0;
}

void GotchiPet::save() {
    Preferences prefs;
    prefs.begin("gotchi", false);
    prefs.putUChar("h",  _stats.hunger);
    prefs.putUChar("e",  _stats.energy);
    prefs.putUChar("hp", _stats.health);
    prefs.putUChar("st", (uint8_t)_stage);
    prefs.putUChar("ty", (uint8_t)_resolvedType);
    prefs.putUInt("sa", _stageAgeMs);
    prefs.putUInt("fc", _feedCount);
    prefs.putUInt("pc", _playCount);
    prefs.putUChar("di", _dirtyness);
    prefs.putBool("li", _lightOn);
    prefs.putUChar("ms", _moodScore);
    prefs.putUChar("ah", _avgHealthPct);
    prefs.putUChar("ne", _neglectCount);
    prefs.end();
}

void GotchiPet::load() {
    Preferences prefs;
    prefs.begin("gotchi", true);
    if (!prefs.isKey("h")) { prefs.end(); return; }
    _stats.hunger = prefs.getUChar("h",  _stats.hunger);
    _stats.energy = prefs.getUChar("e",  _stats.energy);
    _stats.health = prefs.getUChar("hp", _stats.health);
    _stage = (LifeStage)prefs.getUChar("st", (uint8_t)LifeStage::EGG);
    if (prefs.isKey("ty")) {
        _resolvedType = (GotchiType)prefs.getUChar("ty", 0);
        _typeLoaded = true;
    }
    _stageAgeMs = prefs.getUInt("sa", 0);
    _feedCount  = prefs.getUInt("fc", 0);
    _playCount  = prefs.getUInt("pc", 0);
    _dirtyness     = prefs.getUChar("di", 0);
    _lightOn       = prefs.getBool("li", true);
    _moodScore     = prefs.getUChar("ms", 65);
    _avgHealthPct  = prefs.getUChar("ah", 100);
    _neglectCount  = prefs.getUChar("ne", 0);
    prefs.end();
}

// ── Main tick ─────────────────────────────────────────────────────────────────

void GotchiPet::tick(uint32_t deltaMs) {
    if (_dead) return;

    uint32_t now = millis();

    if (_tempMood != Mood::NEUTRAL && now >= _moodExpiry)
        _tempMood = Mood::NEUTRAL;

    _updateSleep();

    _stageAgeMs += deltaMs;

    bool stageChanged = false;
    if (_stage == LifeStage::EGG && _stageAgeMs >= STAGE_EGG_MS) {
        if (!_eggHatched) _eggHatched = true;
        _stageAgeMs = STAGE_EGG_MS;  // clamp — wait for restart
    }
    else if (_stage == LifeStage::BABY && _stageAgeMs >= STAGE_BABY_MS) {
        _stage = LifeStage::YOUNG;
        _stageAgeMs = 0;
        _selectEvolvedType();
        stageChanged = true;
    }
    else if (_stage == LifeStage::YOUNG && _stageAgeMs >= STAGE_YOUNG_MS) {
        _stage = LifeStage::ADULT;
        _stageAgeMs = 0;
        stageChanged = true;
    }

    _decayAccum += deltaMs;
    if (_decayAccum >= DECAY_INTERVAL_MS) {
        _decayAccum -= DECAY_INTERVAL_MS;

        const StageDecay& d = DECAY_TABLE[static_cast<uint8_t>(_stage)];
        uint8_t hD = _sleeping ? d.hungerSleep : d.hungerAwake;
        if (hD > 0)
            _stats.hunger = (_stats.hunger > hD) ? _stats.hunger - hD : 0;
        if (!_sleeping && d.energyDrain > 0)
            _stats.energy = (_stats.energy > d.energyDrain) ? _stats.energy - d.energyDrain : 0;
        if (_sleeping && d.energyGain > 0)
            _stats.energy = (uint8_t)min(100, (int)_stats.energy + d.energyGain);
        uint8_t dD = _sleeping ? d.dirtySleep : d.dirtyAwake;
        if (dD > 0 && _dirtyness < 100)
            _dirtyness = (uint8_t)min(100, (int)_dirtyness + dD);
        _updateCareHistory();
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
    _feedCount++;
    _setTempMood(Mood::HAPPY, 3000);
}

void GotchiPet::play() {
    if (_dead || _sleeping) return;
    _stats.energy = (_stats.energy > 10) ? _stats.energy - 10 : 0;
    _playCount++;
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

AdultForm GotchiPet::adultForm() const {
    uint16_t avg = ((uint16_t)_stats.hunger + _stats.energy + _stats.health) / 3;
    if (avg >= 70) return AdultForm::HEALTHY;
    if (avg >= 40) return AdultForm::NORMAL;
    return AdultForm::NEGLECTED;
}

void GotchiPet::medicine() {
    if (_dead) return;
    if (_stats.health < 30) {
        _stats.health = min(100, (int)_stats.health + 25);
        _setTempMood(Mood::HAPPY, 4000);
    } else {
        _setTempMood(Mood::ANNOYED, 3000);
    }
}

void GotchiPet::toggleLight() {
    if (_dead) return;
    _lightOn = !_lightOn;
    if (_lightOn && _sleeping && !(_hour >= 22 || _hour < 7)) {
        _sleeping = false;
        _setTempMood(Mood::STARTLED, 2000);
    }
}

void GotchiPet::clean() {
    if (_dead) return;
    _dirtyness = 0;
    _setTempMood(Mood::HAPPY, 3000);
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
    if (nightTime && !_lightOn && !_sleeping) {
        _sleeping = true;
        _setTempMood(Mood::SLEEPING, 0);
    }
    if (nightTime && _lightOn && _sleeping && _stats.energy > 20) {
        _sleeping = false; // light kept on — gotchi wakes
    }
    if (!nightTime && _sleeping && _stats.energy > 30) {
        _sleeping = false;
    }
    // Low energy during day → nap; light off lowers nap threshold
    uint8_t napThreshold = _lightOn ? 15 : 30;
    if (!nightTime && !_sleeping && _stats.energy < napThreshold) {
        _sleeping = true;
    }
}

void GotchiPet::_updateHealth(uint32_t deltaMs) {
    // Neglect = any stat below 20 for extended time
    bool neglected = (_stats.hunger < 20 || _stats.energy < 10 || _dirtyness > 85);
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
            _handleDeath();
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

    if (_sleeping)               next = Mood::SLEEPING;
    else if (_stats.health < 20) next = Mood::SICK;
    else if (_stats.hunger < 20) next = Mood::SAD;
    else if (_stats.energy < 20) next = Mood::PENSIVE;
    else if (_dirtyness > 70)    next = Mood::PENSIVE;
    else if (_stats.hunger > 70 && _stats.energy > 70 && _stats.health > 70)
                                 next = Mood::HAPPY;

    if (_mood != next) { _mood = next; _moodChanged = true; }
}

void GotchiPet::_setTempMood(Mood m, uint32_t durationMs) {
    _tempMood   = m;
    _moodExpiry = durationMs > 0 ? millis() + durationMs : UINT32_MAX;
    if (_mood != m) { _mood = m; _moodChanged = true; }
}

void GotchiPet::restartEgg() {
    uint16_t newSeed = (uint16_t)(millis() ^ (millis() >> 5) ^ random(0xFFFF));
    const uint8_t noParent[3] = {0, 0, 0};
    _id          = createNewID(0, noParent, newSeed);
    _stage       = LifeStage::EGG;
    _stageAgeMs  = 0;
    _eggHatched  = false;
    _feedCount   = 0;
    _playCount   = 0;
    GotchiVisual vis = decodeVisual(newSeed);
    _resolvedType  = gotchiTypeFromSeed(vis.body_shape, vis.mark_type);
    _moodScore     = 65;
    _avgHealthPct  = 100;
    _neglectCount  = 0;
    save();
}

void GotchiPet::_selectEvolvedType() {
    uint32_t total = _feedCount + _playCount;
    if (total == 0) {
        GotchiVisual vis = decodeVisual(_id.visual_seed);
        _resolvedType = gotchiTypeFromSeed(vis.body_shape, vis.mark_type);
        return;
    }

    uint8_t feedPct = (uint8_t)((_feedCount * 100) / total);
    uint8_t carePct = (uint8_t)(((uint16_t)_stats.hunger + _stats.energy + _stats.health) / 3);

    if (feedPct > 66) {
        _resolvedType = (carePct >= 60) ? GotchiType::ELEMENTAL : GotchiType::CRYSTAL;
    } else if (feedPct < 34) {
        _resolvedType = (carePct >= 60) ? GotchiType::ENERGY : GotchiType::SOUL;
    } else {
        _resolvedType = (carePct >= 60) ? GotchiType::ORGANIC : GotchiType::CYBER;
    }
}

void GotchiPet::_updateCareHistory() {
    uint8_t moodPts;
    switch (_mood) {
    case Mood::HAPPY:    case Mood::EXCITED: case Mood::LAUGHING: moodPts = 100; break;
    case Mood::SLEEPING:                                           moodPts = 75;  break;
    case Mood::NEUTRAL:                                            moodPts = 65;  break;
    case Mood::PENSIVE:                                            moodPts = 45;  break;
    case Mood::SAD:      case Mood::ANNOYED:                       moodPts = 30;  break;
    default:                                                       moodPts = 10;  break;
    }

    _moodScore    = (uint8_t)((_moodScore    * 15u + moodPts)       / 16u);
    _avgHealthPct = (uint8_t)((_avgHealthPct * 15u + _stats.health) / 16u);

    bool critical = (_stats.hunger < 20 || _stats.energy < 10 || _stats.health < 20);
    if (critical && _neglectCount < 255) _neglectCount++;
}

void GotchiPet::_handleDeath() {
    _dead = true;

    GotchiAncestor dying{};
    dying.id = _id;
    dying.days_lived = (uint16_t)(millis() / 86400000UL);
    dying.cause_of_death = (_stats.hunger < 10) ? 0 : 1;
    dying.avg_mood_pct   = _moodScore;
    dying.avg_health_pct = _avgHealthPct;

    GotchiHeritage newHeritage = GotchiLineage::computeHeritage(dying, _ancestors);
    _lineage.shiftAncestors(_ancestors, dying);

    uint32_t rng = millis() ^ ((uint32_t)_id.visual_seed << 16);
    uint16_t newSeed = mutateSeed(_id.visual_seed, rng);
    uint8_t newGen = _id.generation + 1;
    GotchiID newID = createNewID(newGen, _id.parent_id, newSeed);

    _id = newID;
    _heritage = newHeritage;
    _lineage.save(_id, _ancestors, _heritage);
    _newEggReady = true;

    save();
}
