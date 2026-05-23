#include "CreatureRegistry.h"
#include "CreatureAnimIds.h"

using M = GotchiMood;
using S = CreatureState;
using E = BehaviourEvent;

static constexpr uint8_t NS = (uint8_t)S::_COUNT;
static constexpr uint8_t NE = (uint8_t)E::_COUNT;

// Fill the entire reaction table with passthrough NOOPs.
static void _noopAll(CreatureDef& d) {
    for (uint8_t s = 0; s < NS; ++s)
        for (uint8_t e = 0; e < NE; ++e)
            d.reactions[s][e] = Reaction{ M::_KEEP, NO_ANIM, (S)s, 0 };
}

// ── BYTEE ──────────────────────────────────────────────────────────────────────

static void _buildBytee(CreatureDef& d) {
    namespace A = ByteeAnim;

    d.type                 = CreatureType::BYTEE;
    d.name                 = "Bytee";
    d.visualSeed           = 0xB17EE001;
    d.attentionThresholdMs = 4  * 60 * 1000;
    d.shortIdleMs          = 2  * 60 * 1000;
    d.longIdleMs           = 10 * 60 * 1000;
    d.micSensitivity       = 160;
    d.imuSensitivity       = 128;

    d.idleAnims[0] = A::LOOK_AROUND;
    d.idleAnims[1] = A::READ_BOOK;
    d.idleAnimCount = 2;

    d.attentionAnims[0] = A::ATTENTION_FLOAT;
    d.attentionAnims[1] = A::ATTENTION_SPARK;
    d.attentionAnims[2] = A::ATTENTION_ANTENNA;
    d.attentionAnimCount = 3;

    _noopAll(d);

    // ── IDLE ──────────────────────────────────────────────────────────
    auto& I = d.reactions[(uint8_t)S::IDLE];
    I[(uint8_t)E::IMU_TAP]         = Reaction{ M::STARTLED, A::STARTLED_CURIOUS,  S::STARTLED, 3000  };
    I[(uint8_t)E::IMU_SHAKE]       = Reaction{ M::DIZZY,    A::TUMBLE_DIZZY,      S::REACTING, 2500  };
    I[(uint8_t)E::IMU_PICKUP]      = Reaction{ M::EXCITED,  A::PICKUP_JOY,        S::REACTING, 2000  };
    I[(uint8_t)E::IMU_PUTDOWN]     = Reaction{ M::PENSIVE,  A::SET_DOWN,          S::REACTING, 1500  };
    I[(uint8_t)E::IMU_WALKING]     = Reaction{ M::HAPPY,    A::WONDER_WALK,       S::IDLE,     500   };
    I[(uint8_t)E::MIC_NOISE_SOFT]  = Reaction{ M::HAPPY,    A::TILT_ANALYZE,      S::REACTING, 2000  };
    I[(uint8_t)E::MIC_NOISE_LOUD]  = Reaction{ M::SCARED,   A::SCARED_VISOR,      S::STARTLED, 4000  };
    I[(uint8_t)E::BTN_A]           = Reaction{ M::HAPPY,    A::HAPPY_SPARK,       S::PLAYING,  1500  };
    I[(uint8_t)E::BTN_B]           = Reaction{ M::EXCITED,  A::SURPRISE_BTN,      S::REACTING, 1500  };
    I[(uint8_t)E::IDLE_SHORT]      = Reaction{ M::_KEEP,    A::LOOK_AROUND,       S::IDLE,     60000 };
    I[(uint8_t)E::IDLE_LONG]       = Reaction{ M::_KEEP,    A::SLEEP,             S::IDLE,     120000};
    I[(uint8_t)E::IDLE_SLEEP]      = Reaction{ M::SLEEPING, A::SLEEP,             S::SLEEPING, 0     };
    I[(uint8_t)E::ATTENTION_TIMER] = Reaction{ M::EXCITED,  A::ATTENTION_FLOAT,   S::SEEKING,  0     };

    // ── REACTING / STARTLED ───────────────────────────────────────────
    auto& RT = d.reactions[(uint8_t)S::REACTING];
    RT[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::DIZZY,  A::TUMBLE_DIZZY, S::REACTING, 2500 };
    RT[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::SCARED, A::SCARED_VISOR, S::STARTLED, 4000 };
    auto& ST = d.reactions[(uint8_t)S::STARTLED];
    ST[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::DIZZY,  A::TUMBLE_DIZZY, S::REACTING, 2500 };
    ST[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::SCARED, A::SCARED_VISOR, S::STARTLED, 4000 };

    // ── SEEKING ───────────────────────────────────────────────────────
    auto& SK = d.reactions[(uint8_t)S::SEEKING];
    SK[(uint8_t)E::BTN_A]           = Reaction{ M::HAPPY,   A::HAPPY_SPARK,       S::PLAYING,  1500 };
    SK[(uint8_t)E::BTN_B]           = Reaction{ M::EXCITED, A::SURPRISE_BTN,      S::PLAYING,  1500 };
    SK[(uint8_t)E::IMU_TAP]         = Reaction{ M::HAPPY,   A::STARTLED_CURIOUS,  S::REACTING, 2000 };
    SK[(uint8_t)E::IMU_PICKUP]      = Reaction{ M::EXCITED, A::PICKUP_JOY,        S::REACTING, 2000 };
    SK[(uint8_t)E::ATTENTION_TIMER] = Reaction{ M::_KEEP,   A::ATTENTION_SPARK,   S::SEEKING,  0    };

    // ── PLAYING ───────────────────────────────────────────────────────
    auto& PL = d.reactions[(uint8_t)S::PLAYING];
    PL[(uint8_t)E::BTN_A]      = Reaction{ M::HAPPY,   A::HAPPY_SPARK,  S::PLAYING, 1000 };
    PL[(uint8_t)E::BTN_B]      = Reaction{ M::EXCITED, A::SURPRISE_BTN, S::PLAYING, 1000 };
    PL[(uint8_t)E::IDLE_SHORT] = Reaction{ M::_KEEP,   A::LOOK_AROUND,  S::IDLE,    500  };

    // ── SLEEPING ──────────────────────────────────────────────────────
    auto& SL = d.reactions[(uint8_t)S::SLEEPING];
    SL[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::ANNOYED, A::STARTLED_CURIOUS, S::REACTING, 3000 };
    SL[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::SCARED,  A::SCARED_VISOR,     S::STARTLED, 3000 };
    SL[(uint8_t)E::BTN_A]          = Reaction{ M::HAPPY,   A::PICKUP_JOY,       S::REACTING, 2000 };
}

