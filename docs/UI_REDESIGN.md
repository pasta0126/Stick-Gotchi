# Stick-Gotchi UI/UX Redesign

## Design Principle

> The creature IS the interface. Everything else gets out of the way.

The screen belongs to the companion. No stats bars, no action menus, no management chrome.
The gotchi's expression communicates everything. The user interacts through touch (buttons)
and presence (IMU/mic), not through menus.

---

## Screen Inventory

### 1. Main Companion Screen (default)

**What's on screen:**
- Creature occupies ~90% of the 240x135 display
- Subtle dynamic background (per-creature atmosphere, see below)
- Optional: tiny day/night icon, top-right corner (8x8px max)
- Optional: creature name, bottom-left, very small (only visible for 3s after wake-up)
- Nothing else

**What's NOT on screen:**
- No hunger/health/energy bars
- No action bar icons
- No mood label
- No stats of any kind

**Button mapping:**
```
Btn A short   →  Poke creature  (BTN_A event → GotchiBehaviour)
Btn A long    →  Open creature selector
Btn B short   →  Secondary touch (BTN_B event → GotchiBehaviour)
Btn B long    →  Mood peek overlay (3 seconds, then auto-dismiss)
Btn C short   →  Open system menu
```

**Mood peek overlay (Btn B long):**
- Semi-transparent dark panel slides up from bottom
- Shows: [mood icon] [mood name] · [time since last interaction]
- Example: "😌 Calm · 4 min ago"
- Auto-dismisses after 3 seconds, no button needed

---

### 2. System Menu

**Current:** full-screen vertical text list, hard cut in/out
**New:** same full-screen, but with personality

**Changes:**
- Background: blur/dim of the current companion view (don't erase the creature)
- Items: large icon cards in a grid (2 columns) instead of text list
- Color accent: matches the active creature's palette
- Entry animation: fade + scale-up (100ms)
- Exit animation: fade + scale-down (80ms)

**Menu items (new layout):**
```
[  Bytee/creature icon  ]  [  Mini-juegos icon  ]
[  Diario icon          ]  [  IMU Demo icon     ]
[  Reiniciar icon       ]
```

**Icon style:** Large (32x32px), simple pixel art, glows on selection

---

### 3. Creature Selector

**Triggered by:** Btn A long from main screen, or menu item

**Layout:**
```
┌─────────────────────────────────────────────────┐
│                                                 │
│         [  LARGE CREATURE PREVIEW  ]            │
│              (idle animation)                   │
│                                                 │
│              ◄  BYTEE  ►                        │
│        "Curious magic robot"                    │
│                                                 │
│         [A] Select    [B] Cycle                 │
└─────────────────────────────────────────────────┘
```

- Preview: ~64x64px creature sprite with idle animation running
- Name: large, centered
- Tagline: personality in 3 words
- Btn B: cycle through creatures (with slide transition)
- Btn A: confirm selection
- On selection: creature does a "hello" animation before entering main screen

**Creature taglines:**
| Creature | Tagline |
|---|---|
| Bytee | "Curious magic robot" |
| Cthulhu | "Just wants a hug" |
| Jack | "Basically never reacts" |
| Lumi | "Extremely emotional fae" |

---

### 4. Companion Diary (replaces StatsApp)

**Single screen, no tabs. Clean card layout.**

```
┌─────────────────────────────────────────────────┐
│  [creature icon]  LUMI                          │
│  ─────────────────────────────────────          │
│  Together      143 h 22 min                     │
│  Today         12 interactions                  │
│  Right now     [mood icon]  Playful             │
│  Last seen     3 min ago                        │
│  ─────────────────────────────────────          │
│  [A] Back                                       │
└─────────────────────────────────────────────────┘
```

- No hunger/health bars
- No lineage tree (not relevant to companion model)
- "Together" is the primary metric — time spent with the creature
- Mood shown as icon + word, not as a number
- Clean, warm typography, creature palette colors

---

### 5. Transitions & Animations

| Transition | Animation |
|---|---|
| Menu open | fade in + scale 0.85→1.0, 100ms |
| Menu close | fade out + scale 1.0→0.85, 80ms |
| Creature selector open | slide up from bottom, 150ms |
| Creature change | current creature: sleep anim → fade out; new: fade in → wake anim |
| Power on | black screen → creature appears with a "yawn/stretch" |
| Wake from sleep | creature stretches, blinks, looks around |
| Mood peek in | slide up 60ms |
| Mood peek out | auto-fade after 3s |

---

## Per-Creature Atmosphere

Each creature has a background that communicates their world without distracting.

| Creature | Background | Particles |
|---|---|---|
| Bytee | Deep space purple (#1a0a2e) | Small gold sparkles, occasional magic rune fades |
| Cthulhu | Deep ocean green (#0a1a0f) | Slow green mist swirls, occasional bubble |
| Jack | Stone grey (#1a1a1a) | Nothing. Rarely a rain drop. |
| Lumi | Twilight lavender (#1a0a2e→#2a1a3e) | Soft star twinkles, occasional flower petal |

Backgrounds are drawn behind the creature. Max 4 active particles at once (memory constraint).

---

## Typography & Visual Language

- **Font:** M5Stack default pixel font (already in use)
- **Primary text size:** 2 (medium, readable)
- **Secondary text size:** 1 (small details)
- **Colors:** Per-creature accent on white text, dark backgrounds
- **Icons:** 16x16 or 32x32 pixel art, consistent line weight
- **No gradients** — solid fills only (TFT performance + pixel art aesthetic)

---

## What Doesn't Change

- Display resolution and orientation (240x135 landscape)
- Btn C always opens system menu
- AppManager / MenuOverlay architecture (modify, don't replace)
- GotchiRenderer FreeRTOS task structure
- DisplayManager mutex pattern

---

## Implementation Order

1. Button remapping (Btn A/B new roles) — unblocks all interaction work
2. Remove action bar from GotchiRenderer — clears screen real estate
3. Per-creature backgrounds — visual impact, standalone change
4. Menu visual polish (icons, fade transitions)
5. Creature selector screen
6. Mood peek overlay (Btn B long)
7. Companion diary screen
8. Transition animations (last — polish layer)
