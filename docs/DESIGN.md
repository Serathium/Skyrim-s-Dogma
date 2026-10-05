# Design document

The current design is maintained in [the project-root DESIGN.md](../DESIGN.md).
Read that file before changing the architecture or implementation plan.

## Planned step: observer 0.1.1 boundary continuity

Preserve the independent stamina model when consecutive valid samples cross
between exterior cells in the same nonzero worldspace (a shared coordinate
system). Store copied cell/worldspace IDs, interior status, position, and stamina
capacity, never engine pointers. Classify continuity in a small engine-independent
function used by the observer and deterministic regression tests.

Keep resets on session lifecycle, menus/unsupported states, intervals outside
(0, 0.5] seconds, changed stamina capacity, interior-cell transitions, changed
worldspace, missing worldspace at a cell boundary, or movement exceeding 2,000
units/second in three dimensions. The distance guard is a heuristic; a very
short teleport can be indistinguishable from walking without additional events.
It does not establish that a destination cell is geographically adjacent.

Log every accepted cell crossing with old/new cell, worldspace, elapsed interval,
speed, and model time/stamina before and after. These numbers must prove the
model advanced instead of resetting to native stamina. Add worldspace/interior
fields to regular samples. No gameplay writes or settings changes.

Test ordinary crossings, exterior/interior/worldspace changes, zero worldspace,
timing gaps, capacity changes, horizontal/vertical discontinuities, invalid
numbers, and model integration across repeated boundaries. Build/package 0.1.1
without overwriting 0.1.0. The user then repeats walk/sprint crossings in Whiterun,
interior transitions, menu and reload tests; overworld and mounting remain
optional coverage. Do not claim live continuity until that session is analyzed.