// ── CTHULHU ────────────────────────────────────────────────────────────────────

static void _buildCthulhu(CreatureDef& d) {
    namespace A = CthulhuAnim;

    d.type                 = CreatureType::CTHULHU;
    d.name                 = "Cthulhu";
    d.visualSeed           = 0xC7110002;
    d.attentionThresholdMs = 5  * 60 * 1000;
    d.shortIdleMs          = 2  * 60 * 1000;
    d.longIdleMs           = 8  * 60 * 1000;
    d.micSensitivity       = 200;
    d.imuSensitivity       = 128;

    d.idleAnims[0] = A::TENTACLE_WIGGLE;
    d.idleAnimCount = 1;

    d.attentionAnims[0] = A::ATTENTION_SIGN;
    d.attentionAnims[1] = A::ATTENTION_REACH;
    d.attentionAnims[2] = A::ATTENTION_GLOW;
    d.attentionAnimCount = 3;

    _noopAll(d);

    // ── IDLE ──────────────────────────────────────────────────────────
    auto& I = d.reactions[(uint8_t)S::IDLE];
    I[(uint8_t)E::IMU_TAP]         = Reaction{ M::PENSIVE,  A::TENTACLE_FLAIL,  S::REACTING, 2000  };
    I[(uint8_t)E::IMU_SHAKE]       = Reaction{ M::SCARED,   A::PANIC_TENTACLES, S::REACTING, 2500  };
    I[(uint8_t)E::IMU_PICKUP]      = Reaction{ M::HAPPY,    A::CLING_PICKUP,    S::REACTING, 2000  };
    I[(uint8_t)E::IMU_PUTDOWN]     = Reaction{ M::PENSIVE,  A::IDLE,            S::IDLE,     1000  };
    I[(uint8_t)E::IMU_WALKING]     = Reaction{ M::HAPPY,    A::WALKING_CONTENT, S::IDLE,     500   };
    I[(uint8_t)E::MIC_NOISE_SOFT]  = Reaction{ M::EXCITED,  A::COSMIC_ABSORB,   S::REACTING, 2000  };
    I[(uint8_t)E::MIC_NOISE_LOUD]  = Reaction{ M::EXCITED,  A::ELDRITCH_SURGE,  S::REACTING, 3000  };
    I[(uint8_t)E::BTN_A]           = Reaction{ M::HAPPY,    A::HUGS_REACH,      S::PLAYING,  1500  };
    I[(uint8_t)E::BTN_B]           = Reaction{ M::EXCITED,  A::CUBE_OFFER,      S::REACTING, 1500  };
    I[(uint8_t)E::IDLE_SHORT]      = Reaction{ M::_KEEP,    A::TENTACLE_WIGGLE, S::IDLE,     45000 };
    I[(uint8_t)E::IDLE_LONG]       = Reaction{ M::_KEEP,    A::CURL_SLEEP,      S::IDLE,     90000 };
    I[(uint8_t)E::IDLE_SLEEP]      = Reaction{ M::SLEEPING, A::CURL_SLEEP,      S::SLEEPING, 0     };
    I[(uint8_t)E::ATTENTION_TIMER] = Reaction{ M::HAPPY,    A::ATTENTION_SIGN,  S::SEEKING,  0     };

    // ── REACTING / STARTLED ───────────────────────────────────────────
    auto& RT = d.reactions[(uint8_t)S::REACTING];
    RT[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::SCARED,  A::PANIC_TENTACLES, S::REACTING, 2500 };
    RT[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::EXCITED, A::ELDRITCH_SURGE,  S::REACTING, 3000 };
    auto& ST = d.reactions[(uint8_t)S::STARTLED];
    ST[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::SCARED,  A::PANIC_TENTACLES, S::REACTING, 2500 };
    ST[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::EXCITED, A::ELDRITCH_SURGE,  S::REACTING, 3000 };

    // ── SEEKING ───────────────────────────────────────────────────────
    auto& SK = d.reactions[(uint8_t)S::SEEKING];
    SK[(uint8_t)E::BTN_A]           = Reaction{ M::HAPPY,   A::HUGS_REACH,      S::PLAYING,  1500 };
    SK[(uint8_t)E::BTN_B]           = Reaction{ M::EXCITED, A::CUBE_OFFER,      S::PLAYING,  1500 };
    SK[(uint8_t)E::IMU_PICKUP]      = Reaction{ M::HAPPY,   A::CLING_PICKUP,    S::REACTING, 2000 };
    SK[(uint8_t)E::ATTENTION_TIMER] = Reaction{ M::_KEEP,   A::ATTENTION_REACH, S::SEEKING,  0    };

    // ── PLAYING ───────────────────────────────────────────────────────
    auto& PL = d.reactions[(uint8_t)S::PLAYING];
    PL[(uint8_t)E::BTN_A]      = Reaction{ M::HAPPY, A::HUGS_REACH,      S::PLAYING, 1000 };
    PL[(uint8_t)E::IDLE_SHORT] = Reaction{ M::_KEEP, A::TENTACLE_WIGGLE, S::IDLE,    500  };

    // ── SLEEPING ──────────────────────────────────────────────────────
    auto& SL = d.reactions[(uint8_t)S::SLEEPING];
    SL[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::ANNOYED, A::PANIC_TENTACLES, S::REACTING, 3000 };
    SL[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::EXCITED, A::ELDRITCH_SURGE,  S::STARTLED, 2000 };
    SL[(uint8_t)E::BTN_A]          = Reaction{ M::HAPPY,   A::HUGS_REACH,      S::REACTING, 2000 };
}

