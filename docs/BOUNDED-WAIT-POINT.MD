# Bounded Wait Point

`BoundedWaitPoint<TSignalProvider>` is an allocation-free Threading-owned synchronization capability for ordinary non-ISR callers that are not required to be managed Threading execution contexts.

It encapsulates the Platform Signal provider and exposes only Threading vocabulary. `Wake()` latches a notification; `WaitFor()` consumes a finite provider timeout and translates provider results to `Woken`, `TimedOut`, or `ProviderFailure`. The underlying Signal contract provides wake-before-wait safety.

The Type is non-copyable and non-movable so its provider address/lifetime remains stable. Consumers must always re-check their authoritative predicate after a wake; the wake itself is not state.

This capability exists so domains such as EDP-Command do not instantiate Platform synchronization directly.

## Memory compliance

Task payload construction, move-construction and destruction in `detail/TaskPayloadAdapter.hpp` are routed through EDP-Memory `ObjectLifetime`. Direct `std::move` at that typed payload lifetime boundary has been removed.
