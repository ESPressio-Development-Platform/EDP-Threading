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

### `OrdinaryMutexReleaseResult`

- `Released`
- `ProviderFailure`

### `template<class TIdentity,class TMutexProvider> class OrdinaryMutexProvider`

Composition provider offering exactly `OrdinaryMutex<TIdentity>`. It owns exactly one concrete Platform non-recursive Mutex provider and validates that provider through `MutexProviderTraits`. Copy/move are deleted.

Public operations:

- `Acquire() noexcept` — waits indefinitely in ordinary execution context and maps the Platform result to `OrdinaryMutexAcquireResult`.
- `Release() noexcept` — releases ownership and maps the Platform result to `OrdinaryMutexReleaseResult`.

No hidden allocation, registry, timeout policy, ISR operation or fallback mutex exists.
