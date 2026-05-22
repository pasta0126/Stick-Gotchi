#pragma once
#include <stdint.h>
#include "GotchiMoods.h"

// ── Events ─────────────────────────────────────────────────────────────────────

enum class BehaviourEvent : uint8_t {
    IMU_TAP,
    IMU_SHAKE,
    IMU_PICKUP,
    IMU_PUTDOWN,
    IMU_WALKING,
    MIC_NOISE_SOFT,
    MIC_NOISE_LOUD,
    BTN_A,
    BTN_B,
    IDLE_SHORT,
    IDLE_LONG,
    IDLE_SLEEP,
    ATTENTION_TIMER,
    _COUNT
};

// ── States ─────────────────────────────────────────────────────────────────────

enum class CreatureState : uint8_t {
    IDLE,
    REACTING,
    SEEKING,
    PLAYING,
    SLEEPING,
    STARTLED,
    _COUNT
};

// ── Creature types ─────────────────────────────────────────────────────────────

enum class CreatureType : uint8_t {
    BYTEE        = 0,  // magic steampunk robot — curious, studious, surprised
    CTHULHU      = 1,  // baby eldritch — creepy-cute, wants hugs, chaotic
    JACK         = 2,  // stone with hat — stoic deadpan, rarely reacts
    LUMI         = 3,  // deer-fox fae — vivid, anxious, most reactive
    _COUNT
};

// ── Reaction descriptor ────────────────────────────────────────────────────────

static constexpr uint8_t NO_ANIM = 0xFF;

struct Reaction {
    GotchiMood    moodTarget;
    uint8_t       animationId;  // NO_ANIM = skip
    CreatureState nextState;
    uint16_t      cooldownMs;
};

// ── Behaviour engine ───────────────────────────────────────────────────────────

class CreatureRegistry;

class GotchiBehaviour {
public:
    void begin(CreatureType type, const CreatureRegistry& registry);
    void setCreature(CreatureType type, const CreatureRegistry& registry);

    void pushEvent(BehaviourEvent evt);
    void update(uint32_t deltaMs);

    CreatureState state()     const { return _state; }
    GotchiMood    mood()      const { return _mood; }
    uint8_t       animId()    const { return _currentAnim; }
    bool          animDirty()       { bool d = _animChanged; _animChanged = false; return d; }

private:
    void _applyReaction(const Reaction& r);
    void _tickIdle(uint32_t deltaMs);
    void _tickAttention(uint32_t deltaMs);
    void _tickReacting(uint32_t deltaMs);

    CreatureState _state       = CreatureState::IDLE;
    GotchiMood    _mood        = GotchiMood::NEUTRAL;
    uint8_t       _currentAnim = 0;
    bool          _animChanged = false;

    uint32_t _idleMs      = 0;
    uint32_t _attentionMs = 0;
    uint32_t _reactingMs  = 0;
    uint32_t _cooldowns[(uint8_t)BehaviourEvent::_COUNT] = {};

    const CreatureRegistry* _registry = nullptr;
    CreatureType             _type    = CreatureType::FANTASY;
};
