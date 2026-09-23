# src/threading/detail/DedicatedWorkerLeaseRuntime.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `50dbad50650df9fc38dd17cd3b17a60a92a5d802`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/50dbad50650df9fc38dd17cd3b17a60a92a5d802/src/threading/detail/DedicatedWorkerLeaseRuntime.hpp)

## Direct includes

- `cstddef`
- `optional`
- `type_traits`
- `utility`
- `TaskFacilityRuntime.hpp`
- `TaskWorkerExecutionContext.hpp`

## Documented declarations

### `TTaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DedicatedWorkerLeaseRuntime`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TRecordCapacity`:** Bounded Task-record capacity.
- **Template parameter `TCallableCapacity`:** Per-record callable-storage capacity in bytes.
- **Template parameter `TResultCapacity`:** Per-record result-storage capacity in bytes.
- **Template parameter `TStackCapacity`:** Semantic stack-capacity value requested by the topology.
- **Template parameter `TExecutionContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TTaskIdentity, std::size_t TRecordCapacity, std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TStackCapacity, std::size_t TExecutionContextIndex, std::size_t TExecutionContextCapacity, class TMutexProvider, class TExecutionContextProvider, class TManagedContextRouter>
    class DedicatedWorkerLeaseRuntime final
```

### `Facility`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Concrete Task-facility runtime Type owned by this resource.

```cpp
using Facility = TaskFacilityRuntime<
                TRecordCapacity,
```

### `WorkerContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Persistent Worker execution-context Type owned by this resource.

```cpp
using WorkerContext = TaskWorkerExecutionContext<
                TExecutionContextProvider,
```

### `_facility`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Isolated Task-record/FIFO/Worker-Lease facility for this critical Task Type only.

```cpp
Facility _facility;
```

### `_worker`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Exactly one isolated persistent Worker context.

```cpp
WorkerContext _worker;
```

### `TaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Semantic identity Type of this Dedicated Worker task.

```cpp
using TaskIdentity = TTaskIdentity;
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
using TaskForCallable = typename Facility::template TaskForCallable<TCallable>;
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
using DispatchResultFor = typename Facility::template DispatchResultFor<TCallable>;
```

### `WorkerCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of Workers owned by this resource.

```cpp
static constexpr std::size_t WorkerCount = 1U;
```

### `DedicatedWorkerLeaseRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs the isolated Worker lease runtime against topology routing and shutdown state.

```cpp
DedicatedWorkerLeaseRuntime(
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept :
                _facility(router),
```

### `Initialize`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Validates facility synchronization and initializes the isolated persistent Worker context.

```cpp
WorkerExecutionInitializationResult Initialize(
                ESPressio::Platform::Execution::ExecutionPriority priority,
                ESPressio::Platform::Execution::ProcessorAffinity affinity,
                const char* name = nullptr
            ) noexcept
```

### `StartInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts the isolated persistent Worker after topology initialization commits.

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

Destroys the isolated Worker execution context and releases its native provider state.

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

Dispatches one callable through this isolated lease's bounded Task facility.

```cpp
auto Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            )
```

### `BeginShutdownCancellation`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Cancels queued work and requests cooperative cancellation of any running lease Task.

```cpp
TaskFacilityShutdownCancellationResult BeginShutdownCancellation() noexcept
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this isolated Worker lease has no remaining executable Task work.

```cpp
bool IsExecutionQuiescent() noexcept
```

### `CurrentContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns this isolated Worker's dense context index when it owns the current Platform context.

```cpp
std::optional<typename Facility::ManagedContextIndex> CurrentContextIndex() const noexcept
```

### `IsContextInterrupted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the isolated Worker context currently carries cancellation or shutdown interruption.

```cpp
bool IsContextInterrupted(
                typename Facility::ManagedContextIndex contextIndex
            ) noexcept
```

### `RecordCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time capacity reported by `RecordCapacity`.

```cpp
static constexpr std::size_t RecordCapacity() noexcept
```

### `RecordsInUse`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the number of structurally allocated Task records currently in use.

```cpp
std::size_t RecordsInUse() noexcept
```

### `QueuedTasks`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the number of Tasks currently queued in this isolated lease.

```cpp
std::size_t QueuedTasks() noexcept
```

### `WorkersInUse`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns whether the single isolated Worker is currently granted to Task execution.

```cpp
std::size_t WorkersInUse() noexcept
```

### `ProviderObjectBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Deterministic byte count reported for `ProviderObjectBytes`.

```cpp
static constexpr std::size_t ProviderObjectBytes() noexcept
```

### `ControlBackingBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Deterministic byte count reported for `ControlBackingBytes`.

```cpp
static constexpr std::size_t ControlBackingBytes() noexcept
```

### `StackBackingBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Deterministic byte count reported for `StackBackingBytes`.

```cpp
static constexpr std::size_t StackBackingBytes() noexcept
```

