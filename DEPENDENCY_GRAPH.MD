# EDP-Threading Dependency Graph

> Revalidated: 2026-11-10 — feature/logging-v1 exact Clock/main and Memory/main optional-owner review; new keyed ordinary mutex timed result adds no dependency edge.\n\n> Historical revalidation: 2026-09-28 — Event V1 support exposes the existing Dedicated-Thread managed-context wake path publicly and adds a keyed Threading-owned ordinary mutex adapter. No dependency edge changes; EDP-Memory remains mandatory.

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

## Full transitive dependency closure (10 November 2026)

The graph below represents each mandatory direct edge **and** its recursively reachable mandatory dependencies. Duplicate nodes appear wherever they are independently reached; that is intentional provenance rather than a cyclic dependency.

```text
EDP-Threading
├── EDP-System [direct]
├── EDP-Platform [direct]
│   └── EDP-System [transitive]
├── EDP-Clock [direct]
│   ├── EDP-System [transitive]
│   └── EDP-Platform [transitive]
│       └── EDP-System [transitive]
├── EDP-BoundedTopology [direct]
└── EDP-Memory [direct]
    ├── EDP-System [transitive]
    ├── EDP-Platform [transitive]
    │   └── EDP-System [transitive]
    └── EDP-BoundedTopology [transitive]
```

There are **no** new mandatory EDP package dependencies. Explicit context binding additionally requires the selected Platform binding contract/provider as described below. Memory's direct native finite/Forever blocked-waiter path remains separately unqualified for synchronization-provider failure under [Memory #16](https://github.com/ESPressio-Development-Platform/EDP-Memory/issues/16). Threading's Logging integration exercises the Memory `NoWait` acquisition route, with its own Clock-owned absolute deadline checks. This is not an assertion that every Memory waiting path has become safe.

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



## Selected current-context binding (23 November 2026)

The default source closure remains System, Platform, Clock, BoundedTopology and Memory above. Including `ESPressio_Threading_ContextBinding.hpp` and selecting `ContextBindingPolicy` consumes the optional CurrentContextBinding capability from the already mandatory Platform owner (`feature/profiling-v1` until reintegration), plus existing Signal/SpinLock and Clock contracts. There is no Threading -> Profiling or Threading -> SDK include edge. No foundation acquires a reverse Threading edge.

Concrete native demos optionally select Platform-ESP-IDF/main for binding, execution, SpinLock and canonical timebase, and Platform-FreeRTOS/main for Signal/Mutex. Their exact transitive package closure is Platform, System, Clock, Memory, Persistence, BoundedTopology and Security; these are application concrete dependencies, not mandatory Threading SDK dependencies. Portable synchronization in host tests is test-only. Versions stay 0.1.0.
