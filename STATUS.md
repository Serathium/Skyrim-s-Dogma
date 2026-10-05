# Current status

Updated: 2026-10-05.

## Goal and ownership

Reimplement DDDA-inspired locomotion, movement/stamina states, melee, skills/
vocations, reactions, and eventually Pawn-like companion AI inside Skyrim.
Skyrim owns actors, position, physics, rendering, quests, and saves. Dragon's
Dogma is a reference only; there is no passthrough or live inter-game connection.

## Current step

Build an observer SKSE DLL and a deterministic experimental stamina model. The
package must not change movement, actor values, input, or save data. Development
was authorized after the reconnaissance report was completed.

The observer 0.1.1, INI, and pure model are built. The model passed **5,512 checks**
and the continuity tests passed **107 checks**, all with zero failures (CTest
**2/2 passed**). Full 0.1.1 DLL compilation and binary metadata checks passed.
The user completed the first **0.1.0** playtest in
Whiterun/Dragonsreach with unchanged gameplay reported. Logs confirm loading,
movement/stamina samples, pauses, and successful reload. The first observer
milestone is accepted within that scope; see
[session evidence](docs/PLAYTEST-2026-10-05.md). Horse mounting, overworld travel,
and long-term stability are **not tested**. Version 0.1.1 now preserves prediction
across bounded movement between exterior cells sharing a nonzero worldspace.
Interior/worldspace changes, loading, unsupported states, sample gaps, capacity
changes, and three-dimensional position discontinuities still reset it.
**0.1.1 in-game behavior is not tested.** The old 0.1.0 package is retained;
the new package is `dist/SkyrimsDogma-0.1.1-observer.zip`.

Latest DLL: 1,559,552 bytes, SHA-256
`8CA5C3504450F9F3D93160F903ABE595EE56C0D8A53E67EFB916823FE1305A94`.
Metadata verification confirms version 0.1.1 and the same exact runtime/SKSE
requirements. Package validation passed: 29 observer-package entries, matching
embedded DLL/manifest, portable paths, and 34 source/documentation entries in
the separate source ZIP. The 0.1.0 ZIP hash is unchanged. Checksums use
`dist/SHA256SUMS.txt`.

## Verified setup

- Repository: `C:\Users\samue\Documents\Skyrim's Dogma`.
- Skyrim: Steam app 489830, build 24914197, executable **1.7.104.0**, x64.
- Skyrim location: `C:\Program Files (x86)\Steam\steamapps\common\Skyrim Special Edition`.
- SKSE **2.3.1**; Address Library **v13**, format-5 database for 1.7.104.
- Vortex **2.7.2**; prior logs show Engine Fixes 7.0.21 and Priority 3.4.0 loaded.
- DDDA: Steam app 367500, build **2364871**, generic version 1.0.0.0, x86.
- DDDA location: `C:\Program Files (x86)\Steam\steamapps\common\DDDA`.
- CommonLibSSE-NG v11.0.0 commit `94faaed0c60eddd8347767f2d4d29a97c93bde8c`.
- Visual Studio Build Tools **17.14.41**, MSVC **19.44.35229.0**, Windows SDK
  **10.0.26100.0**, CMake **3.31.6-msvc6**.

Full game fingerprints and evidence are in [RECONNAISSANCE.md](docs/RECONNAISSANCE.md).

## Established evidence from the 0.1.0 baseline

- Read-only reconnaissance completed and documented.
- Prior SKSE logs show operation before this project's DLL exists.
- Pinned CommonLib source supports this runtime and Address Library format.
- `scripts/check-runtime.ps1` passed: Skyrim **1.7.104.0**, SKSE loader file
  version **0,2,3,1**, Address Library format **5**, and the same executable
  SHA-256 recorded during reconnaissance. This is a file check, not a live load.
- `scripts/build.ps1 -CoreOnly` passed: **5,512 model checks**, **0 failures**,
  and **1/1 CTest tests**. This tests state rules outside Skyrim.
- Project design and initial human test procedure are written.
- Full `scripts/build.ps1` completed successfully, including CTest 1/1.
- `scripts/verify-artifact.ps1` verified an x64 DLL exporting
  `SKSEPlugin_Load`, `SKSEPlugin_Query`, and `SKSEPlugin_Version`; metadata
  identifies plugin 0.1.0, runtime 1.7.104.0 only, and minimum SKSE 2.3.1.
  Dependency inspection found no required third-party runtime DLLs.
