# src/threading/OrdinaryMutexProvider.hpp

**Primary classification:** PUBLIC COMPOSITION / SYNCHRONIZATION API

**Source baseline:** `5e276063a24790fb77d7366136e1e94db2c5ee3d`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/5e276063a24790fb77d7366136e1e94db2c5ee3d/src/threading/OrdinaryMutexProvider.hpp)

## Purpose and boundary

Provides a keyed ordinary-context non-recursive mutual-exclusion capability owned by Threading while delegating the native lock mechanism to the selected EDP-Platform Mutex provider. This lets higher domains depend on Threading rather than Platform synchronization vocabulary. No ISR or recursive-lock surface is exposed.

## Declaration inventory

### `template<class TIdentity> struct OrdinaryMutex`

Exclusive Threading Composition capability. `Identity` is the semantic key separating independent mutex instances.

### `OrdinaryMutexAcquireResult`

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
