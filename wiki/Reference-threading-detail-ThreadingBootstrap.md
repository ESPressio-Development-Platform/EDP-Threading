# src/threading/detail/ThreadingBootstrap.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/ThreadingBootstrap.hpp)

## Direct includes

- `utility`
- `../ThreadingTypes.hpp`
- `InfrastructureLifecycle.hpp`
- `ShutdownCoordinator.hpp`

## Documented declarations

### `TSpinLockProvider`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ThreadingBootstrap`.
- **Template parameter `TSpinLockProvider`:** Concrete Platform SpinLock provider Type protecting lifecycle publication.

```cpp
template<class TSpinLockProvider>
    class ThreadingBootstrap final
```

### `_lifecycle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Authoritative application-wide Threading lifecycle.

```cpp
InfrastructureLifecycle<TSpinLockProvider> _lifecycle;
```

### `Lifecycle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Authoritative lifecycle Type used by Threading Bootstrap.

```cpp
using Lifecycle = InfrastructureLifecycle<TSpinLockProvider>;
```

### `LifecycleState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns mutable access to the authoritative topology lifecycle for owned runtime wiring.

```cpp
Lifecycle& LifecycleState() noexcept
```

### `LifecycleState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns read-only access to the authoritative topology lifecycle.

```cpp
const Lifecycle& LifecycleState() const noexcept
```

### `CommitInitialization`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Commits Threading initialization after the owning application has initialized and
validated every statically realized facility/context/provider. This method starts no
execution context.

```cpp
ThreadingInitializationResult CommitInitialization() noexcept
```

### `Start`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts statically realized execution contexts in the exact argument order supplied
by the application composition. Failure is transactional and prevents operational
commit.
- **Template parameter `TResources`:** Concrete topology-owned runtime resource Types coordinated by Bootstrap.

```cpp
template<class... TResources>
            ThreadingStartResult Start(
                TResources&... resources
            ) noexcept
```

### `TFacility`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `Dispatch`.
- **Template parameter `TFacility`:** Task facility Type receiving lifecycle-gated dispatch.
- **Template parameter `TCallable`:** Callable Type being dispatched.

```cpp
template<class TFacility, class TCallable>
            auto Dispatch(
                TFacility& facility,
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            )
```

### `DispatchResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Typed dispatch result returned for the callable being dispatched.

```cpp
using DispatchResult = typename TFacility::template DispatchResultFor<TCallable>;
```

### `TDedicatedThreadRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `StartThread`.
- **Template parameter `TDedicatedThreadRuntime`:** Dedicated Thread runtime Type being lifecycle-gated.

```cpp
template<class TDedicatedThreadRuntime>
            ThreadStartResult StartThread(
                TDedicatedThreadRuntime& thread
            ) noexcept
```

### `TTaskResourceTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `BeginShutdown`.
- **Template parameter `TTaskResourceTuple`:** Tuple Type containing Task-execution resources.
- **Template parameter `TDedicatedThreadTuple`:** Tuple Type containing Dedicated Thread resources.

```cpp
template<class TTaskResourceTuple, class TDedicatedThreadTuple>
            ThreadingShutdownResult BeginShutdown(
                TTaskResourceTuple& taskResources,
                TDedicatedThreadTuple& dedicatedThreads
            ) noexcept
```

### `TTaskResourceTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `IsExecutionQuiescent`.
- **Template parameter `TTaskResourceTuple`:** Tuple Type containing Task-execution resources.
- **Template parameter `TDedicatedThreadTuple`:** Tuple Type containing Dedicated Thread resources.

```cpp
template<class TTaskResourceTuple, class TDedicatedThreadTuple>
            bool IsExecutionQuiescent(
                TTaskResourceTuple& taskResources,
                TDedicatedThreadTuple& dedicatedThreads
            ) noexcept
```

