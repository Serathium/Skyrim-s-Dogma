# How I made Skyrim's Dogma

## TL;DR

[3-6 bullets. What it is, what stack, roughly how long it took, and the one
thing you'd tell someone starting this.]

## The result

- **Games:** [Game A] v[version] + [Game B] v[version]
- **Stack:** [SKSE C++ plugin / Fabric Java mod / Rust + Bevy]
- **Built with:** [agent and model]
- **Time:** [hours, roughly]
- **Cost:** [plan tier, roughly]
- **OS:** [Windows, because that's what it needed]

[Screenshot or GIF]

## What works

- [Feature]
- [Feature]

## What doesn't work

- [Honest list. This section is the most valuable part of the document.]

## Before you start: what to check

The research I did that saved me time. Be specific enough to act on.

1. **Does Game A have a mod loader?** [Which one, which version, where from.]
   Without this there's no easy path; you'd be reverse engineering.
2. **Existing projects I read first:** [links + one line on what each taught you]
3. **Existing mods for Game A:** [links]
4. **File formats / docs I found:** [links]
5. **Version traps:** [what I had to downgrade or match exactly]

## The plan

[What you built first, second, third, and why that order. Show the milestones
you aimed at. This is the part people can copy.]

1. [Milestone 1: e.g. "plugin loads and writes a log line"]
2. [Milestone 2: e.g. "position crosses from B to A"]
3. [Milestone 3: e.g. "B's objects spawn in A's world"]
4. ...

## Which game owns the player

[Say which side is authoritative for player position and physics, and why. This
is the decision everyone gets wrong, and reversing it later means rewriting
both halves. SkyCraft made Minecraft authoritative for the player even though
Skycraft is the game you're looking at.]

## Transport and architecture

[How the two halves talk. Shared memory, socket, file, IPC? What messages go
across and at what rate? Keep this concrete; it's the part people copy. If the
gameplay game still renders offscreen, say so here rather than claiming it runs
headless.]

## What I prompted, roughly

[Actual prompts you used, with the successful ones intact and the bad ones too.
Do not clean these up. Show that the first prompt didn't work.]

**This one worked:**