# Observer 0.1.1: cell-boundary test

Status: **not tested in Skyrim**. This remains observation-only: controls,
speed, and native stamina should feel unchanged. The only change is how the
independent stamina model handles movement between exterior cells.

## Install and run

1. Close Skyrim. In Vortex, disable the previous Skyrim's Dogma observer version.
2. Use **Install From File** for `dist/SkyrimsDogma-0.1.1-observer.zip`, enable the
   new version, and deploy. Keep only one observer version enabled; no ESP/ESM
   is involved. Leave its settings at their defaults.
3. Launch the existing SKSE entry, or run this executable:

   ```text
   C:\Program Files (x86)\Steam\steamapps\common\Skyrim Special Edition\skse64_loader.exe
   ```

4. Load the same character and use third person in Whiterun.

## Focused actions

| Action | What you should see | What the agent checks |
| --- | --- | --- |
| Stand still 10 seconds. | Normal Skyrim. | Plugin version 0.1.1 and samples. |
| Walk/run the route used last time around Whiterun for 60 seconds; turn and retrace it. | Unchanged movement. No message is expected at cell boundaries. | `cell_crossing continuity=preserved` in the same worldspace, model time advancing instead of resetting. |
| Sprint along that route in bursts for about 30 seconds; rest 20 seconds. | Normal native stamina drain/recovery. | Before/after model stamina/time across crossings; no rebase to native stamina at ordinary boundaries. |
| Enter Dragonsreach, wait 10 seconds, leave, and walk again. | Normal loading. | Safe reset across the interior transition, then resumed samples. A menu pause can be the reset reason. |
| Open inventory for 10 seconds, close it, and walk. | Normal menus and movement. | Suspension and fresh baseline on resume. |
| Reload the same save, wait 10 seconds, and walk. | Normal reload. | Successful load and reset; no old model time carried across it. |
| Exit normally. | Normal exit. | Completed test evidence available locally. |

If convenient, travel through Whiterun's gate and walk a little outside, or
mount/dismount a horse. These are optional; tell me if tried. No risky encounters
are needed. If the city route does not cross any observed boundary, the logs will
show that and we will choose another route instead of assuming success.

## Expected log evidence

Each preserved crossing contains old/new cell IDs, a shared nonzero worldspace,
`dt`, three-dimensional speed, and model stamina/time before and after. The
after time should equal before time plus `dt`, within log rounding. Stamina is
the model prediction, not Skyrim's bar; it may rise during model exhaustion
even if native sprint is still observed. No separate `resume` should be caused
by that accepted boundary alone.

Interior/worldspace transitions, menus, unsupported states, intervals over
0.5 seconds, capacity changes, and movement above 2,000 units/second still reset
the model. This speed rule cannot reliably identify very short teleports; actual
teleport tests are **not tested**. No gameplay state is written by the observer.

`read_us` excludes writing the regular sample line, but includes any earlier
crossing/resume logs during that sample. It is not full plugin frame-time cost.

## Report back

Close Skyrim, then tell me:

```text
Version installed: 0.1.1
City walking/sprinting, Dragonsreach, menus, reload completed:
Movement and native stamina felt unchanged: yes / no
Went outside Whiterun: yes / no
Mounted a horse: yes / no
Crash/freeze/unexpected behavior and what happened just before:
```

I will read the existing `SkyrimsDogma.log` and `skse64.log` in
`C:\Users\samue\Documents\My Games\Skyrim Special Edition\SKSE` directly. Avoid
extra launches before reporting, as logs rotate. No upload or console command
is needed. To revert, close Skyrim, disable 0.1.1 in Vortex, enable 0.1.0 if
desired, and deploy. No save cleaning is required.
