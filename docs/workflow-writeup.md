# How Skyrim's Dogma is being developed

This is a development record, not a completed-mod showcase. The first
implementation is an observer; gameplay features are still ahead. Actual build
and playtest outcomes belong in [MODLOG.md](MODLOG.md) and [STATUS.md](../STATUS.md).

## Responsibilities

The user plays and reports. The agent implements, instruments, builds, packages,
documents, and analyzes logs. Skyrim is authoritative for all game state.
Dragon's Dogma: Dark Arisen is a behavioral reference, not a second runtime.

The first target is Steam Skyrim 1.7.104.0 / SKSE 2.3.1 / Address Library v13.
DDDA's inspected Steam build is 2364871. Executable fingerprints and the installed
mod inventory are preserved in `RECONNAISSANCE.md`.

## Findings that shaped the plan

- Runtime matching comes first. This Skyrim needs current layouts and Address
  Library format-5 support, not an arbitrary older tutorial dependency.
- Frameworks can supply animation, targeting, and hit detection, but each exact
  release and its dependency chain need validation.
- DDDA archive/XFS metadata can answer questions before executable analysis.
  Field names alone do not establish units or semantics.
- Existing DDDA modifications make a controlled measurement baseline necessary.
- Observation establishes loading, lifecycle, and sampling before gameplay
  changes make integration failures harder to diagnose.

## Working method

The [AI Game Modding Guides](https://github.com/trevaintdead/ai-game-modding-guides)
inform design before code, small milestones, telemetry, failure records, and
explicit human tests. The project keeps:

- Root [AGENTS.md](../AGENTS.md): rules and working boundaries.
- Root [DESIGN.md](../DESIGN.md): responsibilities, scope, and acceptance criteria.
- [RECONNAISSANCE.md](RECONNAISSANCE.md): evidence and unresolved assumptions.
- [MODLOG.md](MODLOG.md): changes, tests, results, and failed attempts.
- Root [STATUS.md](../STATUS.md): the current handoff state.
- [PLAYTEST.md](PLAYTEST.md): exact user actions and logs the agent reads locally.

Pure state rules can be tested outside Skyrim. Those tests cannot prove engine
compatibility. Binary/package checks prove build properties; a game session is
still required to establish loading and runtime behavior.

The first model build passed 5,512 checks with no failures (CTest 1/1) using
MSVC 19.44.35229.0. The read-only runtime check, full DLL build, binary metadata
checks, and package validation passed. The first user session in Whiterun and
Dragonsreach confirmed plugin loading, movement/stamina observation, pauses,
and reload. The user reported unchanged gameplay. Horse mounting, overworld
travel, and long-term stability are **not tested**. The
[session report](PLAYTEST-2026-10-05.md) records evidence and the cell-boundary
reset limitation to address before gameplay control.

Observer 0.1.1 addresses ordinary exterior-cell continuity with a pure classifier
and before/after crossing telemetry. The full build passed both CTest groups:
5,512 model checks and 107 continuity checks, zero failures. It resets on changed
coordinate spaces and unsafe sample timing/displacement, including vertical
movement. A bounded short teleport is still ambiguous. Version 0.1.1 is
**not tested in Skyrim**; [its focused test](PLAYTEST-0.1.1.md) is next.

The build record retains problems instead of hiding them: one canceled compiler
installation before a successful second attempt; a prebuilt CommonLib/compiler
version mismatch resolved by selecting source compilation; and an MSBuild path
problem caused by the apostrophe in the project folder. An imported-library
attempt failed; linking OpenVR by basename with a library search directory
succeeded. Binary inspection also caught unwanted default runtime entries in
the upstream metadata helper. Explicit zero-initialized SKSE version data fixed
that; the verified DLL advertises only 1.7.104.0 and minimum SKSE 2.3.1. ZIP
creation was adjusted for portable slash-separated paths. None required changing
either game installation.

The Git whitelist includes source and documentation. Dependencies, builds,
symbols, logs, and packages stay ignored. Proprietary assets, extracted payloads,
decompiled code, and credentials never enter the repository. The agent does not
commit or push until asked. Changes remain small enough to review and revert.

## What SkyCraft contributed

[SkyCraft](https://github.com/chasmlol/SkyCraft) was reviewed for written
responsibilities, milestone organization, and measured performance. Its
Minecraft-authoritative passthrough architecture does not fit this project.
Skyrim's Dogma uses one runtime, with no inter-game shared memory, sockets, or
file transport.

## Milestone sequence

1. Build/test an observer and have the user confirm a clean Skyrim session.
2. Measure a narrow DDDA movement/stamina loop and introduce one reversible
   Skyrim control change.
3. Integrate movement/animation with explicit ownership.
4. Implement one melee weapon and one vocation skill.
5. Add dependable reactions and recovery.
6. Develop one companion's tactical behavior before wider Pawn-like features.

The current milestone is not a playable DDDA recreation. It makes the next
change measurable and easier to diagnose.
