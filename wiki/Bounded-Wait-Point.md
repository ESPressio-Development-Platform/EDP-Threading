# Bounded Wait Point

`BoundedWaitPoint<TSignalProvider>` gives ordinary non-ISR callers a reusable allocation-free wait/wake capability without requiring them to be Threading-managed execution contexts.

It owns and encapsulates its Platform Signal provider. Public waits use EDP-Clock `Duration` or `MonotonicTimestamp`; Threading converts those to the provider timeout internally. `Wake()` latches the provider notification. Consumers must re-check their authoritative predicate after every wake.

This is the synchronization boundary intended for domains such as EDP-Command; those domains must not instantiate Platform Signal directly.
