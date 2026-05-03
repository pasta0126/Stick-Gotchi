# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Stick-Gotchi is a self-contained Tamagotchi-style virtual pet firmware for the **M5Stick C Plus2** (ESP32-based). The device IS the gotchi's world — no phone app required. The gotchi lives, grows, evolves, and dies entirely on the device. It features pixel art visuals generated from a deterministic seed, a lineage/inheritance system across generations, and environmental interaction via microphone and IMU.

Full design spec: `docs/GDD.md` | Technical architecture: `docs/TECH.md`

## Plane Workflow (task management)

Project: **SGOTCHI** — `https://plane.northernarchive.com/northern-archive/projects/6444bf4f-f5fe-4ef7-b865-6fd6f39adfd0/issues/`
User: **pasta0126** (ID: `2be9cbeb-4b1b-42aa-8a9c-1c3028797d95`)

### Rules for agents

1. **On session start**: check Plane for issues in `In Progress`. Continue that work before picking anything new.
2. **Pick up work**: take the lowest `sequence_id` issue in `Todo`, move it to `In Progress`, assign label `agent:claude`, then implement it.
3. **On completion**: move issue to `Done`, leave a short comment summarizing what was done, then pick up the next `Todo`.
4. **On blocker**: move issue to `Blocked`, post a comment with `@pasta0126` describing exactly what is needed, then stop and wait.
5. **Work continuously** through all `Todo` issues while tokens allow. Do not stop between issues unless blocked.

### Key IDs

| State | ID |
|---|---|
| Backlog | `eef1f451-4d6d-4647-9168-1e7e09f42aa2` |
| Todo | `64c60ec4-0e26-4a0d-b2e6-17bc6719151a` |
| In Progress | `e17c7141-36f2-4c73-ba28-64eb852ddcc2` |
| Done | `d88ac65e-7c2d-4e4a-8786-f5303579d216` |
| Blocked | `78d6f783-406d-4e76-8b85-016d6520eeb1` |

| Label | ID |
|---|---|
| agent:claude | `0009d9b1-9f67-491f-8161-1c3b74b4e01e` |
| needs-review | `eb0c0887-3e90-486e-b738-940116781bb6` |

### Plane API helper (Python)

```python
import urllib.request, json
KEY = "plane_api_2192863085c44649836e6c9763b86c6d"
BASE = "https://plane.northernarchive.com/api/v1"
WS = "northern-archive"
PID = "6444bf4f-f5fe-4ef7-b865-6fd6f39adfd0"

def plane(method, path, body=None):
    data = json.dumps(body).encode() if body else None
    req = urllib.request.Request(f"{BASE}{path}", data=data, method=method,
          headers={"X-Api-Key": KEY, "Content-Type": "application/json"})
    with urllib.request.urlopen(req) as r:
        return json.loads(r.read())
```

## Build

Uses **PlatformIO**. From the repo root:

```bash
pio run                        # compile
pio run --target upload        # flash to device
pio device monitor             # serial monitor (115200 baud)
pio run --target upload && pio device monitor   # flash + monitor
```

Target board: `m5stick-c` (M5Unified detects Plus2 at runtime).

## Hardware (M5Stick C Plus2)

| Peripheral | Detail |
|---|---|
| Display | 135×240 TFT, landscape via `setRotation(3)` |
| IMU | MPU6886 — accessed via `M5.Imu.getAccel()` / `getGyro()` |
| Buttons | A (front), B (side), C (power — menu toggle) |
| LED | Single red LED (no RGB) |
| BLE | NimBLE-Arduino (lighter than ESP32 default BLE stack) |

## Architecture

```
main.cpp
  ├── ButtonManager   — polls BtnA/BtnB, fires InputEvent (short/long press)
  ├── DisplayManager  — FreeRTOS mutex + M5Canvas framebuffer (240×135)
  ├── AppManager      — owns one active AppBase*, routes input callbacks
  ├── MenuOverlay     — full-screen list UI, runs on main loop thread
  ├── BleService      — NimBLE GATT server (start/stop per app)
  └── Apps (static instances, no heap alloc)
       ├── GotchiApp  → GotchiPet + GotchiRenderer (FreeRTOS task, Core 0)
       │                + GotchiDNA + GotchiShake + GotchiAudio + GotchiSleep
       ├── StatsApp   → StatsRenderer (3 tabs: status, lineage, history)
       └── ImuDemoApp → reads IMU, draws bar chart
```

