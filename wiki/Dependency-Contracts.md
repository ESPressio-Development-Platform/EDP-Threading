# Dependency Contracts

EDP-Threading depends on **EDP-System**, **EDP-Platform**, **EDP-Clock**, **EDP-BoundedTopology** and **EDP-Memory**.

## EDP-System

Threading uses EDP-System compile-time machinery for its domain/provider representation, while retaining a Threading-specific topology/requirement vocabulary evaluated against `ThreadingTopology`.

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

## EDP-BoundedTopology

Threading consumes three domain-neutral mechanics:

- `BoundedIndex` storage selection for compact Task-record, managed-context and scratch scalar representations;
- `BoundedIndexSet` for Task-record and Worker availability membership;
- `IntrusiveQueue` for queued Task FIFO ordering.

Threading does not delegate Task admission, Worker grants, cancellation, reclamation, scheduling, synchronization, wait semantics, or payload ownership to BoundedTopology. The shared queue stores only head/tail; each `TaskRecord` still owns the lifecycle-reused next-link representation.

No provider object or runtime lifetime is supplied by EDP-BoundedTopology; the dependency is header-only and allocation/synchronization-free.

## Ownership boundary

Threading owns semantic Task/Thread lifecycle and Threading-specific bounded runtime state meaning. Platform owns native execution/synchronization semantics. Clock owns time. BoundedTopology owns only the reusable finite-index topology mechanics. No dependency is allowed to invert those ownership boundaries.

## EDP-Memory

EDP-Memory remains the mandatory typed object-lifetime and ownership-transfer boundary used by Threading Task payloads. Event-support wake/mutex additions do not change that edge.

## Higher-domain synchronization boundary

Higher domains may publish work to a Dedicated Thread through `Thread<TIdentity>::Wake()` and may consume the keyed `OrdinaryMutex<TIdentity>` Threading capability. These surfaces deliberately keep higher domains from depending on Platform Signal/Mutex vocabulary directly.

`FailClosedOrdinaryMutexProvider` reuses the existing mandatory EDP-Platform dependency twice: a non-recursive `Mutex` for consumer serialization and an independent `SpinLock` for sticky failure publication. This adds no new repository dependency edge. The SpinLock is intentionally independent so mutex-provider failure never has to synchronize its own terminal-state publication.