// ── JACK ───────────────────────────────────────────────────────────────────────

static void _buildJack(CreatureDef& d) {
    namespace A = JackAnim;

    d.type                 = CreatureType::JACK;
    d.name                 = "Jack";
    d.visualSeed           = 0xACE5701E;
    d.attentionThresholdMs = 12 * 60 * 1000;
    d.shortIdleMs          = 5  * 60 * 1000;
    d.longIdleMs           = 15 * 60 * 1000;
    d.micSensitivity       = 20;
    d.imuSensitivity       = 30;

    d.idleAnims[0] = A::IDLE;
    d.idleAnimCount = 1;

    d.attentionAnims[0] = A::SLOW_BLINK;
    d.attentionAnimCount = 1;

    _noopAll(d);

    // ── IDLE ──────────────────────────────────────────────────────────
    auto& I = d.reactions[(uint8_t)S::IDLE];
    I[(uint8_t)E::IMU_TAP]         = Reaction{ M::_KEEP,    A::SLOW_BLINK,       S::REACTING, 8000  };
    I[(uint8_t)E::IMU_SHAKE]       = Reaction{ M::ANNOYED,  A::HAT_TILT_ANNOYED, S::REACTING, 5000  };
    I[(uint8_t)E::IMU_PICKUP]      = Reaction{ M::_KEEP,    A::TINY_EXCLAMATION, S::REACTING, 10000 };
    I[(uint8_t)E::IMU_WALKING]     = Reaction{ M::_KEEP,    A::HAT_BOB,          S::IDLE,     15000 };
    I[(uint8_t)E::MIC_NOISE_LOUD]  = Reaction{ M::_KEEP,    A::HAT_MICRO_LIFT,   S::REACTING, 5000  };
    I[(uint8_t)E::BTN_A]           = Reaction{ M::_KEEP,    A::TINY_HEART,       S::REACTING, 5000  };
    I[(uint8_t)E::IDLE_SHORT]      = Reaction{ M::_KEEP,    NO_ANIM,             S::IDLE,     120000};
    I[(uint8_t)E::IDLE_LONG]       = Reaction{ M::_KEEP,    A::SLEEP_ZZZ,        S::IDLE,     180000};
    I[(uint8_t)E::IDLE_SLEEP]      = Reaction{ M::SLEEPING, A::SLEEP_ZZZ,        S::SLEEPING, 0     };
    I[(uint8_t)E::ATTENTION_TIMER] = Reaction{ M::_KEEP,    A::SLOW_BLINK,       S::SEEKING,  0     };

    // ── REACTING / STARTLED ───────────────────────────────────────────
    auto& RT = d.reactions[(uint8_t)S::REACTING];
    RT[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::ANNOYED, A::HAT_TILT_ANNOYED, S::REACTING, 5000 };
    RT[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::_KEEP,   A::HAT_MICRO_LIFT,   S::REACTING, 3000 };
    auto& ST = d.reactions[(uint8_t)S::STARTLED];
    ST[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::ANNOYED, A::HAT_TILT_ANNOYED, S::REACTING, 5000 };
    ST[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::_KEEP,   A::HAT_MICRO_LIFT,   S::REACTING, 3000 };

    // ── SEEKING: Jack blinks once, then gives up ──────────────────────
    auto& SK = d.reactions[(uint8_t)S::SEEKING];
    SK[(uint8_t)E::BTN_A]           = Reaction{ M::_KEEP, A::TINY_HEART, S::PLAYING, 3000 };
    SK[(uint8_t)E::ATTENTION_TIMER] = Reaction{ M::_KEEP, NO_ANIM,       S::IDLE,    0    };

    // ── PLAYING ───────────────────────────────────────────────────────
    auto& PL = d.reactions[(uint8_t)S::PLAYING];
    PL[(uint8_t)E::BTN_A]      = Reaction{ M::_KEEP, A::TINY_HEART, S::PLAYING, 3000 };
    PL[(uint8_t)E::IDLE_SHORT] = Reaction{ M::_KEEP, NO_ANIM,       S::IDLE,    500  };

    // ── SLEEPING ──────────────────────────────────────────────────────
    auto& SL = d.reactions[(uint8_t)S::SLEEPING];
    SL[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::ANNOYED, A::HAT_TILT_ANNOYED, S::REACTING, 5000 };
    SL[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::_KEEP,   A::HAT_MICRO_LIFT,   S::REACTING, 3000 };
    SL[(uint8_t)E::BTN_A]          = Reaction{ M::_KEEP,   A::SLOW_BLINK,       S::REACTING, 3000 };
}

