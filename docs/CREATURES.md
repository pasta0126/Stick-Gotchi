# Stick-Gotchi — Creature Design Reference

Four selectable companions, each with distinct personality, reaction style, and idle behavior.
Visual references in `docs/referencias/`.

---

## BYTEE

**Files:** `bytee1.png`, `bytee2.png`
**Type:** `CreatureType::BYTEE`

### Visual
Small steampunk-magic robot. Round helmet with black visor (glowing purple eyes).
Gold medieval engravings on white/cream armor. Purple cape. Crystal-tipped antenna.
Gem on chest that glows when excited. Tiny black gauntlet hands.

### Personality
Curious and studious. Gets genuinely surprised and delighted. Has a magical side —
spontaneously reads books, brews potions, casts small spells. Floats when very happy.
Asks questions with a "?" bubble. Expressive despite having no visible mouth.

### Behaviour Profile
| Parameter | Value |
|---|---|
| Attention threshold | 4 min |
| IMU sensitivity | Medium (reacts clearly, not hair-trigger) |
| Mic sensitivity | Medium-high (perks up at sounds, curious) |
| Short idle | Looks around, reads tiny book, pokes antenna |
| Long idle | Falls asleep reading, ZZZ bubble, book falls |
| Attention behavior | Floats up, waves; casts spark; antenna blinks frantically |

### Reactions
| Event | Reaction |
|---|---|
| IMU_TAP | Startled, then curious — looks at source |
| IMU_SHAKE | Tumbles, gets dizzy, visor scrambles |
| IMU_PICKUP | Excited — floats slightly, eyes glow bright |
| IMU_WALKING | Looks around with wonder |
| MIC_NOISE_SOFT | Head tilts, curious — analyzes sound |
| MIC_NOISE_LOUD | Jumps, visor cracks briefly, then SCARED |
| BTN_A | Happy spark — bounces, chest gem glows |
| BTN_B | Surprised "!" bubble, then happy |

---

## CTHULHU

**Files:** `chtulhu1.png`, `chtulhu2.png`, `cthulhu3.png`
**Type:** `CreatureType::CTHULHU`

### Visual
Baby Cthulhu. Chubby green body with bumpy texture. Six tentacles around face/mouth area.
Small leathery wings. Oversized black eyes with highlight dot. Stubby clawed feet.
Darker green bumps/nodes on head. Adorably grotesque.

### Personality
Creepy-cute eldritch baby. Wants hugs desperately ("FREE HUGS" sign).
Chaotic and unpredictable in reactions. Can go from scared to glowing with eldritch energy
to deeply asleep in seconds. Has ancient cosmic power but is also very smol.

### Behaviour Profile
| Parameter | Value |
|---|---|
| Attention threshold | 5 min |
| IMU sensitivity | Medium (reacts with tentacle flail) |
| Mic sensitivity | High (sound = eldritch resonance = strong reaction) |
| Short idle | Tentacles wiggle slowly, stares at nothing |
| Long idle | Curls up, wings fold, ZZZ with eldritch glow |
| Attention behavior | Holds up FREE HUGS sign; tentacles reach toward screen; eerie glow pulses |

### Reactions
| Event | Reaction |
|---|---|
| IMU_TAP | Tentacles flail wildly, confused |
| IMU_SHAKE | Full panic — tentacles everywhere, eyes spin |
| IMU_PICKUP | Clings with all tentacles, wings flap |
| IMU_WALKING | Sways, tentacles drag, seems content |
| MIC_NOISE_SOFT | Eyes glow, absorbs the sound cosmically |
| MIC_NOISE_LOUD | Eldritch power surge — glows green, then collapses |
| BTN_A | HUGS — reaches out with tentacles, happy |
| BTN_B | Startled, eyes wide, then offers cube artifact |

---

## JACK

**Files:** `jack1.png`, `jack2.png`
**Type:** `CreatureType::JACK`

