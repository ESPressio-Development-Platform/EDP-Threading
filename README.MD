# ESPressio Threading

`EDP-Threading` provides the ESPressio Development Platform's deterministic execution-domain semantics for finite Tasks and persistent Dedicated Threads.

The v1 architecture is static and Composition-driven. Platform owns native execution/synchronization mechanisms; Threading owns Task/Thread lifecycle, cooperative cancellation/stop, waiting, scheduling policy and shutdown; EDP-BoundedTopology supplies the dependency-minimal compact index, membership-set and intrusive-FIFO mechanics used by those Threading semantics. EDP-Memory supplies the authoritative typed object-lifetime and ownership-transfer boundary for Task payloads. There is no runtime dependency injection, service locator, hidden heap fallback or unbounded EDP-controlled runtime growth.

## Public surface

Include:

```cpp
#include <ESPressio_Threading.hpp>
```

The public vocabulary includes:

- `TaskExecutionFacility<...>` — a finite Task record/callable/result facility backed by one or more declared `Worker<...>` contexts.
- `DedicatedWorkerLease<TaskIdentity,...>` — one isolated persistent Worker and bounded Task facility reserved for one critical Task identity.
- `DedicatedThread<ThreadIdentity,...>` — one persistent execution context whose semantic activation may be started, stopped and restarted.
- `ThreadingTopology<...>` — the complete statically declared Threading resource topology.
- `DedicatedThreadBinding<...>` / `BindDedicatedThread<ThreadIdentity>()` — binds application behaviour to a declared Dedicated Thread without moving behaviour into architectural entitlement.
- `StaticThreadingRuntime<...>` — the concrete statically owned Bootstrap/runtime façade.
- `BoundedWaitWakeProvider<...>` — fixed-capacity ordinary-context finite wait/wake slots for higher domains such as EDP-Command.
- `Thread<TIdentity>::Wake()` plus `ThreadContext::Wait()/WaitFor()/WaitUntil()` — exposes the existing topology-owned Dedicated-Thread wake mechanism without a second signal allocation.
- `OrdinaryMutexProvider<TIdentity,TMutexProvider>` — keyed ordinary-context non-recursive mutual exclusion backed by the selected Platform Mutex provider.

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

Capacities and topology are compile-time properties. Task records use EDP-BoundedTopology capacity-derived index storage, one-bit structural availability sets, shared intrusive FIFO topology and bounded callable/result storage. Managed contexts use dense topology-derived indices and structural resolution rather than a RAM pointer registry. Blocking operations are predicate-driven and use one reusable targeted wake mechanism per managed sequential context. A Dedicated Thread may wait indefinitely after exhausting all of its work because every Threading lifecycle/control path that can require attention, including cooperative stop and infrastructure termination, wakes that same context. External higher domains publish advisory work wakes through `Thread<TIdentity>::Wake()`; the Thread must recheck all work sources and stop state after waking.

Threading consumes EDP-System, EDP-Platform, EDP-Clock, EDP-BoundedTopology and EDP-Memory. There are no optional ESPressio-library dependencies.

## Current implementation status

The v1 source implementation includes compile-time topology/requirements, bounded Task facilities, FIFO Worker-Lease scheduling, isolated single-Worker `DedicatedWorkerLease` resources, persistent restart-safe Dedicated Threads, dense targeted wake routing, canonical monotonic waits/sleep, transactional infrastructure Start/rollback, terminal cooperative shutdown and static runtime ownership.

Current `main` validation is executed on the authoritative RPI400 appliance. The warnings-as-errors GCC host suite, Clang host suite, UBSan host suite, and coherent local-source PlatformIO Arduino/ESP-IDF demo builds pass for the Dedicated-Thread wake/wait and keyed ordinary-mutex tranche. AddressSanitizer is unavailable on this appliance because its primary allocator aborts during initialization before any Threading test reaches `main()`; that is recorded as an appliance/toolchain limitation rather than Threading PASS or failure. The retained-resource matrix and physical ESP32 runtime evidence were completed as part of the Event V1 integration validation.

