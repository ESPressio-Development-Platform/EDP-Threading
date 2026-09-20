# EDP-Threading

EDP-Threading provides the ESPressio Development Platform's deterministic Task and persistent Dedicated Thread execution domain.

The v1 architecture is static and Composition-driven. Application architecture declares finite execution resources at compile time; Bootstrap owns their concrete runtime realization. Threading does not use runtime dependency injection, service locators, heap-backed task registries, or dynamically growing execution-resource collections.

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

1. Construction owns the targeted wake topology, structural router, resource backing, Platform provider objects and Dedicated Thread callable bindings.
2. `Initialize()` validates wake providers and initializes every Platform execution provider in declaration order. It starts no managed execution context.
3. `Start()` starts every declared infrastructure context transactionally in declaration order. Semantic Task dispatch and Dedicated Thread activation remain unavailable until the complete Start succeeds.
4. Task dispatch is addressed by Pool identity. Dedicated Worker dispatch is addressed by its Task identity. Dedicated Threads are addressed by Thread identity.
5. `BeginShutdown()` is terminal. New Task admission/Thread activation is rejected; queued Tasks are cancelled, running Tasks receive cooperative cancellation, and Dedicated Threads receive stop requests.
6. Once `IsExecutionQuiescent()` is true, `FinalizeShutdown(shutdownWaitRuntime)` tears down every infrastructure context and publishes terminal completion. The supplied wait runtime is the bounded shutdown-completion wait surface used by the application/bootstrap layer.

The empty `ThreadingTopology<>` is valid and its context wake/router/resolver storage compiles away.

## Determinism

Capacities and topology are compile-time properties. Task records use compact capacity-derived indices, intrusive FIFO linkage and bounded callable/result storage. Managed contexts use dense topology-derived indices and structural resolution rather than a RAM pointer registry. Blocking operations are predicate-driven and use one reusable targeted wake mechanism per managed sequential context.

Threading consumes EDP-System, EDP-Platform and EDP-Clock. Platform owns native execution and synchronization mechanisms; Clock owns canonical monotonic time.

## Validation status

Implementation is still in the v1 development tranche. Source and compile-time validation surfaces are maintained in `tests/ThreadingFoundationTests.cpp`. Provider/target execution and final deterministic RAM measurements remain completion work and no version mutation is performed during this development.
