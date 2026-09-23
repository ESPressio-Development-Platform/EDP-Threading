# src/threading/detail/DedicatedWorkerOwnedRuntime.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `50dbad50650df9fc38dd17cd3b17a60a92a5d802`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/50dbad50650df9fc38dd17cd3b17a60a92a5d802/src/threading/detail/DedicatedWorkerOwnedRuntime.hpp)

## Direct includes

- `cstddef`
- `optional`
- `utility`
- `../ThreadingComposition.hpp`
- `DedicatedWorkerLeaseRuntime.hpp`

## Documented declarations

### `TDeclaration`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DedicatedWorkerOwnedRuntime`.
- **Template parameter `TDeclaration`:** Static resource declaration Type being realized.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TDeclaration, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedWorkerOwnedRuntime;
```

### `TTaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DedicatedWorkerOwnedRuntime`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.
- **Template parameter `TProperties`:** Compile-time capacities and execution-property Types declared for the Dedicated Worker lease.

```cpp
template<class TTaskIdentity, class... TProperties, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedWorkerOwnedRuntime<
        DedicatedWorkerLease<TTaskIdentity, TProperties...>,
```

### `Declaration`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Static topology declaration Type realized by this owned resource.

```cpp
using Declaration = DedicatedWorkerLease<
                TTaskIdentity,
```

### `Runtime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Concrete runtime Type realized from the static declaration.

```cpp
using Runtime = DedicatedWorkerLeaseRuntime<
                TTaskIdentity,
```

### `_runtime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Topology-owned concrete Dedicated Worker runtime.

```cpp
Runtime _runtime;
```

### `TaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Semantic identity Type of this Dedicated Worker task.

```cpp
using TaskIdentity = TTaskIdentity;
```

### `RuntimeType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Public alias exposing the concrete owned runtime Type.

```cpp
using RuntimeType = Runtime;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `TaskForCallable`.
- **Template parameter `TCallable`:** Callable Type being invoked, stored, or adapted.

```cpp
template<class TCallable>
```

### `TaskForCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Concrete Task handle Type produced for the supplied callable.

```cpp
using TaskForCallable = typename Runtime::template TaskForCallable<TCallable>;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `DispatchResultFor`.
- **Template parameter `TCallable`:** Callable Type being invoked, stored, or adapted.

```cpp
template<class TCallable>
```

### `DispatchResultFor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Typed dispatch-result Type produced for the supplied callable.

```cpp
using DispatchResultFor = typename Runtime::template DispatchResultFor<TCallable>;
```

### `DedicatedWorkerOwnedRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs the topology-owned Dedicated Worker runtime against routing and shutdown services.

```cpp
DedicatedWorkerOwnedRuntime(
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept :
                _runtime(
                    router,
                    shutdownContext,
                    isShutdownRequested
                ) {}
```

### `Initialize`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Initializes synchronization and the isolated persistent Worker context without starting execution.

```cpp
WorkerExecutionInitializationResult Initialize() noexcept
```

### `StartInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts the isolated persistent Worker after the topology initialization barrier.

```cpp
ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept
```

### `RequestInfrastructureTermination`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Wakes the isolated Worker so rollback or shutdown termination can be observed.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult RequestInfrastructureTermination() noexcept
```

### `JoinInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Joins the isolated Worker context using the supplied Platform wait budget.

```cpp
ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

### `DestroyInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys the isolated Worker context and releases its native provider state.

```cpp
ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `Dispatch`.
- **Template parameter `TCallable`:** Callable Type being invoked, stored, or adapted.

```cpp
template<class TCallable>
```

### `Dispatch`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Dispatches one callable through the isolated Dedicated Worker facility.

```cpp
auto Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            )
```

### `CurrentContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns this resource's dense context index when it owns the current Platform context.

```cpp
std::optional<typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type> CurrentContextIndex() const noexcept
```

### `IsContextInterrupted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the addressed isolated Worker context carries cancellation or shutdown interruption.

```cpp
bool IsContextInterrupted(
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type contextIndex
            ) noexcept
```

### `BeginShutdownCancellation`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Applies terminal shutdown cancellation semantics to the isolated Task facility.

```cpp
TaskFacilityShutdownCancellationResult BeginShutdownCancellation() noexcept
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this Dedicated Worker resource has no executable Task work remaining.

```cpp
bool IsExecutionQuiescent() noexcept
```

### `RuntimeState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Exposes the owned concrete runtime to internal topology coordination.

```cpp
Runtime& RuntimeState() noexcept
```

