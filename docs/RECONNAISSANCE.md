# Technical reconnaissance

Evidence snapshot: **2026-10-05**. Reconnaissance was completed before development
was authorized. This document preserves its findings. Later implementation and
test results belong in `MODLOG.md` and `STATUS.md`.

**Verified locally** means a file, executable header, configuration, or existing
log was read. **Verified upstream** means public source or release documentation
was inspected. Neither means that we ran that release in this Skyrim setup.
**Proposed/inferred** identifies a conclusion that still needs an experiment.

## 1. Feasibility

**A staged, Skyrim-native reimplementation is feasible.** Movement states,
stamina rules, limited melee, and individual skills have viable routes through
SKSE, Skyrim's actor systems, and community frameworks. Exact DDDA physics,
animation behavior, and a complete Pawn system have not been demonstrated.

Skyrim remains authoritative for position, collision, actors, quests, rendering,
and saves. Dragon's Dogma informs design and measurement; it will not run
alongside Skyrim or send it player state. Implementation must use original code
and original or permission-cleared assets.

### Verified Skyrim installation

| Evidence | Observation |
| --- | --- |
| Install | `C:\Program Files (x86)\Steam\steamapps\common\Skyrim Special Edition` |
| Executable | `SkyrimSE.exe`, **1.7.104.0**, x64 PE machine `0x8664` |
| Steam identity | App **489830**, installed build **24914197** |
| Fingerprint | 37,910,440 bytes; SHA-256 `846EFCCF0C1374D71F892907F46549560F2FCB0A75CB87A3EED438BAA0F1402F` |
| PE timestamp | 2026-08-24 16:24:38 UTC; a header field, not proof of install date |
| SKSE | **2.3.1**, including `skse64_1_7_104.dll` |
| Address Library | Deployed **v13**; `Data\SKSE\Plugins\versionlib-1-7-104-0.bin` has a format-5 header |
| Existing runtime evidence | Oct 4 SKSE logs show successful initialization, loaded plugins, and co-save activity |

Those logs establish that SKSE worked in the recorded session. A fresh launch
with this project's plugin is **not tested**. “Special Edition” and “Anniversary
Edition” storefront labels are insufficient for selecting compatible DLLs; use
the executable runtime and Address Library format.

### Verified mod manager and installed mods

Vortex **2.7.2** is installed at `C:\Program Files\Vortex`. Skyrim SE staging is
`C:\Users\samue\AppData\Roaming\Vortex\skyrimse\mods`; the inspected profile ID
was `2YRnuzb6O`. No Mod Organizer installation was found in inspected locations,
which is not an exhaustive proof of absence.

Deployed components include SKSE 2.3.1, Address Library v13, Engine Fixes
**7.0.21 beta**, Skyrim Priority **3.4.0**, A Quality World Map, Better Dialogue
Controls, eFPS, No More Radial Blur, and Skyrim Project Optimization. Existing
SKSE logs identify EngineFixes.dll and PriorityMod.dll as loaded correctly.
An unfinished `upot anniversary edition...installing` directory is not evidence
of a completed install. BethINI Pie is present as a setup tool.

Active extra plugins read from `Plugins.txt` were
`Skyrim Project Optimization - Full Version.esm`, `Occ_Skyrim_Tamriel.esp`,
`IcePenguinWorldMap.esp`, and `NoRadialBlur.esp`. TDM, OAR, Precision, BFCO, and
SkyUI were not present in the inspected SE setup. Historical Vortex entries for
other games/profiles were not treated as this profile's inventory.

## 2. Recommended technical route

