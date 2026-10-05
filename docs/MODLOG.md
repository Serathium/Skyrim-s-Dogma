# Change log

## 2026-10-05 - Observer 0.1.1 boundary continuity

**Changed:** Wrote the plan in `docs/DESIGN.md` before code; added a pure
continuity classifier and integrated it into the observer. Exterior-cell
crossings in the same nonzero worldspace preserve model time/stamina when
timing and position checks pass. Added worldspace/interior telemetry and a
before/after record for every accepted crossing. Raised version to 0.1.1 and
updated binary verification, packaging, and the focused playtest procedure.

**Why:** The 0.1.0 user session had 20 ordinary city cell-change resets, preventing
continuous experimental stamina measurements across that route.

**Tested how:** From the project folder:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

The new tests cover repeated crossings without model rebasing, interior and
worldspace transitions, unknown worldspace at boundaries, time gaps, capacity
changes, horizontal/vertical discontinuities, invalid values, and safety limits.
Inspected binary metadata and package paths/embedded DLL hash/source inventory.

**Result:** Full DLL build passed. **5,512 model checks + 107 continuity checks,
zero failures; CTest 2/2 passed.** Metadata identifies 0.1.1, only runtime
1.7.104.0, minimum SKSE 2.3.1. Build log: `build/boundary-build.log` (ignored).
Package checks passed: layout, embedded DLL/manifest hash, source-only inventory,
whitespace, no staged changes, and unchanged 0.1.0 package hash.
Installable output: `dist/SkyrimsDogma-0.1.1-observer.zip`. Version 0.1.0 remains
available for reverting. No failed build/test attempts occurred in this step.

**Still broken / not tested:** 0.1.1 live boundary preservation and performance
are **not tested**. The speed guard is a heuristic, not exact teleport detection;
very short teleports can resemble walking. No actual DDDA gameplay control is
implemented. Mounting/overworld coverage remains optional in the next test.

**Next:** User follows [PLAYTEST-0.1.1.md](PLAYTEST-0.1.1.md); the agent checks
crossing model-time increments and safe resets. Gameplay should remain unchanged.
No files outside the project were written; nothing was staged, committed, or pushed.

## 2026-10-05 - First observer playtest analyzed

**Changed:** Added the session evidence report and updated README, status, and
playtest/workflow documentation. No source, configuration, or tested package
was changed.

**Why:** Record actual integration evidence before moving to gameplay control.

**Tested how:** Read the existing observer and SKSE logs in place and counted
sample records, lifecycle events, suspension reasons, and logged timings.
Compared them with the user's report of completing the checklist in Whiterun
and Dragonsreach except horse mounting. Documentation check:

```powershell
Get-Content .\docs\PLAYTEST-2026-10-05.md
git diff --check
```

**Result:** SKSE confirms successful DLL loading. 231 sample records include
movement, sprinting, native stamina 0–100, and model exhaustion. Initial load
and reload succeeded. Menu, camera, airborne, swimming, and furniture pauses
were recorded. No observer warnings/errors were found; the user noticed no
gameplay change and reported no crash. Full evidence and timing limits are in
[the session report](PLAYTEST-2026-10-05.md).

**Limitations / not tested:** Horse mounting, overworld travel, death, failed
loads, new-game start, disabled observer mode, long sessions, and full frame-time
overhead are **not tested**. Twenty city cell-boundary resets interrupted the
experimental model's continuity; current observer behavior follows the design,
but needs refinement before gameplay control. First-sample read cost was about
7.95 ms; logged costs do not include sample-line writing.

**Next:** Plan continuous observation across ordinary boundaries while keeping
safe resets. No repeated user test is needed for this documentation-only step.
Keep the tested package unchanged; no commit or push was performed.

## 2026-10-05 - Observer build and playtest package verified

**Changed:** Completed the pinned CommonLib source build and observer DLL;
corrected the OpenVR link path, explicit SKSE runtime metadata, and portable ZIP
creation. Added binary verification and packaged matching source, debugging
symbols, instructions, and license notices. Finalized readiness documentation.

**Why:** Deliver a verifiable observation-only baseline for the first user test.

**Tested how:** From the project folder:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

Inspected DLL exports, metadata, runtime dependencies, ZIP paths, embedded DLL
hash, source-only archive inventory, and Git whitespace/staging state.

