# EDP-Threading Dependency Graph

## Library Dependency Tree

```text
EDP-Threading
├── EDP-System [mandatory]
├── EDP-Platform [mandatory]
│   └── EDP-System [transitive]
└── EDP-Clock [mandatory]
    ├── EDP-System [transitive]
    └── EDP-Platform [transitive]
```

## Mandatory Dependencies

- `EDP-System` — supplies the compile-time Composition Framework used to declare execution capabilities and topology contracts.
- `EDP-Platform` — supplies substitutable execution-context and synchronization mechanisms.
- `EDP-Clock` — supplies the singular canonical monotonic timeline used by Threading deadlines and sleep semantics.

## Optional Dependencies

EDP-Threading has no optional ESPressio-library dependencies.

## Dependency direction

Threading is downstream of System, Platform and Clock. Those foundational libraries do not depend on Threading, preserving an acyclic dependency graph.
