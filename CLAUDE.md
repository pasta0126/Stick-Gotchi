# CLAUDE.md

Read and follow `AGENTS.md` — it is the canonical, tool-neutral source for project purpose, architecture map, commands, and workflow rules. This file only adds behavior specific to Claude Code and the Plane integration.

## Claude-specific behavior

- Read `AGENTS.md`, `docs/GDD.md`, and `docs/TECH.md` before proposing structural changes.
- Use subagents (`Explore`, `general-purpose`) for open-ended repo exploration so it doesn't consume the main context window.
- Do not implement a non-trivial feature before its `specs/` entry (if one exists or is warranted per `AGENTS.md`) is clear.
- Review the final diff before reporting a task complete.
- Keep changes scoped to the selected Plane issue or task.

## Plane Workflow (task management)

Project: **SGOTCHI** — `https://plane.northernarchive.com/northern-archive/projects/6444bf4f-f5fe-4ef7-b865-6fd6f39adfd0/issues/`
User: **pasta0126** (ID: `2be9cbeb-4b1b-42aa-8a9c-1c3028797d95`)

Plane is the board of record for issue status and assignment. For non-trivial issues, pair the issue with a `specs/NNN-short-name/` folder (see `AGENTS.md` § Specs and Plane) so the technical detail is versioned in the repo, not only in a Plane comment.

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

## Hardware quick reference (M5Stick C Plus2)

| Peripheral | Detail |
|---|---|
| Display | 135×240 TFT, landscape via `setRotation(1)` |
| IMU | MPU6886 — accessed via `M5.Imu.getAccel()` / `getGyro()` |
| Buttons | A (front, upper side), B (top), C / Power (lower side, `M5.BtnPWR`) |
| LED | Single red LED (no RGB) |

Physical layout: B is the button on top of the device; A and C are stacked on one side, A above C.

Button C / Power is handled entirely in `main.cpp::handlePowerButton()`, outside `ButtonManager`/`AppManager` — short press reboots, long press (700ms) powers off. It never reaches an app. Buttons A/B flow through `ButtonManager` to the active app's `onInput()`.

## Adding a new app

1. Create `src/apps/myapp/MyApp.h/.cpp` inheriting `AppBase`.
2. Add `static MyApp myApp;` in `main.cpp`, call `myApp.inject(&display)`.
3. Wire `myApp.setHomeCallback([]() { apps.launchApp(&carouselHome); });`.
4. Add `carouselHome.addTile({ "My App", myIconFn, []() -> AppBase* { return &myApp; } });`.
5. No changes needed to any core file — new apps are just new tiles.
