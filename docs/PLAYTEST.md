# First playtest: observer

Status: first session completed in Whiterun/Dragonsreach; loading and observation
confirmed, unchanged gameplay reported. Horse mounting and overworld travel are
**not tested**. See [session results](PLAYTEST-2026-10-05.md). Verified package:
`dist/SkyrimsDogma-0.1.0-observer.zip`. Full build, binary metadata, and package
validation passed.
The pure model passed 5,512 checks with no failures, and the installed runtime
check passed. Those automated checks alone do not prove DLL loading; the session
logs now confirm it. See [STATUS.md](../STATUS.md).

This build reads movement/stamina and calculates an independent experimental
stamina model. It should make **no gameplay change**. Its model will not match
the stamina bar and is not a set of DDDA measurements.

## Install

1. Close Skyrim. In Vortex, select the existing Skyrim Special Edition profile.
2. Use **Install From File** to select `SkyrimsDogma-0.1.0-observer.zip`. Enable
   Skyrim's Dogma and deploy.
3. The deployed files should be `Data\SKSE\Plugins\SkyrimsDogma.dll` and
   `Data\SKSE\Plugins\SkyrimsDogma.ini`. There is no ESP/ESM to enable.
4. Start **SKSE**, using the existing Vortex entry or this executable:

   ```text
   C:\Program Files (x86)\Steam\steamapps\common\Skyrim Special Edition\skse64_loader.exe
   ```

5. Load a character in a quiet location. A separate manual save is useful for
   repeatable trials; the observer itself does not require a save.

## Exercise the observer

Use third person and a safe, mostly flat location.

| Step | Action | Expected in the game | What we check in the log |
| --- | --- | --- | --- |
| 1 | Stand still for 10 seconds. | Normal Skyrim; no new menu/message is required. | Startup, player samples, little horizontal displacement. |
| 2 | Walk and run about 15 seconds each; turn several times. | Existing controls and speed remain unchanged. | Changing positions/displacement and continuous samples. |
| 3 | Sprint in short bursts, then as long as the game allows. Rest 20 seconds. | Normal vanilla stamina drain/recovery. | Native sprint/stamina observations and separate model transitions; exhaustion depends on sprint duration and experimental settings. |
| 4 | Open inventory/pause for 10 seconds, then close it. | Normal menus. | Model suspension/resumption without a large displacement spike. |
| 5 | Enter and leave an interior if convenient. | Normal loading. | Safe reset across loading/cell changes. |
| 6 | Reload the same save, wait 10 seconds, then walk. | Normal reload. | Lifecycle reset and resumed sampling without stale state. |
| 7 | Switch to first person for 10 seconds, then return to third person. | Normal camera change. | `suspend reason=camera`, then `resume` and new samples. |
| 8 | If convenient, sit, jump, swim, mount, or start dialogue. | Normal behavior. | Suspension in detected unsupported states; report what was actually tested. |
| 9 | Exit normally. | Normal exit. | A completed session log; a special shutdown event is not required. |

Do not enter danger merely to test death. Mark untried steps **not tested**.
Dragon's Dogma does not need to run for this test.

## What the log should contain

Expected startup records include `plugin_loaded`, `mode=observation_only`, and
`plugin_ready gameplay_writes=false`. A successful load should include
`lifecycle=post_load_game success=true`; a new game uses `lifecycle=new_game`.
After entering a valid third-person, grounded, on-foot state, expect `resume`
followed by `sample` records. Menus or unsupported states produce `suspend` with
a reason. `lifecycle=pre_load_game` pauses observation until the load succeeds.

Each sample distinguishes:

- `actual_stamina` / `max_stamina`: Skyrim's observed values.
- `observed_sprint` / `observed_move`: actor state flags, not captured button
  presses or proof of movement intent.
- `horizontal_speed`: distance in Skyrim units per wall-clock second. It is
  useful only when `speed_valid=true`; the first sample after a reset is invalid.
- `shadow_state` / `shadow_stamina`: the independent model's prediction.
- `shadow_seconds`: valid observed wall-clock time since the latest reset.
- `read_us`: sample processing cost in microseconds before writing its log line;
  it is not the game's frame time or the full cost of file logging.

Sprint transitions are observed only at each sample. The latest sprint flag is
applied to the preceding interval, so transition timing is approximate within
that interval (100 ms by default), not frame-accurate input telemetry.

The model resets to native stamina when continuous observation resumes, then
evolves independently. It resets after menus/unsupported states, cell changes,
large position jumps, stamina-capacity changes, or sample gaps over half a
second. Time dilation is not modelled; the timebase is real elapsed time.
Do not interpret its independent exhaustion as a command sent to Skyrim.

Default settings are in `SKSE/Plugins/SkyrimsDogma.ini`:

| Setting | Default | Meaning |
| --- | --- | --- |
| `[Observer] Enabled` | `1` | `0` disables sampling after restarting Skyrim; the DLL can still load and log lifecycle events. |
| `SampleIntervalMs` | `100` | Schedule a sample roughly every 100 ms; game-thread scheduling can delay it. |
| `LogIntervalMs` | `1000` | Log samples about once per second, plus model state transitions. |
| `[ShadowModel] SprintDrainPerSecond` | `15` | Experimental model drain rate. |
| `RecoveryPerSecond` | `10` | Experimental model recovery rate. |
| `RecoveryThresholdFraction` | `0.25` | Recover to 25% of maximum before leaving model exhaustion. |

Leave these unchanged for the first test. Restart Skyrim after any later edit;
settings do not update during a running game. No setting changes native speed
or stamina in this build.

## Send back

After closing Skyrim, tell me which steps you completed and what you noticed.
On this PC I can read these logs directly, so you do not need to upload them:

- `C:\Users\samue\Documents\My Games\Skyrim Special Edition\SKSE\SkyrimsDogma.log`
- `C:\Users\samue\Documents\My Games\Skyrim Special Edition\SKSE\skse64.log`
- `SkyrimsDogma.ini` if you changed its settings.

The plugin rotates its log at launch and at 5 MiB, retaining up to two backups
(`SkyrimsDogma.1.log` and `.2.log`). I can also inspect those if the current log
no longer contains the startup or incident. Report after closing the test
session, before extra launches rotate away useful evidence.

Include this short result:

```text
Package version:
Loaded a save successfully: yes / no
Walking/running/sprinting felt unchanged: yes / no
Menu, cell transition, and reload completed: yes / no / partly
Optional actions actually tested:
Crash, freeze, or unexpected behavior (what happened just before it):
SkyrimsDogma.log created: yes / no
```

If the DLL fails to load, tell me; I can inspect `skse64.log` even if its own log
is absent. If it crashes, report what happened before launching again so I can
read the logs and any existing crash report. Do not add another framework or
change unrelated mods for this test.

## Remove and compare

With Skyrim closed, disable Skyrim's Dogma in Vortex and deploy. Relaunch SKSE
and compare with the same save. The observer registers no save data and needs no
save cleaning. Keep test logs separately when comparing runs. Do not attempt to
unload its DLL while Skyrim is running.
