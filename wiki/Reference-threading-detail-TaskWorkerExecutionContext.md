# src/threading/detail/TaskWorkerExecutionContext.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/TaskWorkerExecutionContext.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `ESPressio_Platform.hpp`

## Documented declarations

### `TExecutionContextProvider`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ExecutionContextBacking`.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TStackCapacity`:** Semantic stack-capacity value requested by the topology.

```cpp
template<class TExecutionContextProvider, std::size_t TStackCapacity>
    class ExecutionContextBacking final
```

### `Capabilities`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Composition capability set exposed by the execution-context provider.

```cpp
using Capabilities = typename TExecutionContextProvider::CompositionOffers;
```

### `Properties`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resolved capability-property set for the execution-context provider.

```cpp
using Properties = typename Capabilities::template PropertiesFor<
                ESPressio::Platform::Execution::ExecutionContext
            >;
```

### `ControlBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Caller-owned native execution-control backing bytes required by the provider.

```cpp
static constexpr std::size_t ControlBytes =
                Properties::template Value<
                    ESPressio::Platform::Execution::ControlStorageBytes
                >;
```

### `ControlAlignment`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Required alignment of caller-owned execution-control backing.

```cpp
static constexpr std::size_t ControlAlignment =
                Properties::template Value<
                    ESPressio::Platform::Execution::ControlStorageAlignment
                >;
```

### `StackAlignment`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Required alignment of caller-owned execution stack backing.

```cpp
static constexpr std::size_t StackAlignment =
                Properties::template Value<
                    ESPressio::Platform::Execution::StackStorageAlignment
                >;
```

### `StackGranularity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Provider allocation granularity used when reserving stack bytes.

```cpp
static constexpr std::size_t StackGranularity =
                Properties::template Value<
                    ESPressio::Platform::Execution::StackAllocationGranularityBytes
                >;
```

### `RoundedStackBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Physical stack reservation after rounding the semantic request to provider granularity.

```cpp
static constexpr std::size_t RoundedStackBytes =
                (
                    (
                        TStackCapacity +
                        StackGranularity -
                        1U
                    ) /
                    StackGranularity
                ) *
                StackGranularity;
```

### `alignas`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Native provider control backing.

```cpp
alignas(ControlAlignment) std::byte _control[ControlBytes];
```

### `alignas`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Native execution stack backing rounded to provider allocation granularity.

```cpp
alignas(StackAlignment) std::byte _stack[RoundedStackBytes];
```

### `SemanticStackCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Semantic minimum stack capacity requested by Threading topology.

```cpp
static constexpr std::size_t SemanticStackCapacity() noexcept
```

### `PhysicalStackCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Physical caller-owned stack bytes reserved for the concrete provider.

```cpp
static constexpr std::size_t PhysicalStackCapacity() noexcept
```

### `PhysicalControlCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Physical caller-owned native control bytes reserved for the concrete provider.

```cpp
static constexpr std::size_t PhysicalControlCapacity() noexcept
```

### `Storage`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the caller-owned storage supplied during ExecutionContext initialization.

```cpp
ESPressio::Platform::Execution::ExecutionStorage Storage() noexcept
```

### `TExecutionContextProvider`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskWorkerExecutionContext`.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TTaskFacility`:** Task facility Type serviced by the Worker context.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TStackCapacity`:** Semantic stack-capacity value requested by the topology.

```cpp
template<class TExecutionContextProvider, std::size_t TStackCapacity, class TTaskFacility, class TManagedContextRouter>
    class TaskWorkerExecutionContext final
```

### `ProviderTraits`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Validated Platform execution-context provider contract traits.

```cpp
using ProviderTraits =
                ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<
                    TExecutionContextProvider
                >;
```

### `_provider`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Concrete Platform execution-context provider.

```cpp
TExecutionContextProvider _provider;
```

### `TExecutionContextProvider`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Caller-owned native control and stack backing.

```cpp
ExecutionContextBacking<
                TExecutionContextProvider,
```

### `_facility`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning Task facility served by this Worker.

```cpp
TTaskFacility* _facility;
```

### `_router`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning managed-context router owning this Worker's targeted wake primitive.