### Visual
Round mossy stone with a dark fedora/bowler hat. Tiny dot eyes. Subtle smile.
Small patches of green moss. Occasionally wears a scarf. Carries a lantern sometimes.
Has a small wooden sign that says "JACK". Looks absolutely unimpressed at all times.

### Personality
The whole joke is that Jack does almost nothing. Stoic to the point of comedy.
Extremely rare reactions. When he does react, it's minimal — a single slow blink,
a tiny heart that appears briefly, then nothing again. Ancient, patient, unbothered.
Deadpan humor. His "excited" state is barely distinguishable from his "bored" state.

### Behaviour Profile
| Parameter | Value |
|---|---|
| Attention threshold | 12 min (Jack doesn't care) |
| IMU sensitivity | Very low (stones don't scare easily) |
| Mic sensitivity | Near zero (has heard worse, probably) |
| Short idle | Nothing. He just sits there. Hat slightly tilted maybe. |
| Long idle | Nothing. Still there. ZZZ barely visible. |
| Attention behavior | After 12 min: one slow blink at user. That's it. |

### Reactions
| Event | Reaction |
|---|---|
| IMU_TAP | Nothing. A beat. Then one slow blink. |
| IMU_SHAKE | Slides slightly, hat tilts, ANNOYED for 500ms, then stoic again |
| IMU_PICKUP | A tiny "!" that disappears immediately |
| IMU_WALKING | Hat bobs slightly. Stone content. |
| MIC_NOISE_SOFT | Nothing |
| MIC_NOISE_LOUD | Hat lifts 2px. Returns. Silence. |
| BTN_A | Tiny heart appears. Gone in 1 second. |
| BTN_B | Nothing. He saw that. He chose not to engage. |

---

## LUMI

**Files:** `lumi1.png`, `lumi2.png`
**Type:** `CreatureType::LUMI`

### Visual
Small deer-fox fae creature. Fluffy lavender/purple fur all over.
Small branching antlers that glow. Large expressive violet eyes.
Big rounded ears, bushy tail. Gold star pendant necklace.
Sparkle dots on fur. Feminine and magical.

### Personality
The most expressive and reactive of all four. Gets excited easily, scared easily,
happy easily. Tail wags constantly. Makes small magical sparkles appear.
Genuinely loves attention and will not let you ignore her.
The fastest to seek attention, the loudest about it.

### Behaviour Profile
| Parameter | Value |
|---|---|
| Attention threshold | 3 min (Lumi needs to be seen) |
| IMU sensitivity | High (very alert, soft movements register) |
| Mic sensitivity | Medium (reacts to voices more than noise) |
| Short idle | Tail wag, sparkle, looks around with big eyes |
| Long idle | Curls into a ball, antlers dim, soft glow remains |
| Attention behavior | Bounces to screen edge; sparkles everywhere; antlers flash; makes puppy eyes |

### Reactions
| Event | Reaction |
|---|---|
| IMU_TAP | Jumps, big eyes, then EXCITED |
| IMU_SHAKE | SCARED — curls up, tail hides |
| IMU_PICKUP | Instant joy — tail spins, sparkles burst |
| IMU_WALKING | Happy bounce rhythm, matches pace |
| MIC_NOISE_SOFT | Ears perk, listens intently, tilts head |
| MIC_NOISE_LOUD | Jumps, STARTLED, then indignant look |
| BTN_A | Nuzzles — sparkle heart, antlers glow warm |
| BTN_B | Surprised then playful — paws at you |

---

## Personality Comparison

| | Bytee | Cthulhu | Jack | Lumi |
|---|---|---|---|---|
| Reactivity | Medium | Medium-High | Extremely Low | Very High |
| Attention need | 4 min | 5 min | 12 min | 3 min |
| Idle expressiveness | Medium | Medium | Basically none | High |
| Mic sensitivity | Med-High | High | None | Medium |
| IMU sensitivity | Medium | Medium | Very Low | High |
| Vibe | Wonder/Magic | Eldritch chaos | Cosmic indifference | Fae love |
| Comedy | Earnest magic | Horrifying cuteness | The silence IS the joke | Overdramatic |
