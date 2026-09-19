# ESPressio Threading

`EDP-Threading` provides the ESPressio Development Platform's deterministic execution-domain semantics for finite Tasks and persistent Dedicated Threads.

The library is intentionally layered above `EDP-Platform`: Platform owns native execution/synchronization mechanisms, while Threading owns Task/Thread lifecycle, bounded topology, cooperative cancellation/stop, waiting, scheduling policy and shutdown.

## Current implementation status

The v1 implementation is in active development on `main`. Public vocabulary, compile-time topology declarations, move-only Task/Thread handles and compact Task-record foundations are present. Runtime facility/Worker/Dedicated-Thread machinery is being completed under the normative architecture recorded in the Policy repository.

No documentation below should be interpreted as validation of runtime behavior that has not yet been implemented and tested.

## Documentation

- [Architecture and execution model](./docs/ARCHITECTURE.MD)
- [Lifecycle, ownership and cancellation](./docs/LIFECYCLE.MD)
- [Deterministic resources](./docs/RESOURCES.MD)

## Dependencies

```text
EDP-Threading
├── EDP-System [mandatory]
├── EDP-Platform [mandatory]
│   └── EDP-System [transitive]
└── EDP-Clock [mandatory]
    ├── EDP-System [transitive]
    └── EDP-Platform [transitive]
```

There are no optional ESPressio-library dependencies.

See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD).
