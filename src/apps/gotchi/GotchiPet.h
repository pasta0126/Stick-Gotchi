#pragma once
#include <stdint.h>
#include "../../gotchi/GotchiBehaviour.h"
#include "../../gotchi/CreatureRegistry.h"
#include "../../gotchi/GotchiDNA.h"    // GotchiID — renderer uses visual_seed for palette
#include "../../gotchi/GotchiType.h"   // GotchiType — renderer compat until SGOTCHI-72

// Mood alias — renderer uses Mood::XYZ; GotchiMood has identical values.
using Mood = GotchiMood;

// ── Legacy enums (renderer compat until SGOTCHI-72) ────────────────────────────
enum class AdultForm : uint8_t { HEALTHY = 0, NORMAL = 1, NEGLECTED = 2 };
enum class LifeStage : uint8_t { EGG = 0, BABY = 1, YOUNG = 2, ADULT = 3 };

struct PetStats {
    uint8_t hunger  = 50;
    uint8_t energy  = 50;
    uint8_t health  = 50;
};

// ── GotchiPet ──────────────────────────────────────────────────────────────────
// Companion model. Internally runs GotchiBehaviour; exposes sensor inputs and
// button events. Legacy stat/lifecycle getters are stubs until the renderer is
// redesigned (SGOTCHI-72).

class GotchiPet {
public:
    void begin();
    void tick(uint32_t deltaMs);

    // ── Sensor inputs ─────────────────────────────────────────────────
    void onNoiseLevel(float rms);      // raw RMS from mic buffer

    // IMU events — called directly from GotchiApp motion detector
    void onImuTap()     { _behaviour.pushEvent(BehaviourEvent::IMU_TAP);     }
    void onImuShake()   { _behaviour.pushEvent(BehaviourEvent::IMU_SHAKE);   }
    void onImuPickup()  { _behaviour.pushEvent(BehaviourEvent::IMU_PICKUP);  }
    void onImuPutdown() { _behaviour.pushEvent(BehaviourEvent::IMU_PUTDOWN); }
    void onImuWalking() { _behaviour.pushEvent(BehaviourEvent::IMU_WALKING); }

    // ── Button events ─────────────────────────────────────────────────
    void onBtnA() { _behaviour.pushEvent(BehaviourEvent::BTN_A); }
    void onBtnB() { _behaviour.pushEvent(BehaviourEvent::BTN_B); }

    // ── Creature selection ────────────────────────────────────────────
    void         setCreature(CreatureType type);
    CreatureType creature()            const { return _creatureType; }

    // ── Live state ────────────────────────────────────────────────────
    GotchiMood    mood()               const { return _behaviour.mood(); }
    bool          moodChanged()        const { return _moodChangedFlag; }
    void          clearMoodChanged()         { _moodChangedFlag = false; }
    bool          moodDirty()                { return _behaviour.moodDirty(); }
    uint8_t       animId()             const { return _behaviour.animId(); }
    bool          animDirty()                { return _behaviour.animDirty(); }
    CreatureState behaviourState()     const { return _behaviour.state(); }

    // ── Companion lifetime stats ───────────────────────────────────────
    uint32_t totalCompanionMs()        const { return _totalCompanionMs; }
    uint32_t interactionsToday()       const { return _interactionsToday; }

    // ── NVS persistence ───────────────────────────────────────────────
    void save();
    void load();

    // ── Legacy compat stubs (renderer uses these until SGOTCHI-72) ────
    LifeStage   stage()                const { return LifeStage::ADULT; }
    AdultForm   adultForm()            const { return AdultForm::NORMAL; }
    GotchiType  gotchiType()           const {
        switch (_creatureType) {
        case CreatureType::BYTEE:   return GotchiType::CYBER;
        case CreatureType::CTHULHU: return GotchiType::SOUL;
        case CreatureType::JACK:    return GotchiType::CRYSTAL;
        case CreatureType::LUMI:    return GotchiType::ENERGY;
        default:                    return GotchiType::ORGANIC;
        }
    }
    PetStats    stats()                const { return {}; }
    uint8_t     dirtyness()            const { return 0; }
    uint8_t     moodScore()            const { return 65; }
    uint8_t     avgHealthPct()         const { return 100; }
    uint8_t     neglectCount()         const { return 0; }
    bool        isSleeping()           const { return _behaviour.state() == CreatureState::SLEEPING; }
    bool        isTempMood()           const { return false; }
    bool        isSick()               const { return false; }
    bool        isAgony()              const { return false; }
    bool        isDead()               const { return false; }
    bool        isNewEgg()             const { return false; }
    bool        isEggHatched()         const { return false; }
    uint16_t    lastDaysLived()        const { return 0; }
    uint8_t     lastDeathCause()       const { return 0; }
    GotchiID    currentID()            const { return _id; }
    void        restartEgg()                 {}
    void        acceptInheritedEgg()         {}

private:
    GotchiBehaviour  _behaviour;
    CreatureType     _creatureType    = CreatureType::BYTEE;
    bool             _moodChangedFlag = false;

    GotchiID         _id{};   // visual seed kept for renderer palette generation

    uint32_t _totalCompanionMs  = 0;
    uint32_t _interactionsToday = 0;
    uint32_t _saveAccum         = 0;

    uint32_t _micSoftCooldown = 0;
    uint32_t _micLoudCooldown = 0;

    static constexpr uint32_t SAVE_INTERVAL_MS = 300000;   // 5 min
    static constexpr float    MIC_SOFT_RMS     = 300.0f;
    static constexpr float    MIC_LOUD_RMS     = 2000.0f;
    static constexpr uint32_t MIC_COOLDOWN_MS  = 2000;
};