Build a **64-bit C++ SKSE plugin** for Steam **1.7.104.0 only**, using
[SKSE 2.3.1](https://skse.silverlock.org/),
[Address Library v13](https://www.nexusmods.com/skyrimspecialedition/mods/32444),
and pinned current CommonLibSSE-NG source. Reject unsupported runtimes before
accessing game objects.

The inspected [alandtse CommonLibSSE-NG source](https://github.com/alandtse/CommonLibSSE-NG)
at v11.0.0 commit `94faaed0c60eddd8347767f2d4d29a97c93bde8c` contains both the
1.7.104 runtime definition and Address Library format-5 handling. Its build uses
C++23. Record the exact MSVC/toolset and dependencies in build instructions.
An arbitrary older CommonLib revision from a tutorial is not an adequate
substitute. The pinned source carries GPL terms with modding/linking exceptions;
preserve notices and use compatible distribution terms.

Prove loading, lifecycle handling, sampling, logging, and pure model logic first.
Then introduce one reversible movement change through Skyrim's character
controller and actor systems. Assign one owner for speed, stamina, and input
decisions. Avoid repeated teleportation as movement. If TDM owns facing,
coordinate with it rather than competing for writes.

Keep timing-sensitive rules in native C++; use Papyrus/records for slower
progression and content. Adopt animation/hit-detection frameworks after exact
runtime validation. No inter-game transport is needed.

## 3. Existing tools and frameworks to reuse

These are **upstream findings**, not an installed or tested dependency bundle.
Add them individually; combined compatibility remains unverified.

| Candidate | Purpose | Evidence and constraint |
| --- | --- | --- |
| [True Directional Movement](https://www.nexusmods.com/skyrimspecialedition/mods/51614), [source](https://github.com/ersh1/TrueDirectionalMovement) | Facing, target lock, integration API | 2.3.1 inspected; 2.3.0 introduced 1.7.99+ support. Optional headtracking patch and menu dependencies need separate checks. |
| [Open Animation Replacer](https://www.nexusmods.com/skyrimspecialedition/mods/92109), [source](https://github.com/ersh1/OpenAnimationReplacer) | Conditional animations | 3.2.1 documents 1.7.99+ support. Animation Queue Fix 1.0.2 is relevant to normal preloading. |
| [Precision](https://www.nexusmods.com/skyrimspecialedition/mods/72347), [source](https://github.com/ersh1/Precision) | Weapon collision/hit information | 2.0.5 added 1.7 support; 2.0.6 fixes impulse/layout issues. Requires appropriate behavior content. |
| [BFCO](https://www.nexusmods.com/skyrimspecialedition/mods/117052) | Attack behavior and combinations | 3.100.11 inspected; 3.100.8 added 1.7.104 support. Validate its full dependency chain. |
| [Animation Motion Revolution](https://www.nexusmods.com/skyrimspecialedition/mods/50258) | Animation-driven motion | Original release does not establish 1.7.104 support. An [unofficial replacement](https://www.nexusmods.com/skyrimspecialedition/mods/191697) explicitly targets it; installation/provenance need validation. |
| [Payload Interpreter](https://www.nexusmods.com/skyrimspecialedition/mods/65089) | Timed animation actions | A [current DLL replacement](https://www.nexusmods.com/skyrimspecialedition/mods/192679) still needs original behavior content. A DLL update is not a complete package. |
| [Pandora Behavior Engine Plus](https://github.com/Monitor221hz/Pandora-Behaviour-Engine-Plus) | Combined behavior generation | v5.0.0-beta inspected. Choose one compatible generator/output set; do not combine stale Nemesis and Pandora outputs. |
| [TrueHUD](https://www.nexusmods.com/skyrimspecialedition/mods/62775) | Target/resource UI | 1.1.10 documents 1.7.99+ support; optional later. |
| [MCM Helper](https://www.nexusmods.com/skyrimspecialedition/mods/53000) and SkyUI | Configuration | 1.6.3 documents the newer runtime family; combined 1.7.104 behavior is not tested. Not needed initially. |
| [SmoothCam](https://github.com/mwilsnd/SkyrimSE-SmoothCam) | Third-person camera | Beta 1.8.1 exists, but inspected README still named older runtimes. Exact 1.7.104 support remains uncertain. |
| [SCAR](https://github.com/max-su-2019/SCAR) and forks | NPC attack selection | 1.7.104 compatibility was not established. It is not a complete Pawn planner. |

BFCO's documented dependency set includes AMR, MCM Helper, OAR, Payload
Interpreter, XPMSSE, and a behavior generator. Version 3.100.10 removed its
PapyrusUtil dependency. Check the chosen release, not an old guide's list.

### Existing Dragon's Dogma-inspired Skyrim work

The search found [krembrule's Fighter moveset](https://www.nexusmods.com/skyrimspecialedition/mods/77366),
[dagger moveset](https://www.nexusmods.com/skyrimspecialedition/mods/80085), and a
[BFCO conversion](https://www.nexusmods.com/skyrimspecialedition/mods/124772).
These are animation/attack precedents, not a complete locomotion, stamina,
vocation, and Pawn system. A [Dragon's Dogma 2 Fighter moveset](https://www.nexusmods.com/skyrimspecialedition/mods/123708)
is sequel-inspired and should not be treated as DDDA evidence. Read original
and conversion permissions before reuse. No complete implementation of all six
requested systems was found; the search cannot prove none exists.

## 4. Work requiring custom SKSE/C++ code

- Movement/stamina state rules, input timing, cancellation, and coordination
  with Skyrim's controller and other movement mods.
- Stamina ownership, exhaustion/recovery, and prevention of duplicate drains.
- Attack buffering, skill activation windows, hit callback policy, and damage
  or reaction arbitration where frameworks provide suitable APIs.
- Runtime validation, lifecycle reset/suspension, telemetry, and reliable
  restoration of any future transient override.
- Eventually a tactical companion policy while Skyrim continues to own actors,
  navigation, packages, and saves.

These are proposed responsibilities, not proof that every engine access point
is known. Prefer public events/APIs and relocation-backed access; add hooks only
for specific measured gaps.

## 5. Work Papyrus can handle

Papyrus, quests, perks, spells, records, and aliases can handle vocation
selection/progression, skill learning/equipment, quest integration, menu-to-native
calls, follower registration, dialogue, and slow companion setup. Papyrus is
not the preferred layer for per-frame movement, precise hit windows, stamina
integration, or high-frequency tactical decisions. The observer needs neither
Papyrus scripts nor a content plugin.

## 6. Dragon's Dogma information realistically recoverable

### Verified installation

| Evidence | Observation |
| --- | --- |
| Install | `C:\Program Files (x86)\Steam\steamapps\common\DDDA` |
| Steam identity | App **367500**, installed build **2364871** |
| Executable | `DDDA.exe`, generic file/product **1.0.0.0**, x86 PE machine `0x014c` |
| Fingerprint | 21,841,776 bytes; SHA-256 `19FACC2642F79A09D14CE55EACE5F228161F2F8BAB60EC67EB3C6196CC6277B8` |
| PE timestamp | 2017-12-13 15:50:09 UTC |
| Rendering imports | `d3d9.dll`, `d3dx9_43.dll`; also imports `DINPUT8.dll` |
| Existing proxy | `dinput8.dll`, file version 3.4.1.0; identical to local package labelled 3.5.0 |

Steam build and executable fingerprint are more useful than the generic file
version. This is original **32-bit MT Framework DDDA**, not Dragon's Dogma 2
and not an REFramework target.

The installed dinput8 configuration and old log show camera adjustments and
movement/jump hook support, with movement/jump changes disabled in the inspected
INI. Some settings disagree with the recorded session, so this is not yet a
verified vanilla measurement baseline. An old missing-backup-path message also
means backup success cannot be assumed from a setting alone.

Read-only signature checks from public [ddda-dinput8 source](https://github.com/kubik-jaroslav/ddda-dinput8)
found unique movement/jump signatures at preferred virtual addresses
`0x00B81BD7`, `0x00937CFF`, and `0x00937D2F`, consistent with the old hook log.
This identifies relevant locations, not validation of all hooks. No runtime
instrumentation was attached.

### Without reverse engineering the executable

- Controlled play trials and recordings can measure visible sprint, exhaustion,
  recovery, jump, turning, acceleration, and cancellation behavior.
- Archive inventories and readable metadata can expose parameter classes,
  animation names, skills, vocation relationships, and offline Pawn data.
- Public tools/mod source can explain structure and access patterns, which
  still need checks against this exact build.
- Save-format documentation can explain persistence without editing a save.
  No save was inspected or modified during reconnaissance.

Read-only inspection found **8,536 ARC files**. `game_main.arc` has version 7 and
2,320 entries; `bbsrpg_core.arc` has 187 entries; the inspected offline Pawn
archive has 45 entries. Decompression was in memory, with no proprietary
payloads added to this repository.

Data includes named stamina recovery resources for normal/walk/run/dash/lift/
climb states, jump and defense parameters, skill tables, action parameters, and
GOAP-labelled AI resources. XFS samples expose names including `mStamina`,
`mFrame`, `mMotSpd`, `mShrinkMax`, and `mBlowMax`. Units, indexing, timing, and
relationship to live behavior remain **unverified**. Names alone do not reveal
complete formulas or an AI decision process.

### Suitable tools and limitations

| Tool/resource | Appropriate use |
| --- | --- |
| [ArisenTools](https://github.com/mhvuze/ArisenTools) | ARC reader/extractor source and type mappings. Some list-like utilities write output; execute future tooling only in ignored scratch space. |
| [dd-tools](https://github.com/chrispurnell/dd-tools) | Skill/XML conversion and XFS metadata research; not proof every format is decoded. |
| [ddda-dinput8](https://github.com/kubik-jaroslav/ddda-dinput8), [fork](https://github.com/chrispurnell/ddda-dinput8) | Existing x86 instrumentation/configuration/structures; adapt narrowly for this fingerprint. |
| [DDDAFix](https://github.com/Lyall/DDDAFix) | Another example of DDDA hooks/display fixes; not a gameplay specification. |
| [Ghidra](https://github.com/NationalSecurityAgency/ghidra) | Static x86 analysis for precise questions left unanswered by files/observation. Decompiled output is fallible and stays out of Git. |
| [Frida](https://frida.re/) | Targeted runtime function/argument observation with matching Windows/x86 setup. Not installed or used during reconnaissance. |
| [Cheat Engine](https://www.cheatengine.org/) | Find candidate live values and correlate actions. Addresses and semantics need repeated validation. |
| [x64dbg / x32dbg](https://x64dbg.com/) | Use x32dbg for DDDA breakpoints and disassembly; exact hook behavior needs testing. |
| [RenderDoc](https://github.com/baldurk/renderdoc) | Its supported APIs do not include DDDA's native D3D9 path. Not the appropriate first tool for gameplay questions. |
| [apitrace](https://github.com/apitrace/apitrace) | Possible D3D9 rendering-call inspection, generally not useful for stamina or tactical AI rules. |

Recovering useful parameter relationships and approximate timings is realistic.
Automatically recovering original source, the whole Pawn AI, or a complete
behavioral specification is not. Start from files, public source, and controlled
observation; analyze or instrument only to answer a defined remaining question.
Online systems, anti-cheat, and DRM work are out of scope.

## 7. Major unknowns

1. Fresh-load stability of our DLL/toolchain/CommonLib combination, especially
   actor layout access and lifecycle transitions on 1.7.104.
2. Movement behavior on slopes, during turns/jumps/swimming, and across mounts,
   dialogue, animation transitions, and cell changes.
3. DDDA parameter units and the division of behavior between controller,
   animation, parameter tables, and code.
4. Which changes can use framework APIs without fragile hooks.
5. Compatibility/ownership across TDM, BFCO, Precision, AMR, and other mods on
   this exact runtime.
6. Animation availability, permissions, and original animation work required.
7. How closely Skyrim navigation/packages can support Pawn-like coordination.
8. Controller input, save/load restoration, long sessions, and performance
   budgets. These require actual game tests.

## 8. Risks

- **Runtime mismatch:** compilation cannot prove layout compatibility. Pin
  dependencies, validate runtime, observe first, and keep versioned logs.
- **Competing systems:** vanilla sprint, animation motion, and custom code can
  disagree about speed or drain stamina twice. Assign ownership explicitly.
- **Persistent bad state:** later overrides need restoration across interruption
  and removal. Initial observation changes no gameplay state or save data.
- **Scope:** movement, combat, and Pawn AI each deserve separate milestones.
- **False fidelity:** a field name, number, or single video is not a complete
  specification. Record units and repeat trials; label inferred constants.
- **Updates:** runtime/framework changes can invalidate DLLs and behavior graphs.
  Preserve reproducible builds and a tested version matrix.
- **Asset/source boundaries:** availability does not grant reuse permission.
  Keep assets, extracted payloads, and decompiled code out of Git, and preserve
  dependency notices and source obligations.
- **Measurement baseline:** existing DDDA proxy settings may affect findings.
  Record a controlled baseline before measuring.

## 9. Recommended development phases

1. **Loader/telemetry:** build the observer, test pure logic and packaging, then
   have the user verify loading and useful logs.
2. **DDDA measurements:** gather repeatable movement/stamina observations and
   decode only data needed for specific questions.
3. **Movement/stamina:** introduce one optional reversible override, then a
   normal/sprint/exhaustion/recovery loop with cleanup tests.
4. **Animation:** integrate directional movement and animation frameworks, one
   dependency at a time.
5. **One melee weapon:** establish attack phases, hits, damage ownership,
   cancellation, and instrumentation before broader movesets.
6. **One vocation/skill:** resource costs, activation, interruption, progression,
   and save behavior.
7. **Reactions:** stagger/knockdown/recovery within dependable engine limits.
8. **One companion:** instrument decisions/navigation before expanding tactical
   coordination, knowledge, and Pawn-inspired roles.

Use [AI Game Modding Guides](https://github.com/trevaintdead/ai-game-modding-guides)
principles: design before code, reversible increments, retained failure records,
telemetry, human playtests, and source-only Git. Commit/push only when asked, as
this project's `AGENTS.md` requires.

[SkyCraft](https://github.com/chasmlol/SkyCraft) was inspected for organization.
Its design/performance counters demonstrate responsibility boundaries, milestone
records, and measured costs. Its Minecraft-authoritative passthrough/player-puppet
architecture is unsuitable here and is not adopted.

## 10. Smallest prototype proving the route

**First integration step:** an observer SKSE DLL that rejects unsupported
runtimes, records lifecycle/movement/stamina, suspends/resets across invalid
states, and calculates an experimental stamina model without applying it. A
successful user test proves loading, data access, and telemetry integration.
It does **not** prove recreated locomotion.

**Then the smallest gameplay proof:** third-person, grounded, on-foot movement
with one optional configurable acceleration/stamina rule, a measurable
sprint/exhaustion/recovery cycle, and restoration when disabled or interrupted.
Use existing animations and one location; combat, vocations, new assets, and
companions are not needed for this proof.

Compare requested state, actual displacement, native stamina, model state, and
update cost. Test menus, reloads, death, mounting, and removal. The user plays;
the agent analyzes logs and measurements. No fresh in-game test or gameplay
viability claim was made at the end of reconnaissance.
