# Stick-Gotchi

Minimalist multi-tool launcher firmware for the M5Stick C Plus2. The home screen is a horizontal, circular carousel of tiles; picking one launches a self-contained mini-app (Coin Flip, Magic 8-Ball, IMU demos). No virtual pet, no phone app, no persistence — a pocket toy, not a companion.

## Requirements

- [PlatformIO](https://platformio.org/) (CLI or IDE extension)
- An M5Stick C Plus2

## Build & flash

```bash
pio run                                          # compile
pio run --target upload                          # flash
pio device monitor                                # serial monitor (115200 baud)
pio run --target upload && pio device monitor     # flash + monitor
```

## Validating changes

```bash
./scripts/verify.sh
```

This compiles the firmware. There is no automated test suite (embedded firmware, no test framework) — hardware-observable behavior (buttons, display, IMU, LED) needs manual verification on-device.

## Documentation

- `docs/GDD.md` — design spec (what the product is)
- `docs/TECH.md` — technical architecture
- `docs/decisions/` — architecture decision records
- `AGENTS.md` — instructions for coding agents (architecture map, workflow, commands)
- `CLAUDE.md` — Claude Code specific behavior and the Plane task-tracking workflow
- `specs/` — versioned spec/plan/tasks detail for non-trivial features

## Status

Actively developed; current architecture is the carousel launcher (see `docs/TECH.md`). Task tracking lives in Plane (project SGOTCHI).
