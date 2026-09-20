# ESPressio Threading

`EDP-Threading` provides the ESPressio Development Platform's deterministic execution-domain semantics for finite Tasks and persistent Dedicated Threads.

The v1 architecture is static and Composition-driven. Platform owns native execution/synchronization mechanisms; Threading owns Task/Thread lifecycle, bounded topology, cooperative cancellation/stop, waiting, scheduling policy and shutdown. There is no runtime dependency injection, service locator, hidden heap fallback or unbounded EDP-controlled runtime growth.

## Public surface

Include:

```cpp
#include <EDP_Threading.hpp>
```

The public vocabulary includes:

- `TaskExecutionFacility<...>` — a finite Task record/callable/result facility backed by one or more declared `Worker<...>` contexts.
- `DedicatedWorkerLease<TaskIdentity,...>` — one isolated persistent Worker and bounded Task facility reserved for one critical Task identity.
- `DedicatedThread<ThreadIdentity,...>` — one persistent execution context whose semantic activation may be started, stopped and restarted.
- `ThreadingTopology<...>` — the complete statically declared Threading resource topology.
- `DedicatedThreadBinding<...>` / `BindDedicatedThread<ThreadIdentity>()` — binds application behaviour to a declared Dedicated Thread without moving behaviour into architectural entitlement.
- `StaticThreadingRuntime<...>` — the concrete statically owned Bootstrap/runtime façade.

## Bootstrap lifecycle

`StaticThreadingRuntime` deliberately separates initialization from execution start:

1. Construction owns the targeted wake topology, structural router, resource backing, Platform provider objects, bounded shutdown-wait surface and Dedicated Thread callable bindings.
2. `Initialize()` validates wake providers and initializes every Platform execution provider in declaration order. It starts no managed execution context.
3. `Start()` starts every declared infrastructure context transactionally in declaration order. Applications that need an absolute cross-resource order may use `InitializeInOrder<...>()` and `StartInOrder<...>()`, naming every topology resource exactly once. Semantic Task dispatch and Dedicated Thread activation remain unavailable until the complete Start succeeds.
4. Task dispatch is addressed by Pool identity. Dedicated Worker dispatch is addressed by its Task identity. Dedicated Threads are addressed by Thread identity.
5. `BeginShutdown()` is terminal. New Task admission/Thread activation is rejected; queued Tasks are cancelled, running Tasks receive cooperative cancellation, and Dedicated Threads receive stop requests.
6. Once `IsExecutionQuiescent()` is true, `FinalizeShutdown()` tears down every infrastructure context and publishes terminal completion. Applications can use `WaitForShutdown()`, `WaitForShutdownFor()` or `WaitForShutdownUntil()`.

The empty `ThreadingTopology<>` is valid. It owns no managed execution contexts, and context wake/router/resolver plus shutdown-registration storage compile away.

## Determinism

Capacities and topology are compile-time properties. Task records use compact capacity-derived indices, intrusive FIFO linkage and bounded callable/result storage. Managed contexts use dense topology-derived indices and structural resolution rather than a RAM pointer registry. Blocking operations are predicate-driven and use one reusable targeted wake mechanism per managed sequential context.

Threading consumes EDP-System, EDP-Platform and EDP-Clock. There are no optional ESPressio-library dependencies.

## Current implementation status

The v1 source implementation is in its completion tranche. It includes compile-time topology/requirements, bounded Task facilities, FIFO Worker-Lease scheduling, isolated single-Worker `DedicatedWorkerLease` resources, persistent restart-safe Dedicated Threads, dense targeted wake routing, canonical monotonic waits/sleep, transactional infrastructure Start/rollback, terminal cooperative shutdown and static runtime ownership.

Provider/target compilation, local suite execution and final deterministic resource-layout evidence remain completion gates. Runtime behavior should not be treated as released until those validation gates are recorded. No version mutation is performed during this development.

## Documentation

- [Architecture and execution model](./docs/ARCHITECTURE.MD)
- [Lifecycle, ownership and cancellation](./docs/LIFECYCLE.MD)
- [Deterministic resources](./docs/RESOURCES.MD)
- [Dependency graph](./DEPENDENCY_GRAPH.MD)