- The DLL is 1,556,992 bytes. SHA-256:
  `1967D90AE456873F71B00F82EB47DB7FA2E1B821484A7EA8D33A255727C3076D`.
- Installable ZIP layout and its embedded DLL hash passed inspection. The
  separate source ZIP contains only source/documentation, not game data or
  binaries. Package hashes are in `dist/SHA256SUMS.txt`; do not install the
  source ZIP or PDB (debugging symbols).
- The source separates game-thread observations from the independent model.
  The INI defaults sample at 100 ms and log at 1,000 ms. Actor sprint/movement
  flags are observed; button presses are not captured. No gameplay-write or
  save-registration API is used by the observer implementation.

The first live session now establishes loading on this runtime: SKSE reports
`loaded correctly (handle 3)`. The observer wrote 231 sample records, including
movement, sprint, native stamina 0–100, and model exhaustion. Two successful
loads and menu/camera/airborne/swimming/furniture pauses are recorded. Logged
sampling cost median is 23.5 microseconds; the first sample took 7,946.1
microseconds. This excludes sample-line logging and does not prove frame-time
impact. No observer warnings/errors were found; no crash was reported.

Twenty cell-change resets occurred during the 0.1.0 city test. They are consistent
with the conservative design but interrupt stamina-model continuity. Address
ordinary boundary continuity before introducing stamina control; do not confuse
cell changes with loading or teleports.

## Attempts and resolved build issues

1. First Build Tools installation returned **1602** (canceled). The second
   attempt installed **17.14.41** successfully.
2. The available CommonLib prebuilt uses **MSVC 19.51**, newer than the installed
   **19.44** toolchain. The build selected the pinned library's source fallback
   instead of mixing incompatible compiler/library versions.
3. Source compilation was interrupted by session usage resets. It subsequently
   completed; those interruptions were not compiler failures.
4. The full build exposed **MSB4100**: the apostrophe in this project's absolute
   path reached the generated OpenVR library link expression. An imported target
   attempt failed. Linking by the `openvr_api` basename with a build-only library
   search directory succeeded, including CMake export generation.
5. Binary validation caught fifteen unused runtime entries advertised as 1.0.0
   by an upstream metadata helper. Explicit zero-initialized SKSE version data
   fixed the list. Validation now confirms only 1.7.104.0 is advertised.
6. Initial ZIP creation emitted backslash paths. Explicit portable ZIP creation
   uses forward slashes; loading both required .NET compression assemblies
   resolved its first PowerShell 5 attempt. Package validation passed.

## Next steps

1. User installs 0.1.1 with only that observer version enabled and follows
   [the boundary playtest](docs/PLAYTEST-0.1.1.md). Check crossings on the same
   Whiterun route, Dragonsreach, menus, and reload.
2. Agent analyzes crossing before/after model values and safe-reset evidence.
   Actual short-teleport detection remains uncertain: bounded displacement alone
   cannot distinguish it from walking. No gameplay-control change yet.
3. After continuity is verified, measure a narrow DDDA movement/stamina loop and document what its parameters
   mean before selecting gameplay constants.
4. Introduce one opt-in, reversible gameplay change under the documented design.
   Carry mounting and overworld coverage into the next relevant user test.

From the project folder, the intended setup/build commands are:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\setup.ps1 -InstallBuildTools
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

Already-verified isolated checks:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -CoreOnly
powershell -ExecutionPolicy Bypass -File .\scripts\check-runtime.ps1
```

The expected results are a passing model test and the recorded runtime/loader/
database values. Neither command launches a game.

## Constraints and handoff

Read `AGENTS.md` and `DESIGN.md`. Keep `.gitignore` as a source whitelist. Do not
write into game installations or add proprietary assets, extracted payloads,
decompiled code, or credentials to Git. Do not commit or push until requested.
The user performs gameplay testing.

[RECONNAISSANCE.md](docs/RECONNAISSANCE.md) contains evidence and assumptions;
[DESIGN.md](DESIGN.md) defines responsibility boundaries;
[PLAYTEST.md](docs/PLAYTEST.md) gives the user procedure;
[MODLOG.md](docs/MODLOG.md) retains changes and attempts. The user moved
`AGENTS.md`, `DESIGN.md`, and `STATUS.md` to the repository root; preserve that
organization. Work is on `feature/observer-prototype`, uncommitted and unstaged.
No game was launched or modified by the agent. Inspect the actual working tree
and preserve any newer results.
