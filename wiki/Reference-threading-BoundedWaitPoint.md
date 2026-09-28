# src/threading/BoundedWaitPoint.hpp

**Primary classification:** PUBLIC ORDINARY-CONTEXT WAIT/WAKE API

**Source baseline:** `5e276063a24790fb77d7366136e1e94db2c5ee3d`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/5e276063a24790fb77d7366136e1e94db2c5ee3d/src/threading/BoundedWaitPoint.hpp)

## Purpose

`BoundedWaitPoint<TSignalProvider>` is a single reusable, allocation-free ordinary-context wait/wake point backed by one Platform Signal provider. It is independent of Dedicated Thread lifecycle and is intended for finite higher-domain wait/wake use where one explicit wait point is sufficient.

## Public declarations

- `BoundedWaitResult` — `Woken`, `TimedOut`, `ProviderFailure`.
- `BoundedWaitPoint<TSignalProvider>` — non-copyable/non-movable provider-owned synchronization state.
- `IsReady()` — probes provider readiness without retaining extra state.
- `WaitFor(Duration)` — finite relative canonical monotonic wait.
- `WaitUntil(MonotonicTimestamp)` — finite absolute canonical monotonic wait.
- `Wake()` — publishes one latched advisory notification.

The class owns no heap storage, counter, queue, ISR path, or Event semantics.