// ── LUMI ───────────────────────────────────────────────────────────────────────

static void _buildLumi(CreatureDef& d) {
    namespace A = LumiAnim;

    d.type                 = CreatureType::LUMI;
    d.name                 = "Lumi";
    d.visualSeed           = 0x10D1F4E3;
    d.attentionThresholdMs = 3 * 60 * 1000;
    d.shortIdleMs          = 1 * 60 * 1000;
    d.longIdleMs           = 5 * 60 * 1000;
    d.micSensitivity       = 128;
    d.imuSensitivity       = 210;

    d.idleAnims[0] = A::TAIL_WAG;
    d.idleAnims[1] = A::EARS_PERK;
    d.idleAnimCount = 2;

    d.attentionAnims[0] = A::ATTENTION_BOUNCE;
    d.attentionAnims[1] = A::ATTENTION_FLASH;
    d.attentionAnims[2] = A::ATTENTION_EYES;
    d.attentionAnimCount = 3;

    _noopAll(d);

    // ── IDLE ──────────────────────────────────────────────────────────
    auto& I = d.reactions[(uint8_t)S::IDLE];
    I[(uint8_t)E::IMU_TAP]         = Reaction{ M::EXCITED,  A::JUMP_EXCITED,    S::REACTING, 2500  };
    I[(uint8_t)E::IMU_SHAKE]       = Reaction{ M::SCARED,   A::CURL_SCARED,     S::REACTING, 3000  };
    I[(uint8_t)E::IMU_PICKUP]      = Reaction{ M::EXCITED,  A::PICKUP_SPARKLES, S::REACTING, 2000  };
    I[(uint8_t)E::IMU_PUTDOWN]     = Reaction{ M::PENSIVE,  A::EARS_PERK,       S::REACTING, 1500  };
    I[(uint8_t)E::IMU_WALKING]     = Reaction{ M::HAPPY,    A::HAPPY_BOUNCE,    S::IDLE,     500   };
    I[(uint8_t)E::MIC_NOISE_SOFT]  = Reaction{ M::_KEEP,    A::LISTEN_TILT,     S::REACTING, 1500  };
    I[(uint8_t)E::MIC_NOISE_LOUD]  = Reaction{ M::STARTLED, A::STARTLED_JUMP,   S::STARTLED, 3000  };
    I[(uint8_t)E::BTN_A]           = Reaction{ M::HAPPY,    A::NUZZLE_HEART,    S::PLAYING,  1500  };
    I[(uint8_t)E::BTN_B]           = Reaction{ M::EXCITED,  A::PAWS_PLAYFUL,    S::REACTING, 1500  };
    I[(uint8_t)E::IDLE_SHORT]      = Reaction{ M::_KEEP,    A::TAIL_WAG,        S::IDLE,     30000 };
    I[(uint8_t)E::IDLE_LONG]       = Reaction{ M::_KEEP,    A::CURL_SLEEP,      S::IDLE,     60000 };
    I[(uint8_t)E::IDLE_SLEEP]      = Reaction{ M::SLEEPING, A::CURL_SLEEP,      S::SLEEPING, 0     };
    I[(uint8_t)E::ATTENTION_TIMER] = Reaction{ M::EXCITED,  A::ATTENTION_BOUNCE,S::SEEKING,  0     };

    // ── REACTING / STARTLED ───────────────────────────────────────────
    auto& RT = d.reactions[(uint8_t)S::REACTING];
    RT[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::SCARED,   A::CURL_SCARED,   S::REACTING, 3000 };
    RT[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::STARTLED, A::STARTLED_JUMP, S::STARTLED, 3000 };
    auto& ST = d.reactions[(uint8_t)S::STARTLED];
    ST[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::SCARED,   A::CURL_SCARED,   S::REACTING, 3000 };
    ST[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::STARTLED, A::STARTLED_JUMP, S::STARTLED, 3000 };

    // ── SEEKING ───────────────────────────────────────────────────────
    auto& SK = d.reactions[(uint8_t)S::SEEKING];
    SK[(uint8_t)E::BTN_A]           = Reaction{ M::HAPPY,   A::NUZZLE_HEART,    S::PLAYING,  1500 };
    SK[(uint8_t)E::BTN_B]           = Reaction{ M::EXCITED, A::PAWS_PLAYFUL,    S::PLAYING,  1000 };
    SK[(uint8_t)E::IMU_TAP]         = Reaction{ M::EXCITED, A::JUMP_EXCITED,    S::REACTING, 2000 };
    SK[(uint8_t)E::IMU_PICKUP]      = Reaction{ M::EXCITED, A::PICKUP_SPARKLES, S::REACTING, 2000 };
    SK[(uint8_t)E::ATTENTION_TIMER] = Reaction{ M::_KEEP,   A::ATTENTION_FLASH, S::SEEKING,  0    };

    // ── PLAYING ───────────────────────────────────────────────────────
    auto& PL = d.reactions[(uint8_t)S::PLAYING];
    PL[(uint8_t)E::BTN_A]      = Reaction{ M::HAPPY,   A::NUZZLE_HEART, S::PLAYING, 1000 };
    PL[(uint8_t)E::BTN_B]      = Reaction{ M::EXCITED, A::PAWS_PLAYFUL, S::PLAYING, 800  };
    PL[(uint8_t)E::IDLE_SHORT] = Reaction{ M::_KEEP,   A::TAIL_WAG,     S::IDLE,    500  };

    // ── SLEEPING ──────────────────────────────────────────────────────
    auto& SL = d.reactions[(uint8_t)S::SLEEPING];
    SL[(uint8_t)E::IMU_SHAKE]      = Reaction{ M::SCARED,   A::CURL_SCARED,   S::REACTING, 3000 };
    SL[(uint8_t)E::MIC_NOISE_LOUD] = Reaction{ M::STARTLED, A::STARTLED_JUMP, S::STARTLED, 2000 };
    SL[(uint8_t)E::BTN_A]          = Reaction{ M::HAPPY,    A::NUZZLE_HEART,  S::REACTING, 2000 };
}

// ── Registry ───────────────────────────────────────────────────────────────────

CreatureDef CreatureRegistry::_defs[(uint8_t)CreatureType::_COUNT] = {};

CreatureRegistry::CreatureRegistry() {
    _buildBytee  (_defs[(uint8_t)CreatureType::BYTEE]);
    _buildCthulhu(_defs[(uint8_t)CreatureType::CTHULHU]);
    _buildJack   (_defs[(uint8_t)CreatureType::JACK]);
    _buildLumi   (_defs[(uint8_t)CreatureType::LUMI]);
}

const CreatureRegistry& CreatureRegistry::instance() {
    static CreatureRegistry inst;
    return inst;
}

const CreatureDef* CreatureRegistry::get(CreatureType type) const {
    const uint8_t idx = (uint8_t)type;
    if (idx >= (uint8_t)CreatureType::_COUNT) return nullptr;
    return &_defs[idx];
}
