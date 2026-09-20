# ESPressio Threading

`EDP-Threading` provides the ESPressio Development Platform's deterministic execution-domain semantics for finite Tasks and persistent Dedicated Threads.

The library is intentionally layered above `EDP-Platform`: Platform owns native execution/synchronization mechanisms, while Threading owns Task/Thread lifecycle, bounded topology, cooperative cancellation/stop, waiting, scheduling policy and shutdown.

## Current implementation status

The v1 runtime architecture is implemented on `main` through the host-validation stage. The implementation includes compile-time topology/requirements, bounded Task facilities, FIFO Worker-Lease scheduling, isolated single-Worker `DedicatedWorkerLease` resources, persistent restart-safe Dedicated Threads, dense targeted wake routing, canonical monotonic waits/sleep, transactional infrastructure Start/rollback, and terminal cooperative shutdown.

The implementation deliberately contains no hidden heap fallback or unbounded EDP-controlled runtime growth.

Host/provider/target compilation and resource-layout evidence remain completion gates. Runtime behavior should not be treated as released until those validation gates are recorded.

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
