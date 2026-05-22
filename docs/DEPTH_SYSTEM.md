# Depth System — Ephemeral 3D Space

## Concept

The creature exists in an imaginary Z-axis in front of the screen.
It is never fully "contained" by the display — it can lean in until only its eyes are visible,
or retreat until it's a tiny silhouette in the distance.

This Z-movement IS expression. The creature communicates through where it positions itself
as much as through its facial animations.

```
Z = 0.0  FAR      Z = 1.0  NORMAL     Z = 1.6  CLOSE     Z = 2.0  PEEK
┌───────┐          ┌───────┐           ┌───────┐           ┌───────┐
│       │          │       │           │  ┌──┐ │           │██████ │
│  [o]  │          │ [   ] │           │  │  │ │           │██  ██ │  ← only eyes
│       │          │       │           └──┘  └─┘           └───────┘
└───────┘          └───────┘           (overflows)          (cropped)
  ~20% h             ~55% h              ~85% h              face only
```

## Z-Depth Scale

| Z Value | Name | Sprite scale | Behavior |
|---|---|---|---|
| 0.0 – 0.3 | FAR | ~20% screen height | Retreating, scared, sulking, sleeping far |
| 0.4 – 0.7 | MID | ~40% screen height | Default idle position |
| 0.8 – 1.2 | NORMAL | ~55% screen height | Engaged, reacting, happy idle |
| 1.3 – 1.7 | CLOSE | ~75-90% screen height | Excited, seeking attention, very happy |
| 1.8 – 2.0 | PEEK | >100%, cropped | Extreme close — only eyes/forehead visible |

## Renderer Implementation

GotchiRenderer maintains:
```cpp
float _zDepth       = 1.0f;   // current depth
float _zTarget      = 1.0f;   // target depth (behaviour engine sets this)
float _zVelocity    = 0.0f;   // for smooth interpolation
```

Each frame: `_zDepth = lerp(_zDepth, _zTarget, zLerpSpeed * deltaMs)`

Sprite is drawn centered horizontally, anchored to bottom of creature visible area,
scaled by `spriteScale = baseSizePx * zDepth`. At PEEK, the draw origin is pushed down
so only the top portion (eyes) is visible within the 135px height.

## Z-Movement as Vocabulary

| Z Motion | Meaning | Example Triggers |
|---|---|---|
| Slow approach MID→CLOSE | Curiosity, interest | MIC_NOISE_SOFT, BTN_A |
| Fast rush FAR→CLOSE | Excitement, joy | BTN_A (Lumi), IMU_PICKUP |
| Slow retreat CLOSE→FAR | Fear, sadness, shyness | IMU_SHAKE, IDLE_SLEEP |
| Hover at PEEK | Watching, stalking, boredom peak | ATTENTION_TIMER (Cthulhu) |
| Oscillate MID↔CLOSE | Anxious, can't decide | Lumi attention behavior |
| Stay at FAR | Sulking, deep sleep | Jack post-shake |
| Drift MID→FAR slowly | Dozing off | IDLE_LONG |

## Per-Creature Z Personality

| Creature | Default Z | Range | Z Speed | Notes |
|---|---|---|---|---|
| Bytee | 1.0 | 0.4 – 1.7 | Medium | Floats gently, rushes toward on discoveries |
| Cthulhu | 0.9 | 0.2 – 2.0 | Slow | Creeping approaches, dramatic retreats |
| Jack | 1.0 | 0.8 – 1.1 | Glacial | Barely moves on Z. Ever. |
| Lumi | 1.1 | 0.5 – 1.8 | Fast | Most Z-active, rushes in constantly |

## Speech Bubbles

Positioned relative to creature's current Z and screen position.

```
NORMAL Z:                  CLOSE Z:                   FAR Z:
┌───────────┐              ┌──────┐                   ┌──────────────┐
│ Hello!    │              │ HI!! │                   │ ...          │
└─────┬─────┘              └──┬───┘                   └──────┬───────┘
      ▼                       ▼  (larger, higher)            ▼ (small, centered)
   [creature]             [CREATURE]                      [creature]
```

### Bubble Types

| Creature | Style | Border | Tail |
|---|---|---|---|
| Bytee | Star/magic burst | Gold dots | Crystal point |
| Cthulhu | Organic blob | Tentacle squiggle | Tentacle curl |
| Jack | Flat rectangle | Single pixel line | Right angle |
| Lumi | Soft cloud | Rounded, sparkle | Gentle curve |

### Text Types

| Type | Use | Display |
|---|---|---|
| Short reaction | 1-4 words, instant | Static bubble, 2-3 seconds |
| Attention phrase | 3-8 words | Static bubble, 4 seconds |
| Long message | 8+ words | Marquee scroll, left to right |

### Sample Phrases Per Creature

**Bytee:**
- Reactions: "!", "Oh!", "Interesting...", "Calculating..."
- Attention: "Hey, are you there?", "I found something!", "Look at this!"

**Cthulhu:**
- Reactions: "eep!", "the sounds...", "*tentacle noises*", "hugs?"
- Attention: "free hugs available", "...I see you", "ancient loneliness"

**Jack:**
- Reactions: (nothing, usually), ".", "hmm"
- Attention: (after 12 min): "..." (that's it)

**Lumi:**
- Reactions: "!!!", "oh oh oh!", "notice me!", "*sparkles*"
- Attention: "HELLO??", "pay attention to meeee", "I'm right here!!!"

---

## Sprite Requirements for Aseprite

Each creature needs sprites designed at **base size MID** with the renderer scaling for other depths.
However, for PEEK state a dedicated cropped frame works better than scaling.

### Recommended sprite sizes (MID base)
```
Display: 240 x 135 px (landscape)
MID target: ~55% of 135px height = ~74px tall

Recommended canvas per creature: 64 x 64 px
  - Fits ~55% height when rendered at 1.0x
  - Scales to ~13px at FAR (z=0.2), fills screen at CLOSE (z=1.8)
  - PEEK frame: separate 80x40 sprite showing only face/eyes region
```

### Layers per creature in Aseprite
```
Layer structure:
  [body]        — main body, all poses
  [face]        — eyes + expression, swappable
  [accessory]   — hat (Jack), cape (Bytee), antlers (Lumi), wings (Cthulhu)
  [overlay]     — sparkles, glow, effects (Bytee magic, Lumi sparkles)
```

### Animation tags needed (minimum viable)
```
idle_far      — small, slow minimal movement (2-4 frames)
idle_mid      — default idle (4-6 frames)
idle_close    — enlarged idle, more expressive (4 frames)
peek          — eyes only, blinking (2-3 frames)
happy         — joy expression (4 frames)
startled      — shock (3 frames, fast)
sleepy        — closing eyes, drooping (4 frames)
attention     — creature-specific attention behavior (4-6 frames)
```

**Blocked until:** user provides reference sketches/mockups for creature sizes and expressions.
