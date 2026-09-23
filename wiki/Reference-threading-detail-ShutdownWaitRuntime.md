# src/threading/detail/ShutdownWaitRuntime.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/ShutdownWaitRuntime.hpp)

## Direct includes

- `cstddef`
- `../ThreadingTypes.hpp`
- `InfrastructureLifecycle.hpp`
- `TaskFacilityRuntime.hpp`
- `WaitRegistration.hpp`

## Documented declarations

### `ShutdownWaitSynchronizationResult`

**Classification:** PRIVATE IMPLEMENTATION

Result of validating the synchronization provider used by shutdown waiting.

```cpp
enum class ShutdownWaitSynchronizationResult : std::uint8_t
```

### `ShutdownWaitRegistrationRemovalResult`

**Classification:** PRIVATE IMPLEMENTATION

Result of removing one published shutdown-wait registration.

```cpp
enum class ShutdownWaitRegistrationRemovalResult : std::uint8_t
```

### `ShutdownWaitWakeResult`

**Classification:** PRIVATE IMPLEMENTATION

Result of waking all contexts registered for terminal shutdown completion.

```cpp
enum class ShutdownWaitWakeResult : std::uint8_t
```

### `TInfrastructureLifecycle`

**Classification:** PRIVATE IMPLEMENTATION

Owns bounded terminal-shutdown wait registration and targeted wake behavior.

- **Template parameter `TInfrastructureLifecycle`:** Authoritative Threading infrastructure lifecycle Type.
- **Template parameter `TExecutionContextCapacity`:** Number of managed execution contexts eligible to wait.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider protecting waiter registration state.
- **Template parameter `TManagedContextRouter`:** Topology router used for context identity and targeted wake delivery.

```cpp
template<class TInfrastructureLifecycle, std::size_t TExecutionContextCapacity, class TMutexProvider, class TManagedContextRouter>
    class ShutdownWaitRuntime;
```

### `TInfrastructureLifecycle`

**Classification:** PRIVATE IMPLEMENTATION

Empty-topology shutdown wait specialization retaining only the lifecycle reference.

- **Template parameter `TInfrastructureLifecycle`:** Authoritative Threading infrastructure lifecycle Type.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type selected by Bootstrap.
- **Template parameter `TManagedContextRouter`:** Empty-topology router Type selected by Bootstrap.

```cpp
template<class TInfrastructureLifecycle, class TMutexProvider, class TManagedContextRouter>
    class ShutdownWaitRuntime<
        TInfrastructureLifecycle,
```

### `_lifecycle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning reference to the application-wide Threading lifecycle.

```cpp
TInfrastructureLifecycle* _lifecycle;
```

### `IsComplete`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether the authoritative Threading lifecycle reached terminal shutdown completion.

```cpp
bool IsComplete() const noexcept
```

### `ShutdownWaitRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Binds terminal-shutdown waiting to the authoritative lifecycle and managed-context router.

```cpp
explicit ShutdownWaitRuntime(
                TInfrastructureLifecycle& lifecycle,
                TManagedContextRouter&
            ) noexcept :
                _lifecycle(&lifecycle) {}
```

### `ValidateSynchronization`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Reports that the empty topology requires no synchronization provider validation.

```cpp
ShutdownWaitSynchronizationResult ValidateSynchronization() noexcept
```

### `Wait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Observes terminal shutdown completion without blocking for an empty topology.

```cpp
ShutdownWaitResult Wait()
```

### `WaitFor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Observes terminal shutdown completion for the empty topology; duration does not require blocking.

```cpp
ShutdownWaitResult WaitFor(
                Duration
            )
```

### `WaitUntil`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Observes terminal shutdown completion for the empty topology; deadline does not require blocking.

```cpp
ShutdownWaitResult WaitUntil(
                MonotonicTimestamp
            )
```

### `WakeCompleted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Performs no wake work because an empty topology owns no managed contexts.

```cpp
ShutdownWaitWakeResult WakeCompleted() noexcept
```

### `TInfrastructureLifecycle`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ShutdownWaitRuntime`.
- **Template parameter `TInfrastructureLifecycle`:** Authoritative Threading infrastructure lifecycle Type.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TInfrastructureLifecycle, std::size_t TExecutionContextCapacity, class TMutexProvider, class TManagedContextRouter>
    class ShutdownWaitRuntime final
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Compact Type used to identify one managed execution context.

```cpp
using ContextIndex =
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type;
```

### `Registration`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Concrete bounded registration Type stored by this wait surface.

```cpp
using Registration = ShutdownWaitRegistration<ContextIndex>;
```

### `_lifecycle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Authoritative global lifecycle.

```cpp
TInfrastructureLifecycle* _lifecycle;
```

### `_router`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning topology targeted-wake router.

```cpp
TManagedContextRouter* _router;
```

### `Registration`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Target-owned bounded terminal-shutdown wait registrations.

```cpp
RegistrationSet<
                Registration,
```

### `_mutex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Serializes registration publication/removal with terminal wake discovery.

```cpp
TMutexProvider _mutex;
```

### `AcquireLock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Acquires the shutdown-wait registration mutex indefinitely.

```cpp
ESPressio::Platform::Synchronization::LockAcquireResult AcquireLock() noexcept
```

### `ReleaseLock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Releases the shutdown-wait registration mutex.

```cpp
ESPressio::Platform::Synchronization::LockReleaseResult ReleaseLock() noexcept
```

### `IsComplete`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether the authoritative Threading lifecycle reached terminal shutdown completion.

```cpp
bool IsComplete() const noexcept
```

### `Unregister`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Removes one previously published terminal-shutdown wait registration.

```cpp
ShutdownWaitRegistrationRemovalResult Unregister(
                std::size_t registrationIndex
            ) noexcept
```

### `WaitWithBudget`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Waits for terminal shutdown completion using one canonical monotonic wait budget.

```cpp
ShutdownWaitResult WaitWithBudget(
                const MonotonicWaitBudget& budget
            )
```

### `ShutdownWaitRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Binds terminal-shutdown waiting to the authoritative lifecycle and managed-context router.

```cpp
explicit ShutdownWaitRuntime(
                TInfrastructureLifecycle& lifecycle,
                TManagedContextRouter& router
            ) noexcept :
                _lifecycle(&lifecycle),
```

### `ValidateSynchronization`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Validates that the configured Mutex provider can be acquired and released.

```cpp
ShutdownWaitSynchronizationResult ValidateSynchronization() noexcept
```

### `Wait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waits indefinitely for terminal shutdown completion.

```cpp
ShutdownWaitResult Wait()
```

### `WaitFor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waits for terminal shutdown completion for at most the supplied duration.

```cpp
ShutdownWaitResult WaitFor(
                Duration duration
            )
```

### `WaitUntil`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waits for terminal shutdown completion until the supplied monotonic deadline.

```cpp
ShutdownWaitResult WaitUntil(
                MonotonicTimestamp deadline
            )
```

### `WakeCompleted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Wakes every context registered against the non-restartable terminal shutdown predicate.

```cpp
ShutdownWaitWakeResult WakeCompleted()
```

