# Skyrim's Dogma design

## Authority and scope

Skyrim owns the world, collision, actors, rendering, quests, saves, and player
position. Dragon's Dogma: Dark Arisen is a reference for mechanics. It is not a
runtime dependency and does not run alongside this mod. We will write original
code and use original or permission-cleared assets.

The first supported host is Steam Skyrim SE 1.7.104.0 with SKSE 2.3.1 and Address
Library v13. Other executable versions must be rejected before game access.
CommonLibSSE-NG is pinned to alandtse's v11.0.0 commit
`94faaed0c60eddd8347767f2d4d29a97c93bde8c`, which includes the current layouts and
Address Library format 5 support.

## First development step: observation before control

Build a small SKSE DLL that logs its version, runtime, lifecycle events, and
player movement/stamina samples. Run a separate, deterministic movement/stamina
model beside the observed data. Its output is a prediction only: this build must
not write actor values, change speed, patch movement, inject inputs, register
save data, or require a content plugin. This makes the initial game test a check
of loading, data access, lifecycle handling, and instrumentation.

The model represents unavailable, ready, sprinting, and exhausted states. It
uses configurable stamina drain/recovery and an exhausted recovery threshold.
Its parameters are experimental, not recovered DDDA constants. Suspend it in
menus and unsupported player states; reset on new/load game and state changes
that invalidate a continuous sample. Measure horizontal displacement separately
from the observed sprint/movement flags; do not mistake cell changes or loading
for movement. The observer does not capture requested button inputs. Its model
uses wall-clock seconds and samples flags at intervals; it is not frame-accurate
input telemetry and does not account for game time dilation.

The adapter accesses game objects only on the game thread. Prefer SKSE's task
and messaging interfaces to executable patches for this first step. Throttle
samples and logs, limit outstanding work, and release observation state when
the player is unavailable. Never retain actor pointers across samples.

## Components

- `src/core`: engine-independent state rules and configuration validation.
- `src/plugin`: SKSE entry point, lifecycle adapter, sampling, and logging.
- `tests`: deterministic checks of transitions, timestep behavior, suspension,
  and invalid inputs. These do not prove game compatibility.
- `scripts`: reproducible builds and packaging, confined to this project.
- `config`: shipped plugin settings. Observation is the only initial mode.
- Root `AGENTS.md`, `DESIGN.md`, and `STATUS.md`: rules, design, and current status.
- `docs`: reconnaissance, change history, workflow, and human playtests.
- Ignored local directories hold tool downloads, dependencies, builds, logs,
  symbols, and packages. No game assets or extracted game files enter Git.

## Build and delivery

Use MSVC, CMake, and pinned dependencies. Compile the DLL for x64 and run the
engine-independent tests. Produce a Vortex-compatible ZIP containing
`SKSE/Plugins/SkyrimsDogma.dll` and its settings file. Keep symbols separately.
Do not deploy into either game installation automatically. The user installs
the package and performs the game tests; we read logs directly on this PC
afterward. No log upload is needed for this local workflow.

The validated core toolchain is Build Tools 17.14.41 / MSVC 19.44.35229.0,
Windows SDK 10.0.26100.0, and CMake 3.31.6-msvc6. Build CommonLib from the pinned
source when its prebuilt uses a newer incompatible compiler. Full DLL/package
results remain in [STATUS.md](STATUS.md), separate from core model tests.

Do not commit or push without a user request. Keep the source whitelist in
`.gitignore`. Any future distribution must carry dependency notices and source
license terms compatible with the pinned CommonLib version.

## Later milestones

The next narrow change is observer 0.1.1: retain model continuity across bounded
movement between exterior cells in the same worldspace. Interior/worldspace
transitions and time/position/capacity discontinuities still reset it. A pure
continuity classifier provides repeatable tests; copied identifiers and values
remain the only state held between samples. Every accepted boundary gets a
before/after model record. The implementation plan is in
[docs/DESIGN.md](docs/DESIGN.md). This remains observation-only.

1. Confirm the observer loads and samples safely on the user's runtime.
2. Measure DDDA movement and stamina. Introduce one opt-in, reversible Skyrim
   control change, prove restoration, then implement the movement state loop.
3. Integrate directional movement and animation frameworks after verifying each
   exact runtime-compatible release. Assign one owner to each movement field.
4. Add one melee weapon, its hit detection, and one explicit skill before a
   broader vocation system.
5. Add hit reactions and recovery with limits imposed by Skyrim's animation
   graphs and character controller.
6. Add one companion's tactical behavior using Skyrim actors and navigation.
   Expand toward a Pawn-inspired system only after telemetry proves it stable.

Papyrus handles slow orchestration, progression, records, menus, and companion
setup. Native C++ handles timing-sensitive state, input arbitration, telemetry,
and eventually combat/AI decisions. Neither layer bypasses Skyrim's collision
or save ownership.

## Acceptance of this first step

Automated: clean configure/build, deterministic tests, correct x64 DLL exports,
source-only Git inventory, and valid package layout. Human: successful SKSE
load, samples while moving, suspension in menus, reset across reloads, and no
gameplay change. The first user session met these criteria within Whiterun and
Dragonsreach; see [evidence and limits](docs/PLAYTEST-2026-10-05.md). Mounting,
overworld travel, and long-term stability are **not tested**. Ordinary cell
boundary resets interrupt the model and must be refined before stamina control.
The first step is not a recreation of DDDA locomotion.
