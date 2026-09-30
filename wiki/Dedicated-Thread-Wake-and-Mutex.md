# Dedicated Thread Wake and Ordinary Mutex

A Dedicated Thread owns one reusable topology signal for its full infrastructure lifetime. `Thread<TIdentity>::Wake()` publishes to that signal. `ThreadContext::Wait()`, `WaitFor()` and `WaitUntil()` wait on the same signal from inside the Thread callable. Start, cooperative stop and infrastructure termination also signal the same route.

A wake is not a counted work token. It means that the Thread must inspect its authoritative stop state and all work sources it owns. This is why a multi-purpose Dedicated Thread may safely wait indefinitely while idle: every path that makes work/control state relevant must wake it.

`OrdinaryMutex<TIdentity>` is a keyed exclusive Threading Composition capability. `OrdinaryMutexProvider<TIdentity,TMutexProvider>` owns one Platform non-recursive Mutex and maps indefinite ordinary-context acquire/release into Threading-owned strong results. It has no ISR or recursive-lock surface.

`FailClosedOrdinaryMutex<TIdentity>` is the stronger keyed capability for consumers which cannot safely continue after mutex-provider failure. `FailClosedOrdinaryMutexProvider<TIdentity,TMutexProvider,TFailureSpinLockProvider>` owns the selected non-recursive Mutex plus an independent SpinLock-protected sticky failure byte. Healthy acquisition uses finite native waits internally and rechecks the sticky latch between waits, so a caller already blocked behind a mutex can observe a failure discovered by another caller instead of waiting forever. Public acquire/release results distinguish healthy success, an already-latched `FailedState`, and the operation that first observes `ProviderFailure`. The latch is terminal for the provider object lifetime.
