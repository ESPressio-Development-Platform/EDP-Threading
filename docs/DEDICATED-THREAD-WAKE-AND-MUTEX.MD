# Dedicated Thread Wake and Ordinary Mutex

## Dedicated Thread wake ownership

Every managed sequential Threading context already owns exactly one reusable latched wake signal. A `DedicatedThread` uses that same signal for semantic activation, cooperative stop, infrastructure termination, Join coordination, and higher-domain work notification. `Thread<TIdentity>::Wake()` publishes an advisory wake to that existing signal; it does not allocate or count another token.

Inside the Dedicated Thread callable, `ThreadContext::Wait()` may block indefinitely. `WaitFor(Duration)` and `WaitUntil(MonotonicTimestamp)` expose the same signal with canonical EDP-Clock budgets. A wake means only that the Thread should recheck authoritative state and every work source it owns. Multi-purpose Threads therefore service Event work and unrelated application work after the same wake.

`RequestStop()` first publishes authoritative stop state and then wakes the Thread. Infrastructure termination likewise wakes the persistent context. A Thread may consequently wait forever when idle without making cooperative shutdown unreachable.

These APIs are ordinary-context operations. They make no ISR-safety promise.

## Keyed ordinary mutex

`OrdinaryMutex<TIdentity>` is an exclusive Threading Composition capability whose identity separates unrelated consumers. `OrdinaryMutexProvider<TIdentity,TMutexProvider>` owns one selected Platform non-recursive Mutex provider and exposes only indefinite ordinary-context `Acquire()` and `Release()` operations with strong Threading-owned result enums.

The adapter exists so higher domains such as Event can depend on a Threading synchronization contract without depending directly on Platform synchronization vocabulary. It adds no global registry, recursion, ISR surface, hidden allocation, or fallback resource.

## Fail-closed keyed ordinary mutex

`FailClosedOrdinaryMutex<TIdentity>` is a separate exclusive Threading capability for consumers whose correctness requires a sticky terminal synchronization-failure state. `FailClosedOrdinaryMutexProvider<TIdentity,TMutexProvider,TFailureSpinLockProvider>` owns the selected Platform non-recursive Mutex together with an independent Platform SpinLock protecting one sticky failure byte. It remains ordinary-context only and non-recursive.

Healthy `Acquire()` uses finite native-mutex wait slices and rechecks the failure latch between slices. The public operation therefore still waits indefinitely while healthy, while a caller that was already waiting can observe a failure latched by another caller instead of remaining blocked forever behind a native mutex whose ownership integrity is uncertain. The target failure-observation interval is 1 ms, rounded upward to the selected Mutex provider's advertised wait resolution.

`Acquire()` distinguishes `Acquired`, previously latched `FailedState`, and the call which first observes a native `ProviderFailure`. `Release()` similarly distinguishes `Released`, `FailedState`, and `ProviderFailure`. Any native acquire/release provider failure latches terminal failure before returning. `IsFailed()` reads that latch under the independent SpinLock. The latch is never cleared during the provider object's lifetime.

The SpinLock is intentionally used instead of assuming a lock-free C++ atomic exists on every target. This keeps Platform mechanics inside Threading and allows higher domains such as RemoteSystem to consume one Threading provider without taking a direct Platform dependency. The failure SpinLock protects only a Boolean latch and must be held for no external work. No ISR surface is exposed by the Threading adapter.