The bounded-topology migration is now integrated into `main`. Its validation is complete: warnings-as-errors host execution, the separate Host foundation workflow, UBSan, and all fourteen coherent Arduino/ESP-IDF resource-measurement environments are green. Every validated `total` and `intrinsic` retained-RAM figure is byte-for-byte identical to the 23 September baseline; see [Deterministic resources](./docs/RESOURCES.MD). No version mutation is performed during this development.

## Local validation

Run the warnings-as-errors host suite with `bash tests/run-host-tests.sh` when EDP-System, EDP-Platform, EDP-Clock, EDP-BoundedTopology and EDP-Memory are cloned beside this repository. Run UBSan with `EDP_THREADING_SANITIZER_MODE=undefined bash tests/run-host-tests.sh`.

Target resource measurements are built without flashing hardware from `tests/resource-measurements` using `bash ./run-resource-measurements.sh`.

For cross-repository feature validation, set `EDP_THREADING_RESOURCE_LOCAL_ROOT` to the directory containing the sibling EDP repositories. All participating production libraries are header-only, so this mode bypasses PlatformIO Library Manager and injects the required sibling `src` directories directly into every measurement environment. That prevents transitive package manifests from mixing `main` with the branch under validation. In coherent-local mode the harness includes only the exact ESP-IDF ExecutionContext and SpinLock provider headers it consumes, avoiding unrelated provider-domain dependencies:

```bash
EDP_THREADING_RESOURCE_LOCAL_ROOT=~/DevProjects/ESPressio_Repositories \
    bash tests/resource-measurements/run-resource-measurements.sh
```

Without that variable, the committed `platformio.ini` retains the normal remote `main` dependency baseline.

A concrete ESP32 integration demo is provided in `demos/basic-threading` for Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF. It exercises initialization, transactional infrastructure Start, bounded Task execution/result extraction, an indefinitely waiting multi-purpose Dedicated Thread, external advisory wake delivery, keyed ordinary mutex acquisition/release, cooperative stop wake and terminal shutdown. Successful execution prints `EDP-Threading demo: PASS`.

## Dependencies

```text
EDP-Threading
├── EDP-System [mandatory]
├── EDP-Platform [mandatory]
├── EDP-Clock [mandatory]
├── EDP-BoundedTopology [mandatory]
└── EDP-Memory [mandatory]
```

- `EDP-System` — supplies the compile-time Composition Framework.
- `EDP-Platform` — supplies substitutable execution-context and synchronization abstractions.
- `EDP-Clock` — supplies the canonical monotonic timeline used for deadlines, waits and sleeping.
- `EDP-BoundedTopology` — supplies compact capacity-derived bounded identity, one-bit membership sets and non-owning intrusive FIFO topology without owning Threading semantics or payload lifetime.
- `EDP-Memory` — supplies authoritative typed construction, move-construction/ownership transfer and destruction for Task payloads so Threading does not bypass the Memory domain with direct C++ lifetime primitives.
- Optional ESPressio dependencies: none.

See the [dependency graph](./DEPENDENCY_GRAPH.MD) for the authoritative dependency-direction description.

## Documentation

- [Architecture and execution model](./docs/ARCHITECTURE.MD)
- [Lifecycle, ownership and cancellation](./docs/LIFECYCLE.MD)
- [Deterministic resources](./docs/RESOURCES.MD)
- [Dependency graph](./DEPENDENCY_GRAPH.MD)

## C++ Language Baseline

ESPressio requires C++20. Threading host tests, CI workflows, resource-measurement builds, and ESP32 integration demos all compile against that baseline. PlatformIO-compatible ESP32 development should use PIOArduino; repository-specific validated pins are retained where required for known toolchain compatibility.