**Result:** Full build passed; **5,512 model checks, 0 failures; CTest 1/1**.
Binary validation passed: x64, three SKSE exports, only runtime 1.7.104.0,
minimum SKSE 2.3.1. Package layout/hash checks passed. The installable package is
`dist/SkyrimsDogma-0.1.0-observer.zip`; checksums are in `dist/SHA256SUMS.txt`.

**Failures retained:** The imported OpenVR target attempt did not resolve the
apostrophe path issue; basename linking and a build-only search directory did.
Metadata inspection caught upstream unused runtime slots set to 1.0.0; explicit
zero-initialized SKSE data corrected them. Initial ZIPs used backslashes; the
portable replacement needed both .NET compression assemblies before succeeding.

**Still broken / not tested:** Loading, sampling, performance, lifecycle, and
unchanged gameplay inside Skyrim are **not tested**. No DDDA gameplay control
is implemented yet. No game installation or save was changed, and nothing was
staged, committed, or pushed.

**Next:** User installs through Vortex and follows [PLAYTEST.md](PLAYTEST.md).
The agent then reads local logs before implementing movement/stamina control.

## 2026-10-05 - Toolchain, core tests, and runtime checks

**Changed:** Installed the authorized C++ toolchain, implemented the independent
model and observer source, added repeatable setup/build/runtime-check scripts,
and updated documentation to preserve the user's move of `AGENTS.md`,
`DESIGN.md`, and `STATUS.md` to the repository root. Human test instructions now
make clear that the agent can read local logs directly.

**Why:** Prove model rules and installed-runtime assumptions before delivering a
DLL for the user's first in-game test.

**Tested how:** Ran these commands from the project folder:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -CoreOnly
powershell -ExecutionPolicy Bypass -File .\scripts\check-runtime.ps1
```

**Result:** **5,512 checks, 0 failures; CTest 1/1 passed.** Runtime check reports
Skyrim 1.7.104.0, SKSE loader file version 0,2,3,1, Address Library format 5,
and the previously recorded Skyrim executable SHA-256. Installed toolchain:
Build Tools 17.14.41, MSVC 19.44.35229.0, Windows SDK 10.0.26100.0, and CMake
3.31.6-msvc6.

**Attempts retained:** First compiler installation returned 1602 (canceled);
the second succeeded. Available prebuilt CommonLib used newer MSVC 19.51, so
the build selected pinned source compilation. Usage resets interrupted that
compilation. The full build then exposed MSB4100 from an apostrophe in an
absolute OpenVR library path; an imported library target now emits a relative
path, with verification in progress.

**Still broken / not tested:** Full DLL compilation/export checks and packaging
are pending. Loading, sampling, and gameplay behavior in Skyrim are **not
tested**. The passing pure tests do not establish engine compatibility.

**Next:** Finish source build, validate DLL and package, then run the user
procedure in [PLAYTEST.md](PLAYTEST.md). Keep [STATUS.md](../STATUS.md) current.

## 2026-10-05 — Reconnaissance and observer milestone

**Changed:** Replaced template README/status/workflow documents, added the
technical reconnaissance and human playtest guide, and documented the initial
observer design. The observer/core source and INI are written; build setup is
in progress. Playtest instructions match the actual configuration and log fields.

**Why:** Preserve exact runtime evidence and make the first step measurable
before movement control. Skyrim remains the host and owner of gameplay state.

**Tested how:** Read executable metadata, installed mod/deployment files,
existing SKSE/DDDA proxy logs, and selected DDDA archive metadata during
reconnaissance; inspected upstream source and release documentation. No game was
launched or instrumented for reconnaissance. Read observer/configuration source
to check the test procedure and explain model time, resets, and log limits.

**Result:** Identified Skyrim 1.7.104.0 / SKSE 2.3.1 / Address Library v13;
identified DDDA Steam build 2364871 and executable fingerprint. Existing SKSE
logs show successful prior operation. Verified the pinned CommonLib source has
current runtime and Address Library format-5 support.

**Still broken / not tested:** The new DLL is not yet built or tested in Skyrim.
No DDDA gameplay recreation exists. Framework combinations, parameter semantics,
control overrides, and companion AI are not tested.

**Next:** Build the observer and pure model tests, validate packaging, and record
the exact commands/results before the user runs `PLAYTEST.md`.
