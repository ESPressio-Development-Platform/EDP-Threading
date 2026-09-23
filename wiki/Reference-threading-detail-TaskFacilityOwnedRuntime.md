# src/threading/detail/TaskFacilityOwnedRuntime.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/TaskFacilityOwnedRuntime.hpp)

## Direct includes

- `cstddef`
- `optional`
- `tuple`
- `type_traits`
- `utility`
- `../ThreadingComposition.hpp`
- `StaticTopologyPlan.hpp`
- `TaskFacilityRuntime.hpp`
- `TaskWorkerExecutionContext.hpp`

## Documented declarations

### `TFacility`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskFacilityOwnedRuntime`.
- **Template parameter `TFacility`:** Task facility declaration Type being realized.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TFirstContextIndex`:** First dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TFacility, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TFirstContextIndex, std::size_t TExecutionContextCapacity>
    class TaskFacilityOwnedRuntime;
```

### `TPoolIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskFacilityOwnedRuntime`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type or bounded capacity.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type or byte capacity.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type or byte capacity.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TFirstContextIndex`:** First dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.
- **Template parameter `TWorkers`:** Worker declaration Types owned by this Task facility.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class... TWorkers, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TFirstContextIndex, std::size_t TExecutionContextCapacity>
    class TaskFacilityOwnedRuntime<
        TaskExecutionFacility<
            TPoolIdentity,
```

### `Facility`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Concrete Task facility Type represented by this owned resource.

```cpp
using Facility = TaskFacilityRuntime<
                TRecordCapacity::Value,
```

### `template<std::size_t TWorkerIndex>`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `WorkerDeclaration`.
- **Template parameter `TWorkerIndex`:** Compile-time index of a Worker within its facility.

```cpp
template<std::size_t TWorkerIndex>
```

### `WorkerDeclaration`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Static Worker declaration Type selected at a compile-time index.

```cpp
using WorkerDeclaration = std::tuple_element_t<
                TWorkerIndex,
```

### `template<std::size_t TWorkerIndex>`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `WorkerContext`.
- **Template parameter `TWorkerIndex`:** Compile-time index of a Worker within its facility.

```cpp
template<std::size_t TWorkerIndex>
```

### `WorkerContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Persistent execution-context Type used to realize one Worker declaration.

```cpp
using WorkerContext = TaskWorkerExecutionContext<
                TExecutionContextProvider,
```

### `WorkerTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Tuple Type containing persistent Worker execution contexts selected by compile-time index.
- **Template parameter `TIndices`:** Worker indices expanded into the tuple Type.

```cpp
template<std::size_t... TIndices>
            using WorkerTuple = std::tuple<
                WorkerContext<TIndices>...
            >;
```

### `WorkerArguments`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `WorkerArguments`.
- **Template parameter `TWorkerIndex`:** Compile-time index of a Worker within its facility.

```cpp
template<std::size_t TWorkerIndex>
            static typename WorkerContext<TWorkerIndex>::ConstructionArguments WorkerArguments(
                Facility& facility,
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept
```

### `MakeWorkers`

**Classification:** PRIVATE IMPLEMENTATION

Direct-constructs the heterogeneous Worker tuple in final storage order.
- **Template parameter `TIndices`:** Worker indices expanded into direct Worker construction.

```cpp
template<std::size_t... TIndices>
            static WorkerTuple<TIndices...> MakeWorkers(
                Facility& facility,
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept,
                std::index_sequence<TIndices...>
            ) noexcept
```

### `WorkersTuple`

**Classification:** PRIVATE IMPLEMENTATION

Concrete tuple Type returned by Worker construction.

```cpp
using WorkersTuple = decltype(
                MakeWorkers(
                    std::declval<Facility&>(),
                    std::declval<TManagedContextRouter&>(),
                    nullptr,
                    nullptr,
                    std::make_index_sequence<sizeof...(TWorkers)>{}
                )
            );
```

### `_facility`

**Classification:** PRIVATE IMPLEMENTATION

Deterministic Task facility state owned directly by this topology resource.

```cpp
Facility _facility;
```

### `_workers`

**Classification:** PRIVATE IMPLEMENTATION

Persistent Worker execution contexts owned directly by this topology resource.

```cpp
WorkersTuple _workers;
```