### New modules (v2)

| Module | Responsibility |
|---|---|
| `GotchiDNA` | GotchiID (64-bit), visual seed, lineage mutation |
| `GotchiLineage` | NVS persistence for state + 5 ancestors + heritage bonuses |
| `GotchiShake` | IMU delta-g detection → ShakeLevel enum |
| `GotchiAudio` | Mic RMS sampling (FreeRTOS task, Core 0) → noise events |
| `GotchiSleep` | Day/night schedule, sleep triggers, sleep protection |
| `GotchiSprites` | Pixel art sprite data (uint8 palette indices, per stage) |

### App lifecycle

Every app implements `AppBase`:
- `init()` — called on launch; start tasks, BLE, etc.
- `update(deltaMs)` — called every loop tick when foreground and not suspended
- `suspend()` / `resume()` — called when menu opens/closes over the app
- `destroy()` — called when another app replaces this one
- `onInput(event)` — receives ButtonA/B events; return `true` to consume

### Button routing

- **Button C** is captured in `loop()` directly — always toggles the menu.
- **Buttons A/B** flow through `ButtonManager` → single callback slot.
  - When menu closed: `AppManager` routes events to `currentApp->onInput()`.
  - When menu open: `MenuOverlay` steals the callback slot.
  - On menu close: `AppManager` re-registers its callback via `resumeCurrent()`.

### Display mutex

`DisplayManager::acquire()` / `release()` must bracket every draw sequence.  
`GotchiRenderer` runs as a FreeRTOS task on Core 0. `MenuOverlay` calls `vTaskSuspend` (via `AppManager::suspendCurrent`) before drawing, eliminating contention.

### Adding a new app

1. Create `src/apps/myapp/MyApp.h/.cpp` inheriting `AppBase`.
2. Add `static MyApp myApp;` in `main.cpp`.
3. Call `myApp.inject(...)` for dependencies.
4. Add `menu.addItem({ "My App", MenuItemType::APP, []() -> AppBase* { return &myApp; }, nullptr });`.
5. No changes needed to any core file.

## Button mapping (GotchiApp)

| Button | Gesture | Action |
|---|---|---|
| Btn B | Short | Cycle active icon in action bar |
| Btn A | Short | Execute selected action |
| Btn A | Long | Toggle action bar visibility |
| Btn C | Short | Open system menu |

## BLE Protocol (secondary — future gotchi-to-gotchi)

The device is self-contained. BLE is preserved for future gotchi-to-gotchi interaction but is not required for core gameplay.

| Direction | UUID suffix | Format |
|---|---|---|
| Device→App (NOTIFY) | `beb5483e…` | 7 bytes: `[mood, hunger, thirst, energy, steps_lo, steps_hi, flags]` |
| App→Device (CMD) | `6e400002…` | 1 byte: `0x01`=feed, `0x02`=drink, `0x03`=pet, `0x04`=play |
| App→Device (BAT) | `6e400003…` | 2 bytes: `[level, charging]` |
| App→Device (CTX) | `6e400004…` | 2 bytes: `[hour, tempC]` |

Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c3319100`

## Gotchi moods

Mood byte values (must stay in sync with Android app):
`NEUTRAL=0, HAPPY=1, SICK=2, PENSIVE=3, SAD=4, SLEEPING=5, EXCITED=6, LAUGHING=7, DIZZY=8, ANNOYED=9, ANGRY=10, STARTLED=11, SCARED=12`

> `DEFAULT` fue renombrado a `NEUTRAL` porque `Arduino.h` define `#define DEFAULT 1` lo que rompe el enum.

Priority (highest first): `SCARED > SICK > SAD > PENSIVE/SLEEPING > HAPPY > DEFAULT`.  
Temporary moods (from pet/shake/play) override base mood for a timed duration.
