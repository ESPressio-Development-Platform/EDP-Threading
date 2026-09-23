# src/threading/detail/DedicatedThreadOwnedRuntime.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `50dbad50650df9fc38dd17cd3b17a60a92a5d802`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/50dbad50650df9fc38dd17cd3b17a60a92a5d802/src/threading/detail/DedicatedThreadOwnedRuntime.hpp)

## Direct includes

- `cstddef`
- `optional`
- `type_traits`
- `utility`
- `../ThreadingComposition.hpp`
- `DedicatedThreadRuntime.hpp`

## Documented declarations

### `TDeclaration`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DedicatedThreadOwnedRuntime`.
- **Template parameter `TDeclaration`:** Static topology declaration Type being realized.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed contexts.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TDeclaration, class TCallable, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedThreadOwnedRuntime;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DedicatedThreadOwnedRuntime`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed contexts.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.
- **Template parameter `TProperties`:** Compile-time execution-property Types declared for the Dedicated Thread.

```cpp
template<class TThreadIdentity, class... TProperties, class TCallable, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedThreadOwnedRuntime<
        DedicatedThread<TThreadIdentity, TProperties...>,
```

### `Declaration`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Static Dedicated Thread declaration Type realized by this owned resource.

```cpp
using Declaration = DedicatedThread<
                TThreadIdentity,
```

### `Runtime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Concrete Dedicated Thread runtime Type realized from the static declaration and bound callable.

```cpp
using Runtime = DedicatedThreadRuntime<
                TThreadIdentity,
```

### `_runtime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Topology-owned concrete Dedicated Thread runtime.

```cpp
Runtime _runtime;
```

### `ThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Semantic identity Type of this Dedicated Thread.

```cpp
using ThreadIdentity = TThreadIdentity;
```

### `Callable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Application callable Type bound to this Dedicated Thread.

```cpp
using Callable = TCallable;
```

### `RuntimeType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Public alias exposing the concrete owned runtime Type.

```cpp
using RuntimeType = Runtime;
```

### `DedicatedThreadOwnedRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs the owned Dedicated Thread runtime from its bound callable and topology services.

```cpp
DedicatedThreadOwnedRuntime(
                TCallable callable,
                TManagedContextRouter& router,
                const void* lifecycleContext,
                bool (*canActivate)(const void*) noexcept,
                bool (*shouldTerminate)(const void*) noexcept
            ) noexcept(
                std::is_nothrow_move_constructible_v<TCallable>
            ) :
                _runtime(
                    std::move(
                        callable
                    ),
                    router,
                    static_cast<
                        typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type
                    >(
                        TContextIndex
                    ),
                    lifecycleContext,
                    canActivate,
                    shouldTerminate
                ) {}
```

### `Initialize`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Initializes the persistent Platform execution context without starting it.

```cpp
WorkerExecutionInitializationResult Initialize() noexcept
```

### `StartInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts the persistent Platform execution context after global initialization commits.

```cpp
ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept
```

### `RequestInfrastructureTermination`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Wakes the persistent context so rollback or shutdown termination can be observed.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult RequestInfrastructureTermination() noexcept
```

### `JoinInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Joins the persistent Platform execution context using the supplied Platform wait budget.

```cpp
ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

### `DestroyInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys the initialized Platform execution context and its provider-owned native state.

```cpp
ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept
```

### `Handle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates a non-owning control handle for this topology-owned Dedicated Thread.

```cpp
Thread<TThreadIdentity> Handle() noexcept
```

### `StartActivation`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts one semantic Dedicated Thread activation when lifecycle state permits it.

```cpp
ThreadStartResult StartActivation() noexcept
```

### `RequestStop`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Requests cooperative stop of the currently active Dedicated Thread activation.

```cpp
ThreadStopRequestResult RequestStop() noexcept
```

### `CurrentContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns this resource's dense context index when it owns the current Platform context.

```cpp
std::optional<typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type> CurrentContextIndex() const noexcept
```

### `IsContextInterrupted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the addressed managed context currently carries a stop or shutdown interruption.

```cpp
bool IsContextInterrupted(
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type contextIndex
            ) noexcept
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this Dedicated Thread has no active semantic execution remaining.

```cpp
bool IsExecutionQuiescent() noexcept
```

### `RuntimeState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Exposes the owned concrete runtime to internal topology coordination.

```cpp
Runtime& RuntimeState() noexcept
```

