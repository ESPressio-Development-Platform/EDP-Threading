# Dependency Contracts

EDP-Threading depends on **EDP-System**, **EDP-Platform** and **EDP-Clock**.

## EDP-System

Threading uses EDP-System compile-time machinery for its domain/provider representation, but it also has a Threading-specific topology/requirement vocabulary (`TaskPoolRequirement`, `DedicatedThreadRequirement`, `DedicatedWorkerRequirement`) evaluated against `ThreadingTopology`.

## EDP-Platform direct provider contracts

`StaticThreadingRuntime<TTopology,TBindings,TSignalProvider,TExecutionContextProvider,TSpinLockProvider,TMutexProvider>` takes four concrete Platform provider types:

- **Signal** — one reusable targeted wake per managed execution context;
- **ExecutionContext** — one native managed execution context per declared worker/thread;
- **SpinLock** — protects publication of the topology-wide infrastructure lifecycle;
- **Mutex** — protects resource-local state and bounded shutdown-wait registration.

The runtime validates the corresponding EDP-Platform internal provider-trait contracts. This is deliberately direct template/provider binding, not a hidden runtime registry.

ExecutionContext backing is derived from provider properties: control bytes/alignment, stack alignment and stack-allocation granularity. Semantic stack capacity is rounded to the provider's physical granularity.

## EDP-Clock

Timed waits use the canonical EDP-Clock monotonic timeline. `MonotonicWaitBudget` calls `Clock::MonotonicNow()`, so application Bootstrap must have successfully and permanently bound a lifetime-stable monotonic clock via `BindMonotonicClock()` before Threading timed operations execute.

## Ownership boundary

Threading owns semantic Task/Thread lifecycle and all bounded runtime structures. Platform owns native execution/synchronization semantics. Clock owns time. No dependency is allowed to invert those ownership boundaries.

> Dependency contract audit baseline: `7eeceff86d7e43dbc71b6e15cf08f565466424cf` (`main`).
