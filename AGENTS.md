# Repository Guidelines

## Project Structure & Module Organization

Stick-Gotchi is PlatformIO firmware for the M5Stick C Plus2. Core firmware lives in `src/`, with `src/main.cpp` wiring app, display, buttons, BLE, and menu singletons. Shared systems are under `src/core/`, BLE under `src/ble/`, menu UI under `src/menu/`, pet state under `src/gotchi/`, and apps under `src/apps/` such as `gotchi`, `stats`, and `imudemo`. Generated sprite headers live in `src/generated/`; avoid hand-editing them. Pixel art sources, exports, palettes, and helper scripts live in `art/`. Design notes are in `docs/GDD.md` and `docs/TECH.md`.

## Build, Test, and Development Commands

- `pio run` — compile the firmware using `platformio.ini`.
- `pio run --target upload` — flash the configured board on `COM4`.
- `pio device monitor` — open the serial monitor at `115200` baud.
- `pio run --target upload && pio device monitor` — flash and immediately inspect runtime logs.
- `python tools/spr2c.py ...` — convert exported sprites into C headers.

## Coding Style & Naming Conventions

Use C++ for firmware modules, with paired `.h` and `.cpp` files for reusable classes. Match the existing style: 4-space indentation, braces on the same line, `PascalCase` for classes and app types, `camelCase` for methods/functions, and private members prefixed with `_` where already used. Prefer static allocation and explicit lifecycle methods (`init`, `update`, `suspend`, `resume`, `destroy`) over heap-heavy patterns. Keep includes local and specific.

## Testing Guidelines

There is no dedicated automated test suite yet. Treat `pio run` as the required validation gate before submitting code. For hardware-dependent changes, upload to an M5Stick C Plus2 and verify relevant button input, display rendering, serial output, IMU/mic behavior, and BLE behavior. Document unverified hardware paths in the pull request.

## Commit & Pull Request Guidelines

Recent commits use short, descriptive messages in Spanish or English, often focused on the feature or fix, for example `nuevos sprites` or `Magic 8-ball minigame: shake-to-reveal con ease-out`. Keep commits scoped and imperative when practical. Pull requests should include a concise summary, changed areas (`src/apps/gotchi`, `art/export`, etc.), validation performed, linked issue/task when available, and screenshots or photos for UI or sprite changes.

## Security & Configuration Tips

Do not commit local device secrets, private task-tracker tokens, or machine-specific overrides. `platformio.ini` currently targets `COM4`; if your device uses another port, prefer local workflow notes or temporary changes rather than unrelated commits.

## Code Structure Patterns

All applications inherit from `AppBase` and implement the required lifecycle methods (`init`, `update`, `suspend`, `resume`, `destroy`). Applications should handle input through the `onInput` method, returning `true` to consume events. Display access is managed by `DisplayManager` which uses a mutex to coordinate access between the main thread and FreeRTOS tasks.

## File Organization

- `src/core/` - Core framework components (`AppManager`, `DisplayManager`, `ButtonManager`)
- `src/apps/` - Application implementations (`gotchi`, `stats`, `imudemo`)
- `src/ble/` - Bluetooth Low Energy services
- `src/generated/` - Auto-generated sprite headers
- `src/menu/` - Menu overlay system
- `art/` - Pixel art sources and export tools
- `tools/` - Helper scripts like `spr2c.py` for sprite conversion
- `docs/` - Project documentation (`GDD.md`, `TECH.md`)

## Development Workflow

1. Create or modify Aseprite artwork in `art/sprites/`
2. Export PNG+JSON from Aseprite
3. Run `python tools/spr2c.py art/sprites/gotchi/egg.json` to generate C headers
4. Include generated headers in `src/generated/` in your app code
5. Use `GotchiRenderer` to draw sprites with dynamic palette mapping
5. Test changes with `pio run` and `pio run --target upload`
6. Commit with a clear message following the project's naming conventions