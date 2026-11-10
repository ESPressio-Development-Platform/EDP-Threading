# src/threading/OrdinaryMutexProvider.hpp

**Primary classification:** PUBLIC COMPOSITION / SYNCHRONIZATION API

**Source baseline:** `269d3dbc9b2143f5bd3046182d20db57d97e32a3`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/269d3dbc9b2143f5bd3046182d20db57d97e32a3/src/threading/OrdinaryMutexProvider.hpp)

## Purpose and boundary

Provides a keyed ordinary-context non-recursive mutual-exclusion capability owned by Threading while delegating the native lock mechanism to the selected EDP-Platform Mutex provider. This lets higher domains depend on Threading rather than Platform synchronization vocabulary. No ISR or recursive-lock surface is exposed.

## Declaration inventory

### `template<class TIdentity> struct OrdinaryMutex`

Exclusive Threading Composition capability. `Identity` is the semantic key separating independent mutex instances.

### `OrdinaryMutexAcquireResult`

**Classification:** PUBLIC API. The enum values are `Acquired = 0` (native lock held by caller), `ProviderFailure = 1` (native provider did not satisfy its contract, no lock ownership claim) and `TimedOut = 2` (NoWait contention or genuinely expired relative native wait, distinct from provider failure). These values do not imply a Clock-owned absolute deadline by themselves.

### `OrdinaryMutexAcquireResult` (value listing)

- `Acquired`
- `ProviderFailure`
- `TimedOut` — contended nonblocking or finite native timeout; distinct from provider failure.

### `OrdinaryMutexReleaseResult`

- `Released`
- `ProviderFailure`

### `template<class TIdentity,class TMutexProvider> class OrdinaryMutexProvider`

Composition provider offering exactly `OrdinaryMutex<TIdentity>`. It owns exactly one concrete Platform non-recursive Mutex provider and validates that provider through `MutexProviderTraits`. Copy/move are deleted.

Public operations:

- `Acquire() noexcept` — indefinitely waits via the existing native Forever policy.
- `TryAcquire() noexcept` — immediate native NoWait acquisition, reports `TimedOut` if contended.
- `Acquire(WaitTimeout) noexcept` — caller-selected relative native timeout, reports `TimedOut` separately from `ProviderFailure`. Does not acquire or own a competing Monotonic timebase.
- `Release() noexcept` — releases ownership and maps the Platform result to `OrdinaryMutexReleaseResult`.

No hidden allocation, registry, timeout policy, ISR operation or fallback mutex exists.

The caller is responsible for translating one EDP-Clock absolute deadline into a remaining relative `WaitTimeout` on each attempt. Native provider resolution and quality must be validated. This capability does not fix EDP-Memory's independently acquired internal pool mutex.


## Internal state, limits and consumer obligations

The private alias `MutexTraits` validates the native Platform Mutex provider's contract during instantiation. The private field `_mutex` is the **sole authoritative native synchronization object** for this keyed capability, owned in-place for the adapter's lifetime and never copied/moved. The public `Identity` alias distinguishes semantically independent exclusive Composition capability instances. All calls are ordinary-context only; no ISR or recursive acquisition is supported. `Acquire(timeout)` passes one caller-selected Platform relative wait policy to the native owner, maps Acquired/TimedOut/ProviderFailure explicitly, and returns provider failure for unrecognized native outcomes. `TryAcquire()` delegates to `Acquire(NoWait())`; parameterless `Acquire()` delegates to `Acquire(Forever())`. `Release()` maps native Released to the Threading typed result, otherwise ProviderFailure. No new heap allocation, temporary resource, history or hidden timebase is retained. Callers must compute remaining EDP-Clock budget and re-evaluate absolute deadlines around each bounded operation. Failing native synchronization does **not** guarantee the caller recovered ownership or safety elsewhere; Memory's internal waiter queue remains a separate owner.
