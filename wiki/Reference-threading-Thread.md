# src/threading/Thread.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `5e276063a24790fb77d7366136e1e94db2c5ee3d`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/5e276063a24790fb77d7366136e1e94db2c5ee3d/src/threading/Thread.hpp)

## Purpose and invariants

`Thread<TThreadIdentity>` is a move-only non-owning control handle for one topology-owned Dedicated Thread. Moving the handle never moves the Thread resource. A moved-from handle remains safe and reports conservative typed outcomes. Wake publication is advisory: the Thread must recheck authoritative stop/work state.

## Direct includes

- `utility`
- `ThreadingTypes.hpp`

## Declaration inventory

### `Detail::ThreadHandleOperations` — INTERNAL TYPE-ERASURE TABLE

Static per-concrete-runtime operations: `State`, `Start`, `RequestStop`, `Wake`, `Join`, `JoinFor`, `JoinUntil`. The newly exported `Wake` entry returns `ThreadWakeResult` and targets the same managed-context signal already owned by the Dedicated Thread.

### `template<class TThreadIdentity> class Thread` — PUBLIC API

Retains exactly one opaque resource pointer and one pointer to the static operation table. Copying is deleted; moving transfers only these non-owning pointers.

Public operations:

- `IsValid() const noexcept` — whether the handle is still bound.
- `State() const noexcept` — current public Thread lifecycle state.
- `Start() noexcept` — starts a semantic activation.
- `RequestStop() noexcept` — publishes cooperative stop and wakes the Thread through the runtime.
- `Wake() noexcept` — publishes one advisory wake; invalid/moved-from handles return `ThreadWakeResult::ProviderFailure`.
- `Join()` — indefinite activation join.
- `JoinFor(Duration)` — finite relative join.
- `JoinUntil(MonotonicTimestamp)` — absolute canonical monotonic join.

No operation allocates or creates another wake primitive.
