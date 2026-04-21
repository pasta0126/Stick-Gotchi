# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Stick-Gotchi is a Tamagotchi-style virtual pet firmware for the **M5Stick C Plus2** (ESP32-based). It features a multi-app menu system and a BLE companion protocol compatible with the Atom-Gotchi Android app.

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
       └── ImuDemoApp → reads IMU, draws bar chart
```

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

## BLE Protocol (matches Atom-Gotchi Android app)

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
