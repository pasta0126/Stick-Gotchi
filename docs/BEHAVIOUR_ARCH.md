# GotchiBehaviour Engine — Architecture

## Overview

The new core of Stick-Gotchi is a **reactive companion system** replacing the care simulator.
The gotchi is an agent: it reacts to its physical environment and proactively seeks attention.

```
Sensors                    GotchiBehaviour              GotchiRenderer
──────────                 ───────────────              ──────────────
GotchiShake  ──IMU_TAP──►  EventQueue
GotchiShake  ──IMU_SHAKE►  │
GotchiShake  ──IMU_PICKUP► ▼
GotchiAudio  ──MIC_NOISE►  StateMachine ──reaction──►  AnimQueue
ButtonManager ─BTN_A/B───► │  (per creature)           │
ProactiveTimer─ATTENTION►  ▼                           ▼
                           ReactionSelector           GotchiPet.mood
                           (event x state x type)
```

## Event Types

```cpp
enum class BehaviourEvent : uint8_t {
    IMU_TAP,          // sharp delta-g spike — tap on desk/device
    IMU_SHAKE,        // rapid oscillation — being shaken
    IMU_PICKUP,       // lifted from prolonged stillness
    IMU_PUTDOWN,      // placed back after being held
    IMU_WALKING,      // gentle sustained movement — carried
    MIC_NOISE_SOFT,   // RMS > soft threshold (keyboard, music)
    MIC_NOISE_LOUD,   // RMS > loud threshold (yell, bang)
    BTN_A,            // direct touch — "poke"
    BTN_B,            // secondary touch — "prod"
    IDLE_SHORT,       // ~2 min without any input
    IDLE_LONG,        // ~10 min without any input
    IDLE_SLEEP,       // night hours or ~30 min stillness
    ATTENTION_TIMER,  // proactive attention request fired
};
```

## Creature States

```cpp
enum class CreatureState : uint8_t {
    IDLE,       // default — ambient animations playing
    REACTING,   // mid-reaction animation, ignores new events
    SEEKING,    // actively requesting interaction
    PLAYING,    // happy sustained interaction
    SLEEPING,   // low-power idle, rare animations
    STARTLED,   // sudden shock (brief, high-priority)
};
```

## Creature Types

```cpp
enum class CreatureType : uint8_t {
    BYTEE   = 0,  // magic steampunk robot — curious, studious, surprised
    CTHULHU = 1,  // baby eldritch — creepy-cute, wants hugs, chaotic
    JACK    = 2,  // stone with hat — stoic deadpan, rarely reacts
    LUMI    = 3,  // deer-fox fae — vivid, anxious, most reactive
};
```

Full personality specs and visual reference: `docs/CREATURES.md`

## Reaction Table

Each creature has a `ReactionTable`: `(CreatureState × BehaviourEvent) → Reaction`.

```cpp
struct Reaction {
    GotchiMood    moodTarget;   // mood to set on GotchiPet
    uint8_t       animationId;  // index into creature anim table
    CreatureState nextState;    // state transition
    uint16_t      cooldownMs;   // min ms before same event fires again
};
```

## Proactive Attention System

A software timer fires `ATTENTION_TIMER` when no interaction for `attentionThresholdMs`.
Each creature has an `attention_set[]` with unique behaviors:

- BABY_CTHULHU: tentacle tap on glass, eerie stare, summons fog
- ROCK_HAT: absolutely nothing for a long time, then ONE slow blink at you
- FANTASY: bounces to edge of screen, waves frantically

Timer resets on any event except `IDLE_*`.

## Idle Ambient System

When `IDLE`, a secondary timer cycles `idle_set[]`:
- Short idle: wander, look around, blink, small gestures
- Long idle: yawn, stretch, sit down, start dozing

Each creature's idle set communicates personality passively.

## CreatureRegistry

```cpp
struct CreatureDef {
    CreatureType  type;
    const char*   name;
    uint32_t      visualSeed;
    Reaction      reactions[NUM_STATES][NUM_EVENTS];
    uint8_t       idleAnims[8];
    uint8_t       idleAnimCount;
    uint8_t       attentionAnims[4];
    uint8_t       attentionAnimCount;
    uint32_t      attentionThresholdMs;
    uint32_t      shortIdleMs;
    uint32_t      longIdleMs;
    uint8_t       micSensitivity;   // 0-255
    uint8_t       imuSensitivity;   // 0-255
};
```

## Simplified GotchiPet (post-reboot)

### Keeps
- `CreatureType currentCreature`
- `GotchiMood currentMood`
- `uint32_t totalCompanionMs` (NVS)
- `uint32_t lastInteractionMs`
- `uint32_t interactionsToday`

### Removed
- hunger, thirst, energy, health floats + decay timers
- sickness / death / agony logic
- lifecycle stages EGG/BABY/YOUNG/ADULT
- evolution paths, AdultForm, GotchiType 6-variant system
- heritage bonuses, ancestor tracking

## File Plan

```
src/
  gotchi/
    GotchiBehaviour.h/.cpp     NEW  event queue + state machine + reaction selector
    CreatureRegistry.h/.cpp    NEW  creature definitions table
    GotchiPet.h/.cpp           MOD  strip stat system, keep mood + persistence
  apps/
    gotchi/
      GotchiApp.h/.cpp         MOD  remove action bar, route sensors to behaviour
      GotchiRenderer.h/.cpp    MOD  remove action bar draw, richer expressions
    stats/
      StatsApp.h/.cpp          MOD  replace tabs with companion diary
```

## Personality Reference

| Criatura | IMU | Mic | Atencion | Idle |
|---|---|---|---|---|
| Bytee | Media | Med-Alta | 4 min | Lee, hace magia, mira curioso |
| Cthulhu | Media | Alta | 5 min | Tentaculos lentos, brillo eldrico |
| Jack | Muy baja | Casi nula | 12 min | No hace nada. Eso es el chiste. |
| Lumi | Alta | Media | 3 min | Cola, chispas, ojos enormes |
