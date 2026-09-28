# src/threading/BoundedWaitWakeProvider.hpp

**Primary classification:** PUBLIC COMPOSITION / BOUNDED WAIT-WAKE API

**Source baseline:** `5e276063a24790fb77d7366136e1e94db2c5ee3d`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/5e276063a24790fb77d7366136e1e94db2c5ee3d/src/threading/BoundedWaitWakeProvider.hpp)

## Purpose

`BoundedWaitWakeProvider<TSignalProvider,TCapacity>` supplies a fixed compile-time number of reusable ordinary-context wait/wake slots through the Threading Composition domain. One Platform Signal provider is retained per slot; no slot storage grows at runtime.

## Public declarations

- `BoundedWaitWake` — shared Threading capability.
- `BoundedWaitWakeCapacity` — compile-time capacity property.
- `BoundedWaitWakeResult` — `Woken`, `TimedOut`, `ProviderFailure`, `InvalidSlot`.
- `BoundedWaitWakeProvider<TSignalProvider,TCapacity>` — fixed-capacity provider; `TCapacity > 0`.
- `Capacity` — exact compile-time slot count.
- `WaitFor(slot,Duration)` and `WaitUntil(slot,MonotonicTimestamp)` — finite waits only.
- `Wake(slot)` — publishes one latched advisory notification.

This provider remains useful for bounded higher-domain record waits such as EDP-Command. Dedicated Thread Event wake behavior instead uses the Thread's topology-owned managed-context signal exposed by `Thread::Wake()` and `ThreadContext::Wait*()`.
