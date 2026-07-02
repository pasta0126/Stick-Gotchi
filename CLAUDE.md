# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Stick-Gotchi is a minimalist multi-tool launcher firmware for the **M5Stick C Plus2** (ESP32-based). There is no virtual pet — the home screen is a horizontal, circular **carousel of tiles** (icon + label); selecting a tile launches a self-contained mini-app (Coin Flip, Magic 8-Ball, IMU demos). No phone app, no persistence of care/stats — this is a pocket toy, not a companion.

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
| Display | 135×240 TFT, landscape via `setRotation(1)` |
| IMU | MPU6886 — accessed via `M5.Imu.getAccel()` / `getGyro()` |
| Buttons | A (front, upper side), B (top), C / Power (lower side, `M5.BtnPWR`) |
| LED | Single red LED (no RGB) |

Physical layout: B is the button on top of the device; A and C are stacked on one side, A above C.

## Architecture

```
main.cpp
  ├── ButtonManager   — polls BtnA/BtnB only, fires InputEvent (short/long press)
  ├── DisplayManager  — FreeRTOS mutex + M5Canvas framebuffer (240×135)
  ├── AppManager      — owns one active AppBase*, routes input callbacks
  ├── handlePowerButton() — standalone short/long detection on M5.BtnPWR (reboot/power off)
  └── Apps (static instances, no heap alloc)
       ├── CarouselHome   — root launcher, cyclic tile carousel
       ├── CoinFlipApp    — FlipCoinGame + sprite draw
       ├── Magic8BallApp  — Magic8BallGame + IMU shake polling + sprite draw
       └── ImuDemoApp     — Accelerometer / Gyroscope / Orientation (3 tiles, one instance)
```

### App lifecycle

Every app implements `AppBase`:
- `init()` — called on launch; reset per-launch state
- `update(deltaMs)` — called every loop tick when foreground
- `suspend()` / `resume()` — called if another mechanism pauses this app (currently unused, no overlay exists)
- `destroy()` — called when another app replaces this one
- `onInput(event)` — receives ButtonA/B events; return `true` to consume
- `setHomeCallback(fn)` — apps other than `CarouselHome` call this on Btn B long to return home

### Button routing

- **Button C / Power** (`M5.BtnPWR`) is handled entirely in `main.cpp::handlePowerButton()`, outside `ButtonManager` and outside `AppManager` — short press reboots (`ESP.restart()`), long press (700ms) powers off (`M5.Power.powerOff()`). It never reaches any app.
- **Buttons A/B** flow through `ButtonManager` → single callback slot → `AppManager::_current->onInput()`.
- `CarouselHome`: Btn B short advances the carousel, Btn A short launches the centered tile, Btn B long is a no-op (Home has no level above it).
- Every other app: Btn B long calls `_homeCallback()` → `AppManager::launchApp(&carouselHome)`.

### Display mutex

`DisplayManager::acquire()` / `release()` must bracket every draw sequence. No app currently runs its own FreeRTOS render task — all apps draw inline from `update(deltaMs)`, throttled to ~30fps via an accumulated `deltaMs` counter (see `ImuDemoApp`, `CoinFlipApp`, `Magic8BallApp`).

### Adding a new app

1. Create `src/apps/myapp/MyApp.h/.cpp` inheriting `AppBase`.
2. Add `static MyApp myApp;` in `main.cpp`, call `myApp.inject(&display)`.
3. Wire `myApp.setHomeCallback([]() { apps.launchApp(&carouselHome); });`.
4. Add `carouselHome.addTile({ "My App", myIconFn, []() -> AppBase* { return &myApp; } });`.
5. No changes needed to any core file — new apps are just new tiles.
