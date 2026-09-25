# EDP-Threading Dependency Graph

> Revalidated: 2026-09-25 — the bounded-topology migration adds EDP-BoundedTopology as one mandatory foundational dependency. No optional dependency is introduced.

> Revalidated: 2026-09-23 — Composition Framework migration, manifest-free coherent local-source resource validation, and recorded resource results change compile/test/documentation surfaces only; no ESPressio-library dependency edge changed.

> Revalidated: 2026-09-20 — V2 compliance audit and Threading validation/resource-measurement work introduced no new ESPressio-library dependency.

## Library Dependency Tree

```text
EDP-Threading
├── EDP-System [mandatory]
├── EDP-Platform [mandatory]
│   └── EDP-System [transitive]
├── EDP-Clock [mandatory]
│   ├── EDP-System [transitive]
│   └── EDP-Platform [transitive]
└── EDP-BoundedTopology [mandatory]
    └── no ESPressio-library dependencies
```

## Mandatory Dependencies

- `EDP-System` — supplies the compile-time Composition Framework used to declare execution capabilities and topology contracts.
- `EDP-Platform` — supplies substitutable execution-context and synchronization mechanisms.
- `EDP-Clock` — supplies the singular canonical monotonic timeline used by Threading deadlines and sleep semantics.
- `EDP-BoundedTopology` — supplies capacity-derived bounded index representation, compact one-bit membership sets and non-owning intrusive FIFO topology. Threading retains all Task/Worker lifecycle, scheduling, synchronization and resource semantics.

## Optional Dependencies

EDP-Threading has no optional ESPressio-library dependencies.

## Dependency direction

Threading is downstream of System, Platform, Clock and BoundedTopology. BoundedTopology has no ESPressio-library dependencies and none of the foundational libraries depend on Threading, preserving an acyclic dependency graph.
