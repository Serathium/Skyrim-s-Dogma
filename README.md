# Skyrim's Dogma

A Skyrim Special Edition mod that reimplements gameplay mechanics inspired by
Dragon's Dogma: Dark Arisen. Skyrim owns the world, movement, actors, rendering,
quests, and saves. Dragon's Dogma is a reference game, not a runtime dependency.

## Current milestone

Version **0.1.1-observer** samples Skyrim movement/stamina and calculates an
experimental stamina model beside those samples. It does not change gameplay.
This provides a measurable baseline before movement control is introduced.

The new observer preserves its model across bounded movement between exterior
cells in the same worldspace. Interior transitions, menus, loading, and position/
time discontinuities still reset it. This change is **not tested in Skyrim**;
follow the [focused 0.1.1 test](docs/PLAYTEST-0.1.1.md).

The model passed **5,512 checks** and the new continuity tests passed **107
checks**, all with zero failures (CTest **2/2 passed**). The installed runtime
check and full 0.1.1 DLL build/binary metadata checks passed. Package validation
is recorded in [STATUS.md](STATUS.md). The first **0.1.0** Whiterun/Dragonsreach session
confirmed loading, samples, pause/resume, and reload; the user reported unchanged
gameplay. See [the results and limits](docs/PLAYTEST-2026-10-05.md). Horse mounting,
overworld travel, and long-term stability are **not tested**.

Not implemented: DDDA locomotion, stamina control, melee combat, weapon skills,
vocations, new hit reactions, and Pawn AI. Model numbers are experimental; they
are not recovered Dragon's Dogma constants.

## Requirements for the first target

- Steam Skyrim SE **1.7.104.0**, x64.
- [SKSE **2.3.1**](https://skse.silverlock.org/).
- [Address Library **v13**, including the 1.7.104.0 database](https://www.nexusmods.com/skyrimspecialedition/mods/32444).
- Windows; single-player and offline use.

Other Skyrim versions are outside this build's supported target. No Dragon's
Dogma installation, animation framework, Papyrus script, or ESP/ESM is required
by the observer.

## Install and playtest

The current package is `dist/SkyrimsDogma-0.1.1-observer.zip`; see
[current status](STATUS.md) and the [0.1.1 procedure](docs/PLAYTEST-0.1.1.md).

1. Close Skyrim. In Vortex, select Skyrim Special Edition and use **Install From
   File** to select the ZIP.
2. Enable Skyrim's Dogma and deploy. Its files belong in the game's Data folder:
   `SKSE/Plugins/SkyrimsDogma.dll` and `SKSE/Plugins/SkyrimsDogma.ini`.
3. Start through the existing SKSE launcher and follow [the focused test](docs/PLAYTEST-0.1.1.md).
4. Close the game and report the requested observations. I can read the logs
   directly on this PC; uploading them is unnecessary.

To remove the observer, disable it in Vortex and deploy while Skyrim is closed.
It adds no save data. Keep the package for reproducing a test.

## Build from source

The setup script can install Microsoft C++ Build Tools when requested. It pins
CommonLibSSE-NG and vcpkg beneath ignored project directories. The installed
toolchain is Visual Studio Build Tools **17.14.41**, MSVC **19.44.35229.0**,
Windows SDK **10.0.26100.0**, and CMake **3.31.6-msvc6**. CommonLib was built
from pinned source because its available prebuilt library uses newer MSVC 19.51.
From the project folder, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\setup.ps1 -InstallBuildTools
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

The engine-independent tests can be built with
`powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -CoreOnly`.
Both the isolated tests and full plugin build passed. Packaging checks the x64
DLL exports and exact runtime metadata without loading the DLL. Read-only
installation checks use
`powershell -ExecutionPolicy Bypass -File .\scripts\check-runtime.ps1`.
These commands do not launch a game or deploy into either installation.

## Project documents

- [Reconnaissance and evidence](docs/RECONNAISSANCE.md)
- [Design and milestones](DESIGN.md)
- [Current status](STATUS.md)
- [Change log](docs/MODLOG.md)
- [Workflow record](docs/workflow-writeup.md)
- [Project rules](AGENTS.md)

## Credits and source boundaries

Built with [SKSE](https://skse.silverlock.org/) and
[alandtse's CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG).
The [AI Game Modding Guides](https://github.com/trevaintdead/ai-game-modding-guides)
inform the documentation, telemetry, and human playtest workflow.
[SkyCraft](https://github.com/chasmlol/SkyCraft) was reviewed for organization and
performance instrumentation; its passthrough architecture is not used here.

Unofficial fan project, not affiliated with Bethesda or Capcom. No proprietary
game files, extracted assets, or decompiled game code belong in this repository.
Dependency license and source requirements apply to distributed builds.
