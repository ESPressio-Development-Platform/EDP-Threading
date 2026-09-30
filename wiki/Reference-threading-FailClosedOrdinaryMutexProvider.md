# src/threading/FailClosedOrdinaryMutexProvider.hpp

**Primary classification:** PUBLIC COMPOSITION / SYNCHRONIZATION API

[Open current source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/main/src/threading/FailClosedOrdinaryMutexProvider.hpp)

## Purpose and boundary

This header provides Threading-owned keyed ordinary-context non-recursive mutual exclusion for consumers which must enter a sticky terminal failure state if the native mutex provider loses integrity.

The implementation deliberately composes existing Platform `Mutex` and `SpinLock` contracts. It does not expose Platform result vocabulary through the public Threading API, create a service locator, allocate dynamically, or add an ISR-facing Threading surface.

## `FailClosedOrdinaryMutex<TIdentity>`

**Classification:** PUBLIC COMPOSITION CAPABILITY.

An exclusive Threading capability. `TIdentity` is the compile-time semantic key separating this mutex from every unrelated fail-closed ordinary mutex. The nested `Identity` alias republishes that key for inspection.

It is intentionally distinct from `OrdinaryMutex<TIdentity>`: a provider of the ordinary capability does not satisfy a requirement for the fail-closed capability.

## Result enums

### `FailClosedOrdinaryMutexAcquireResult`

- `Acquired` — healthy ownership of the underlying non-recursive mutex was obtained.
- `FailedState` — the sticky failure latch was already set, or became set while this caller was waiting; no healthy ownership is returned.
- `ProviderFailure` — this operation directly observed a native mutex acquire/release provider failure and latched terminal failure before returning.

### `FailClosedOrdinaryMutexReleaseResult`

- `Released` — healthy ownership was released and the provider remains healthy.
- `FailedState` — release itself succeeded, but terminal failure was already or concurrently latched.
- `ProviderFailure` — the underlying mutex release did not report `Released`; terminal failure is latched.

These enums deliberately distinguish the operation that first discovers infrastructure failure from later calls which observe the already-terminal provider.

## `FailClosedOrdinaryMutexProvider<TIdentity,TMutexProvider,TFailureSpinLockProvider>`

**Classification:** PUBLIC COMPOSITION PROVIDER.

Template parameters:

- `TIdentity` — semantic identity used by the offered `FailClosedOrdinaryMutex<TIdentity>` capability.
- `TMutexProvider` — concrete Platform non-recursive Mutex provider. It is validated through `MutexProviderTraits`, owned by value, and supplies finite timed acquisition plus ordinary release.
- `TFailureSpinLockProvider` — concrete Platform SpinLock provider. It is validated through `SpinLockProviderTraits`, owned by value, and protects only the sticky failure byte independently of the main mutex.

The provider offers exactly `FailClosedOrdinaryMutex<TIdentity>` through the Threading Domain. It is default constructible when its concrete providers are, and all copy/move operations are deleted because synchronization resources must keep stable ownership and address semantics.

`Identity` republishes `TIdentity`. `FailureObservationIntervalNanoseconds` exposes the actual finite native-mutex acquisition slice after rounding the 1 ms target upward to the concrete mutex's advertised wait resolution.

### Retained state and private helpers

- `_mutex` — authoritative native mutual-exclusion resource. It protects consumer critical sections but never protects publication of its own failure state.
- `_failureLock` — independent short Platform SpinLock. It protects only `_failed`; no external work or native mutex operation runs while it is held.
- `_failed` — sticky Boolean terminal-failure fact. It begins false, may transition to true exactly by failure publication, and never returns to false during provider lifetime.
- `TargetFailureObservationNanoseconds` — 1 ms design target for noticing failure while already waiting.
- `MutexWaitResolutionNanoseconds` — concrete provider's advertised finite-wait resolution.
- `WaitSliceNanoseconds` — target rounded upward to a supported native wait boundary.
- `ReleaseFailureLock()` — releases the independent SpinLock. An impossible ordinary-context release failure terminates rather than silently exposing unsynchronized `_failed` state.
- `MarkFailed()` — sets `_failed=true` while holding `_failureLock`; safe even when `_mutex` itself cannot be trusted.

No waiter list, cached owner identity, failure count, dynamic allocation, recursive state or per-caller retained state exists.

### `IsFailed() const noexcept`

Reads the sticky terminal state under `_failureLock`. It never acquires `_mutex`, so it remains meaningful when the main mutex provider has failed. Ordinary context only; no ISR contract is exposed.

### `Acquire() noexcept`

If terminal failure is already latched, returns `FailedState` without entering the native mutex. Otherwise it acquires the native mutex in finite `WaitSliceNanoseconds` intervals. A timeout simply causes the latch to be rechecked and healthy waiting to continue. Native `ProviderFailure` latches failure and returns `ProviderFailure`.

If native ownership is obtained after another caller has already latched failure, the provider immediately releases that ownership and returns `FailedState`; if that cleanup release itself fails, it returns `ProviderFailure`. Thus `Acquired` is returned only while the provider is still healthy.

The operation has no finite public timeout: while healthy it waits indefinitely. The finite internal slices exist only so already-blocked callers can observe terminal failure.

### `Release() noexcept`

Releases native ownership. Any native non-`Released` outcome latches failure and returns `ProviderFailure`. A successful native release returns `FailedState` if failure was already/concurrently latched, otherwise `Released`.

Calling code must obey the selected non-recursive Mutex ownership rules. This Threading adapter does not make invalid ownership safe or recursive.
