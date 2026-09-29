# src/threading/ThreadingTypes.hpp

**Primary classification:** PUBLIC API / INTERNAL TYPE-ERASURE SUPPORT

**Source baseline:** `5e276063a24790fb77d7366136e1e94db2c5ee3d`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/5e276063a24790fb77d7366136e1e94db2c5ee3d/src/threading/ThreadingTypes.hpp)

## Purpose

Defines Threading result/state vocabularies, canonical time aliases and invocation-local Task/Thread context views.

## Public result/state declarations

Existing Task/Thread/lifecycle enums remain unchanged. Event-support work adds:

### `ThreadWakeResult`

- `Woken`
- `ProviderFailure`

### `ThreadWaitResult`

- `Woken`
- `TimedOut`
- `ProviderFailure`

The header also exports `ThreadPriority`, `ProcessorAffinity`, `Duration`, and `MonotonicTimestamp` aliases.

## Context declarations

### `TaskContext` — PUBLIC API

Retains the active Task record and cancellation predicate and exposes `IsCancellationRequested()`.

### `Detail::ThreadContextOperations` — INTERNAL TYPE-ERASURE TABLE

Static operations for `IsStopRequested`, `Wait`, `WaitFor`, and `WaitUntil`. One table is shared by all context views of the same concrete Dedicated Thread runtime Type.

### `ThreadContext` — PUBLIC API

Retains exactly two pointers: the concrete Dedicated Thread runtime and its static operations table. Public operations:

- `IsStopRequested() const noexcept`
- `Wait() noexcept` — waits indefinitely for the topology-owned wake signal.
- `WaitFor(Duration) noexcept` — relative canonical-time wait.
- `WaitUntil(MonotonicTimestamp) noexcept` — absolute canonical monotonic wait.

A wake is not a counted work item. The caller must recheck all authoritative work/control state after return.
