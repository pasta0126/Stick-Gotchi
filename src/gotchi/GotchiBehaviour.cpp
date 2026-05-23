#include "GotchiBehaviour.h"
#include "CreatureRegistry.h"

// How many times longer than longIdleMs before IDLE_SLEEP fires.
static constexpr uint32_t SLEEP_IDLE_FACTOR = 3;

// Default reacting duration when a reaction has cooldownMs == 0.
static constexpr uint32_t DEFAULT_REACT_MS = 1500;

// ── Lifecycle ──────────────────────────────────────────────────────────────────

void GotchiBehaviour::begin(CreatureType type, const CreatureRegistry& registry) {
    _registry = &registry;
    setCreature(type, registry);
}

void GotchiBehaviour::setCreature(CreatureType type, const CreatureRegistry& registry) {
    _registry    = &registry;
    _type        = type;
    _state       = CreatureState::IDLE;
    _mood        = GotchiMood::NEUTRAL;
    _currentAnim = 0;
    _animChanged = true;
    _moodChanged = true;
    _idleMs      = 0;
    _attentionMs = 0;
    _reactingMs  = 0;
    for (auto& c : _cooldowns) c = 0;
}

// ── Event injection ────────────────────────────────────────────────────────────

void GotchiBehaviour::pushEvent(BehaviourEvent evt) {
    if (!_registry) return;

    const uint8_t idx = (uint8_t)evt;

    if (_cooldowns[idx] > 0) return;

    // REACTING / STARTLED only let through violent physical interrupts.
    if (_state == CreatureState::REACTING || _state == CreatureState::STARTLED) {
        if (evt != BehaviourEvent::IMU_SHAKE && evt != BehaviourEvent::MIC_NOISE_LOUD) return;
    }

    const CreatureDef* def = _registry->get(_type);
    if (!def) return;

    const Reaction& r = def->reactions[(uint8_t)_state][(uint8_t)evt];
    _applyReaction(r);

    if (r.cooldownMs > 0) _cooldowns[idx] = r.cooldownMs;

    // External stimulus resets idle + attention counters.
    if (evt <= BehaviourEvent::BTN_B) {
        _idleMs      = 0;
        _attentionMs = 0;
    }
    if (evt == BehaviourEvent::ATTENTION_TIMER) {
        _attentionMs = 0;
    }
}

// ── Update tick ────────────────────────────────────────────────────────────────

void GotchiBehaviour::update(uint32_t deltaMs) {
    for (auto& c : _cooldowns) {
        if (c > 0) c = (c > deltaMs) ? c - deltaMs : 0;
    }

    _tickReacting(deltaMs);
    _tickIdle(deltaMs);
    _tickAttention(deltaMs);
}

// ── Internal ticks ─────────────────────────────────────────────────────────────

void GotchiBehaviour::_applyReaction(const Reaction& r) {
    if (r.moodTarget != GotchiMood::_KEEP) {
        _mood        = r.moodTarget;
        _moodChanged = true;
    }
    if (r.animationId != NO_ANIM) {
        _currentAnim = r.animationId;
        _animChanged = true;
    }

    const CreatureState next = r.nextState;

    if (next == CreatureState::REACTING || next == CreatureState::STARTLED) {
        _reactingMs = r.cooldownMs > 0 ? r.cooldownMs : DEFAULT_REACT_MS;
    } else {
        _reactingMs = 0;
    }

    // Entering sleep resets the idle counter so the creature doesn't
    // immediately re-trigger long-idle events when it wakes.
    if (next == CreatureState::SLEEPING) {
        _idleMs      = 0;
        _attentionMs = 0;
    }

    _state = next;
}

void GotchiBehaviour::_tickReacting(uint32_t deltaMs) {
    if (_state != CreatureState::REACTING && _state != CreatureState::STARTLED) return;

    if (_reactingMs <= deltaMs) {
        _reactingMs = 0;
        _state      = CreatureState::IDLE;
    } else {
        _reactingMs -= deltaMs;
    }
}

void GotchiBehaviour::_tickIdle(uint32_t deltaMs) {
    if (_state != CreatureState::IDLE && _state != CreatureState::SEEKING) return;

    const CreatureDef* def = _registry ? _registry->get(_type) : nullptr;
    if (!def) return;

    _idleMs += deltaMs;

    const uint32_t sleepThreshold = def->longIdleMs * SLEEP_IDLE_FACTOR;

    // Check thresholds highest-priority first so sleep beats long-idle.
    if (_idleMs >= sleepThreshold) {
        pushEvent(BehaviourEvent::IDLE_SLEEP);
    } else if (_idleMs >= def->longIdleMs) {
        pushEvent(BehaviourEvent::IDLE_LONG);
    } else if (_idleMs >= def->shortIdleMs) {
        pushEvent(BehaviourEvent::IDLE_SHORT);
    }
}

void GotchiBehaviour::_tickAttention(uint32_t deltaMs) {
    if (_state == CreatureState::SLEEPING ||
        _state == CreatureState::REACTING ||
        _state == CreatureState::STARTLED) return;

    const CreatureDef* def = _registry ? _registry->get(_type) : nullptr;
    if (!def) return;

    _attentionMs += deltaMs;

    if (_attentionMs >= def->attentionThresholdMs) {
        pushEvent(BehaviourEvent::ATTENTION_TIMER);
    }
}
