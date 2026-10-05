# Dependencies and attribution

The original Skyrim's Dogma source is GPL-3.0-or-later with the permissions in
LICENSE-EXCEPTIONS.md. No Skyrim or Dragon's Dogma assets, decompiled game code,
or extracted game files are included.

| Dependency | Pinned source / use | License |
| --- | --- | --- |
| CommonLibSSE NG (alandtse) | `94faaed0c60eddd8347767f2d4d29a97c93bde8c` (v11.0.0); Skyrim/SKSE adapter | GPL-3.0-or-later with upstream Modding and Linking exceptions |
| vcpkg | `ee12231b20c95013c6638d845d04c91559a1d1ff`; dependency versions and build tooling | MIT |
| spdlog / fmt | versions locked by vcpkg baseline; logging and formatting | MIT |
| rapidcsv | version locked by vcpkg baseline; CommonLib dependency | BSD-3-Clause |
| DirectXTK / DirectXMath / DirectX-Headers | versions locked by vcpkg baseline; CommonLib dependencies | MIT |
| OpenVR | included in the CommonLib multi-runtime distribution; no VR support advertised by this plugin | Valve BSD-3-Clause |
| MinHook hde64 | CommonLib source fallback's patch validation helper; no hooks installed by this mod | upstream BSD notices |

Full applicable dependency notices are copied from the build inputs into the
package's `SkyrimsDogma/notices` directory. The build source is downloaded by
`scripts/setup.ps1` and retained locally under the ignored `.deps` and `.tools`
directories. Keep the exact source, build recipes, notices, and pinned dependency
revisions with any binary distribution; this first package is a local playtest
artifact, not a public release.

The architecture is Skyrim-native. [SkyCraft](https://github.com/chasmlol/SkyCraft)
informed documentation and telemetry organization only. Workflow principles
come from [AI Game Modding Guides](https://github.com/trevaintdead/ai-game-modding-guides).
Neither project's passthrough architecture or code is included here.
