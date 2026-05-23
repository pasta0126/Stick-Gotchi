#include "GotchiPet.h"
#include <Arduino.h>
#include <Preferences.h>
#include "../../gotchi/CreatureRegistry.h"

void GotchiPet::begin() {
    load();

    // First-boot seed
    if (_id.visual_seed == 0) {
        _id.visual_seed = (uint16_t)(millis() ^ 0x1337u);
        _id.generation  = 0;
    }

    _behaviour.begin(_creatureType, CreatureRegistry::instance());
    _moodChangedFlag = true;
}

void GotchiPet::tick(uint32_t deltaMs) {
    _behaviour.update(deltaMs);

    if (_behaviour.moodDirty()) _moodChangedFlag = true;

    _totalCompanionMs += deltaMs;

    if (_micSoftCooldown > 0) _micSoftCooldown = (_micSoftCooldown > deltaMs) ? _micSoftCooldown - deltaMs : 0;
    if (_micLoudCooldown > 0) _micLoudCooldown = (_micLoudCooldown > deltaMs) ? _micLoudCooldown - deltaMs : 0;

    _saveAccum += deltaMs;
    if (_saveAccum >= SAVE_INTERVAL_MS) {
        _saveAccum -= SAVE_INTERVAL_MS;
        save();
    }
}

void GotchiPet::onNoiseLevel(float rms) {
    if (rms >= MIC_LOUD_RMS && _micLoudCooldown == 0) {
        _behaviour.pushEvent(BehaviourEvent::MIC_NOISE_LOUD);
        _micLoudCooldown = MIC_COOLDOWN_MS;
        _micSoftCooldown = MIC_COOLDOWN_MS;
    } else if (rms >= MIC_SOFT_RMS && _micSoftCooldown == 0) {
        _behaviour.pushEvent(BehaviourEvent::MIC_NOISE_SOFT);
        _micSoftCooldown = MIC_COOLDOWN_MS;
    }
}

void GotchiPet::setCreature(CreatureType type) {
    _creatureType = type;
    _behaviour.setCreature(type, CreatureRegistry::instance());
}

void GotchiPet::save() {
    Preferences prefs;
    prefs.begin("gotchi2", false);
    prefs.putUChar("ct",  (uint8_t)_creatureType);
    prefs.putUInt("tc",   _totalCompanionMs);
    prefs.putUInt("it",   _interactionsToday);
    prefs.putUShort("vs", _id.visual_seed);
    prefs.putUChar("gn",  _id.generation);
    prefs.end();
}

void GotchiPet::load() {
    Preferences prefs;
    prefs.begin("gotchi2", true);
    if (!prefs.isKey("ct")) { prefs.end(); return; }
    _creatureType      = (CreatureType)prefs.getUChar("ct", (uint8_t)CreatureType::BYTEE);
    _totalCompanionMs  = prefs.getUInt("tc", 0);
    _interactionsToday = prefs.getUInt("it", 0);
    _id.visual_seed    = prefs.getUShort("vs", 0);
    _id.generation     = prefs.getUChar("gn", 0);
    prefs.end();
}