```cpp
TManagedContextRouter* _router;
```

### `_contextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Dense topology-wide execution-context index assigned to this Worker.

```cpp
typename TTaskFacility::ManagedContextIndex _contextIndex;
```

### `_shutdownContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning authoritative shutdown-state context.

```cpp
const void* _shutdownContext;
```

### `bool`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Predicate reading authoritative terminal-shutdown intent.

```cpp
bool (*_isShutdownRequested)(const void*) noexcept;
```

### `Entry`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Runs the persistent Worker loop after infrastructure Start.

```cpp
static void Entry(
                void* parameter
            ) noexcept
```

### `ConstructionArguments`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Lightweight constructor arguments used to direct-construct a non-movable Worker inside static tuple storage.

```cpp
struct ConstructionArguments final
```

### `Facility`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Task facility served by this Worker.

```cpp
TTaskFacility* Facility = nullptr;
```

### `Router`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Managed-context router owning this Worker's targeted wake primitive.

```cpp
TManagedContextRouter* Router = nullptr;
```

### `typename TTaskFacility::ManagedContextIndex ContextIndex{};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Dense topology-wide execution-context index assigned to this Worker.

```cpp
typename TTaskFacility::ManagedContextIndex ContextIndex{};
```

### `ShutdownContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Authoritative shutdown-state context.

```cpp
const void* ShutdownContext = nullptr;
```

### `bool`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Predicate reading authoritative terminal-shutdown intent.

```cpp
bool (*IsShutdownRequested)(const void*) noexcept = nullptr;
```

### `TaskWorkerExecutionContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Direct-constructs one statically provisioned Worker from lightweight topology bindings.

```cpp
explicit TaskWorkerExecutionContext(
                ConstructionArguments arguments
            ) noexcept :
                _facility(arguments.Facility),
```

### `TaskWorkerExecutionContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Binds one statically provisioned Worker to its facility, wake router and shutdown predicate.

```cpp
TaskWorkerExecutionContext(
                TTaskFacility& facility,
                TManagedContextRouter& router,
                typename TTaskFacility::ManagedContextIndex contextIndex,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept :
                TaskWorkerExecutionContext(
                    ConstructionArguments {
                        &facility,
                        &router,
                        contextIndex,
                        shutdownContext,
                        isShutdownRequested
                    }
                ) {}
```

### `Initialize`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Initializes the Platform execution context without starting Worker execution.

```cpp
WorkerExecutionInitializationResult Initialize(
                ESPressio::Platform::Execution::ExecutionPriority priority,
                ESPressio::Platform::Execution::ProcessorAffinity affinity,
                const char* name = nullptr
            ) noexcept
```

### `StartInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts the already initialized persistent Worker trampoline.

```cpp
ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept
```

### `Start`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compatibility spelling retained inside the implementation while Bootstrap is integrated.

```cpp
ESPressio::Platform::Execution::ExecutionStartResult Start() noexcept
```

### `RequestInfrastructureTermination`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Wakes the persistent Worker so rollback/shutdown termination is re-evaluated.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult RequestInfrastructureTermination() noexcept
```

### `Join`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Joins the Worker after authoritative shutdown has caused its trampoline to return.

```cpp
ESPressio::Platform::Execution::ExecutionJoinResult Join(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

### `Destroy`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys the joined native execution context.

```cpp
ESPressio::Platform::Execution::ExecutionDestroyResult Destroy() noexcept
```

### `IsCurrentContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this Worker's Platform context is the current execution context.

```cpp
bool IsCurrentContext() const noexcept
```

### `IsInterrupted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the current Worker execution should cooperatively return from a Threading-owned block.

```cpp
bool IsInterrupted() noexcept
```

### `ProviderObjectBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the concrete provider object size counted separately from caller backing.

```cpp
static constexpr std::size_t ProviderObjectBytes() noexcept
```

### `ControlBackingBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns caller-owned native control backing bytes.

```cpp
static constexpr std::size_t ControlBackingBytes() noexcept
```

### `StackBackingBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns caller-owned physical stack backing bytes.

```cpp
static constexpr std::size_t StackBackingBytes() noexcept
```

