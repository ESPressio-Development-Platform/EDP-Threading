# Dedicated Thread Wake and Ordinary Mutex

## Dedicated Thread wake ownership

Every managed sequential Threading context already owns exactly one reusable latched wake signal. A `DedicatedThread` uses that same signal for semantic activation, cooperative stop, infrastructure termination, Join coordination, and higher-domain work notification. `Thread<TIdentity>::Wake()` publishes an advisory wake to that existing signal; it does not allocate or count another token.

Inside the Dedicated Thread callable, `ThreadContext::Wait()` may block indefinitely. `WaitFor(Duration)` and `WaitUntil(MonotonicTimestamp)` expose the same signal with canonical EDP-Clock budgets. A wake means only that the Thread should recheck authoritative state and every work source it owns. Multi-purpose Threads therefore service Event work and unrelated application work after the same wake.

`RequestStop()` first publishes authoritative stop state and then wakes the Thread. Infrastructure termination likewise wakes the persistent context. A Thread may consequently wait forever when idle without making cooperative shutdown unreachable.

These APIs are ordinary-context operations. They make no ISR-safety promise.

## Keyed ordinary mutex

`OrdinaryMutex<TIdentity>` is an exclusive Threading Composition capability whose identity separates unrelated consumers. `OrdinaryMutexProvider<TIdentity,TMutexProvider>` owns one selected Platform non-recursive Mutex provider and exposes only indefinite ordinary-context `Acquire()` and `Release()` operations with strong Threading-owned result enums.

The adapter exists so higher domains such as Event can depend on a Threading synchronization contract without depending directly on Platform synchronization vocabulary. It adds no global registry, recursion, ISR surface, hidden allocation, or fallback resource.
