# src/threading/detail/InfrastructureLifecycle.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/InfrastructureLifecycle.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `tuple`
- `utility`
- `../ThreadingTypes.hpp`

## Documented declarations

### `TSpinLockProvider`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `InfrastructureLifecycle`.
- **Template parameter `TSpinLockProvider`:** Concrete Platform SpinLock provider Type protecting lifecycle publication.

```cpp
template<class TSpinLockProvider>
    class InfrastructureLifecycle final
```

### `_state`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

One-byte application-wide Threading lifecycle state.

```cpp
std::uint8_t _state =
                static_cast<std::uint8_t>(
                    InfrastructureState::Uninitialized
                );
```

### `_stateLock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Short-duration topology-wide synchronization protecting lifecycle publication.

```cpp
mutable TSpinLockProvider _stateLock;
```

### `ReadState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Reads the authoritative lifecycle state under the topology-wide SpinLock.

```cpp
InfrastructureState ReadState() const noexcept
```

### `PublishState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Publishes one authoritative lifecycle transition under the topology-wide SpinLock.

```cpp
ESPressio::Platform::Synchronization::SpinLockReleaseResult PublishState(
                InfrastructureState state
            ) noexcept
```

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `StartNext`.
- **Template parameter `TTuple`:** Tuple Type containing the resources traversed by this helper.
- **Template parameter `TIndex`:** Compile-time tuple/resource index used by the recursive traversal.

```cpp
template<std::size_t TIndex, class TTuple>
            static ESPressio::Platform::Execution::ExecutionStartResult StartNext(
                TTuple& resources,
                std::size_t& startedCount
            ) noexcept
```

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `WakeStarted`.
- **Template parameter `TTuple`:** Tuple Type containing the resources traversed by this helper.
- **Template parameter `TIndex`:** Compile-time tuple/resource index used by the recursive traversal.

```cpp
template<std::size_t TIndex, class TTuple>
            static ESPressio::Platform::Synchronization::SignalNotifyResult WakeStarted(
                TTuple& resources,
                std::size_t startedCount
            ) noexcept
```

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `JoinStarted`.
- **Template parameter `TTuple`:** Tuple Type containing the resources traversed by this helper.
- **Template parameter `TIndex`:** Compile-time tuple/resource index used by the recursive traversal.

```cpp
template<std::size_t TIndex, class TTuple>
            static ESPressio::Platform::Execution::ExecutionJoinResult JoinStarted(
                TTuple& resources,
                std::size_t startedCount
            ) noexcept
```

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `DestroyAll`.
- **Template parameter `TTuple`:** Tuple Type containing the resources traversed by this helper.
- **Template parameter `TIndex`:** Compile-time tuple/resource index used by the recursive traversal.

```cpp
template<std::size_t TIndex, class TTuple>
            static ESPressio::Platform::Execution::ExecutionDestroyResult DestroyAll(
                TTuple& resources
            ) noexcept
```

### `InfrastructureLifecycle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates the lifecycle in its Uninitialized state.

```cpp
InfrastructureLifecycle() noexcept = default;
```

### `State`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the synchronized application-wide Threading infrastructure state.

```cpp
InfrastructureState State() const noexcept
```

### `CanActivate`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether semantic Task/Thread activation is currently permitted.

```cpp
bool CanActivate() const noexcept
```

### `ShouldTerminate`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether persistent managed contexts must cooperatively terminate.

```cpp
bool ShouldTerminate() const noexcept
```

### `CanActivateThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Type-erased lifecycle predicate used by statically owned resources before semantic activation.

```cpp
static bool CanActivateThunk(
                const void* context
            ) noexcept
```

### `ShouldTerminateThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Type-erased lifecycle predicate used by managed contexts to observe rollback or shutdown.

```cpp
static bool ShouldTerminateThunk(
                const void* context
            ) noexcept
```

### `CommitInitialization`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Commits the global initialization barrier after every topology resource initialized successfully.

```cpp
ThreadingInitializationResult CommitInitialization() noexcept
```

### `Start`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts all supplied topology resources transactionally in argument order.
- **Template parameter `TResources`:** Concrete topology-owned resource Types participating in infrastructure Start.

```cpp
template<class... TResources>
            ThreadingStartResult Start(
                TResources&... resources
            ) noexcept
```

### `BeginShutdown`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Begins the terminal application-wide shutdown transition without blocking for execution completion.

```cpp
ThreadingShutdownResult BeginShutdown() noexcept
```

### `PublishShutdownComplete`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Publishes the terminal ShutdownComplete state after all managed contexts are destroyed.

```cpp
ESPressio::Platform::Synchronization::SpinLockReleaseResult PublishShutdownComplete() noexcept
```