### `DestroyInitializedPrefix`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DestroyInitializedPrefix`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInitializedPrefix() noexcept
```

### `InitializeNext`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `InitializeNext`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            WorkerExecutionInitializationResult InitializeNext() noexcept
```

### `Declaration`

**Classification:** PRIVATE IMPLEMENTATION

Static Worker/resource declaration Type represented by this specialization.

```cpp
using Declaration = WorkerDeclaration<TIndex>;
```

### `RequestTerminationPrefix`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `RequestTerminationPrefix`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Synchronization::SignalNotifyResult RequestTerminationPrefix(
                std::size_t startedCount
            ) noexcept
```

### `JoinPrefix`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `JoinPrefix`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionJoinResult JoinPrefix(
                std::size_t startedCount
            ) noexcept
```

### `StartNext`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `StartNext`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionStartResult StartNext(
                std::size_t& startedCount
            ) noexcept
```

### `RequestTerminationNext`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `RequestTerminationNext`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Synchronization::SignalNotifyResult RequestTerminationNext() noexcept
```

### `JoinNext`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `JoinNext`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionJoinResult JoinNext(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

### `DestroyNext`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DestroyNext`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyNext() noexcept
```

### `CurrentContextIndexNext`

**Classification:** PRIVATE IMPLEMENTATION

Returns the current Worker's dense context index when one Worker in this facility owns the current Platform context.
- **Template parameter `TIndex`:** Compile-time Worker index currently inspected by the recursive lookup.

```cpp
template<std::size_t TIndex>
            std::optional<typename Facility::ManagedContextIndex> CurrentContextIndexNext() const noexcept
```

### `PoolIdentity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Semantic identity Type of this Task pool.

```cpp
using PoolIdentity = TPoolIdentity;
```

### `FacilityRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Concrete Task-facility runtime Type owned by this resource.

```cpp
using FacilityRuntime = Facility;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `TaskForCallable`.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.

```cpp
template<class TCallable>
```

### `TaskForCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Task handle Type produced for the supplied callable.

```cpp
using TaskForCallable = typename Facility::template TaskForCallable<TCallable>;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `DispatchResultFor`.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.

```cpp
template<class TCallable>
```

### `DispatchResultFor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Typed dispatch result produced for the supplied callable.

```cpp
using DispatchResultFor = typename Facility::template DispatchResultFor<TCallable>;
```

### `WorkerCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of Workers owned by this facility.

```cpp
static constexpr std::size_t WorkerCount = sizeof...(TWorkers);
```

### `TaskFacilityOwnedRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs the facility and every persistent Worker against the stable topology router.

```cpp
TaskFacilityOwnedRuntime(
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept :
                _facility(router),
```

### `Initialize`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Validates facility synchronization and initializes every persistent Worker context.

```cpp
WorkerExecutionInitializationResult Initialize() noexcept
```

### `StartInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts every initialized Worker transactionally in declaration order.

```cpp
ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept
```

### `RequestInfrastructureTermination`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Wakes every Worker so rollback or terminal infrastructure termination can be observed.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult RequestInfrastructureTermination() noexcept
```

### `JoinInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Joins every persistent Worker using the supplied Platform wait budget.

```cpp
ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

### `DestroyInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys every initialized Worker execution context after successful join.

```cpp
ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `Dispatch`.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.

```cpp
template<class TCallable>
            auto Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            )
```

### `FacilityState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns mutable access to the owned Task facility runtime for topology coordination.

```cpp
Facility& FacilityState() noexcept
```

### `FacilityState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns read-only access to the owned Task facility runtime.

```cpp
const Facility& FacilityState() const noexcept
```

### `CurrentContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the dense Worker context index owned by this facility when one is currently executing.

```cpp
std::optional<typename Facility::ManagedContextIndex> CurrentContextIndex() const noexcept
```

### `IsContextInterrupted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the supplied dense Worker context currently observes Task cancellation.

```cpp
bool IsContextInterrupted(
                typename Facility::ManagedContextIndex contextIndex
            ) noexcept
```

### `BeginShutdownCancellation`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Applies terminal-shutdown cancellation semantics to queued and running Tasks in this facility.

```cpp
TaskFacilityShutdownCancellationResult BeginShutdownCancellation() noexcept
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this facility has no queued or actively executing Task work.

```cpp
bool IsExecutionQuiescent() noexcept
```

