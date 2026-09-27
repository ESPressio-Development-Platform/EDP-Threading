# EDP-Threading Dependency Graph

> Revalidated: 2026-09-27 — H16 Command reconciliation changes `BoundedWaitWakeProvider::Wake` from Boolean operational success to the existing typed `BoundedWaitWakeResult`; no dependency edge changes.

> Revalidated: 2026-09-26 — Command prerequisite work adds EDP-Memory as a mandatory dependency so typed Task payload lifetime/ownership operations do not bypass the Memory domain. It also adds the Threading-owned ordinary-context bounded wait point.

## Library Dependency Tree

```text
EDP-Threading
├── EDP-System [mandatory]
├── EDP-Platform [mandatory]
├── EDP-Clock [mandatory]
├── EDP-BoundedTopology [mandatory]
└── EDP-Memory [mandatory]
```

## Mandatory Dependencies

- `EDP-System` — compile-time Composition Framework and topology contracts.
- `EDP-Platform` — substitutable execution-context and synchronization mechanisms.
- `EDP-Clock` — canonical monotonic timeline used by deadlines and sleep semantics.
- `EDP-BoundedTopology` — bounded index, membership and intrusive FIFO primitives.
- `EDP-Memory` — authoritative typed object lifetime and ownership-transfer boundary for Task payloads.

## Optional Dependencies

EDP-Threading has no optional ESPressio-library dependencies.

## Dependency direction

Threading is downstream of System, Platform, Clock, BoundedTopology and Memory. None of these foundations depends on Threading. Higher domains such as EDP-Command consume Threading's synchronization capabilities rather than Platform synchronization directly.
