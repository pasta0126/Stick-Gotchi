#pragma once
#include "GotchiBehaviour.h"

static constexpr uint8_t MAX_IDLE_ANIMS      = 8;
static constexpr uint8_t MAX_ATTENTION_ANIMS = 4;

// ── Creature definition ────────────────────────────────────────────────────────

struct CreatureDef {
    CreatureType  type;
    const char*   name;
    uint32_t      visualSeed;

    Reaction reactions[(uint8_t)CreatureState::_COUNT]
                      [(uint8_t)BehaviourEvent::_COUNT];

    uint8_t  idleAnims[MAX_IDLE_ANIMS];
    uint8_t  idleAnimCount;

    uint8_t  attentionAnims[MAX_ATTENTION_ANIMS];
    uint8_t  attentionAnimCount;

    uint32_t attentionThresholdMs;
    uint32_t shortIdleMs;
    uint32_t longIdleMs;

    uint8_t  micSensitivity;   // 0-255
    uint8_t  imuSensitivity;   // 0-255
};

// ── Registry singleton ─────────────────────────────────────────────────────────

class CreatureRegistry {
public:
    static const CreatureRegistry& instance();

    const CreatureDef* get(CreatureType type) const;
    uint8_t            count() const { return (uint8_t)CreatureType::_COUNT; }

private:
    CreatureRegistry();
    static CreatureDef _defs[(uint8_t)CreatureType::_COUNT];
};
