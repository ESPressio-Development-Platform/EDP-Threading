# src/threading/detail/DedicatedThreadRuntime.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `50dbad50650df9fc38dd17cd3b17a60a92a5d802`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/50dbad50650df9fc38dd17cd3b17a60a92a5d802/src/threading/detail/DedicatedThreadRuntime.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `optional`
- `type_traits`
- `utility`
- `../Thread.hpp`
- `DedicatedThreadControl.hpp`
- `TaskFacilityRuntime.hpp`
- `TaskWorkerExecutionContext.hpp`
- `WaitRegistration.hpp`

## Documented declarations

### `DedicatedThreadSynchronizationResult`

**Classification:** PRIVATE IMPLEMENTATION

Outcome of validating Dedicated Thread synchronization resources.

```cpp
enum class DedicatedThreadSynchronizationResult : std::uint8_t
```

### `DedicatedThreadStoppedPublicationResult`

**Classification:** PRIVATE IMPLEMENTATION

Outcome of publishing terminal state for one Dedicated Thread activation phase.

```cpp
enum class DedicatedThreadStoppedPublicationResult : std::uint8_t
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Detects whether a Dedicated Thread callable returns void when invoked without ThreadContext.
- **Template parameter `TCallable`:** Callable Type being inspected.
- **Template parameter `TEnable`:** SFINAE helper Type used to select the invocable specialization.

```cpp
template<class TCallable, class TEnable = void>
    struct IsVoidThreadCallableWithoutContext final
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result produced by this trait specialization.

```cpp
static constexpr bool Value = false;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsVoidThreadCallableWithoutContext`.
- **Template parameter `TCallable`:** Callable Type being invoked, stored, or adapted.

```cpp
template<class TCallable>
    struct IsVoidThreadCallableWithoutContext<
        TCallable,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result produced by this trait specialization.

```cpp
static constexpr bool Value = std::is_same_v<
            std::invoke_result_t<TCallable&>,
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Detects whether a Dedicated Thread callable returns void when invoked with ThreadContext.
- **Template parameter `TCallable`:** Callable Type being inspected.
- **Template parameter `TEnable`:** SFINAE helper Type used to select the invocable specialization.

```cpp
template<class TCallable, class TEnable = void>
    struct IsVoidThreadCallableWithContext final
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result produced by this trait specialization.

```cpp
static constexpr bool Value = false;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsVoidThreadCallableWithContext`.
- **Template parameter `TCallable`:** Callable Type being invoked, stored, or adapted.

```cpp
template<class TCallable>
    struct IsVoidThreadCallableWithContext<
        TCallable,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result produced by this trait specialization.

```cpp
static constexpr bool Value = std::is_same_v<
            std::invoke_result_t<
                TCallable&,
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DedicatedThreadRuntime`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TCallable`:** Callable Type being invoked, stored, or adapted.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TStackCapacity`:** Semantic stack-capacity value requested by the topology.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TThreadIdentity, class TCallable, std::size_t TStackCapacity, std::size_t TExecutionContextCapacity, class TMutexProvider, class TExecutionContextProvider, class TManagedContextRouter>
    class DedicatedThreadRuntime final
```

### `_control`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

One-byte authoritative Dedicated Thread lifecycle and activation Phase.

```cpp
DedicatedThreadControl _control;
```

### `RegistrationSet< ThreadJoinRegistration< typename ExecutionContextInde`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Target-owned Join registrations, bounded by managed execution-context count.

```cpp
RegistrationSet<
                ThreadJoinRegistration<
                    typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type
                >,
```

### `_mutex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Dedicated Thread lifecycle serialization.

```cpp
TMutexProvider _mutex;
```

### `_callable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Concrete callable retained for the full topology lifetime.

```cpp
TCallable _callable;
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

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Compact Type used to identify one managed execution context.

```cpp
using ContextIndex =
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type;
```

### `LockAcquireResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Typed Platform outcome returned when the runtime acquires its serialization mutex.

```cpp
using LockAcquireResult =
                ESPressio::Platform::Synchronization::LockAcquireResult;
```

### `LockReleaseResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Typed Platform outcome returned when the runtime releases its serialization mutex.

```cpp
using LockReleaseResult =
                ESPressio::Platform::Synchronization::LockReleaseResult;
```

### `_router`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning topology wake/interruption router.

```cpp
TManagedContextRouter* _router;
```

### `_contextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Dense topology-wide index of this Dedicated Thread context.

```cpp
ContextIndex _contextIndex;
```

### `_lifecycleContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning global lifecycle context.

```cpp
const void* _lifecycleContext;
```

### `bool`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Predicate permitting semantic activation only after successful infrastructure Start.

```cpp
bool (*_canActivate)(const void*) noexcept;
```

### `bool`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Predicate requesting terminal trampoline exit during rollback/shutdown.

```cpp
bool (*_shouldTerminate)(const void*) noexcept;
```

### `AcquireLock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Acquires the Dedicated Thread serialization mutex indefinitely.

```cpp
LockAcquireResult AcquireLock() noexcept
```

### `ReleaseLock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Releases the Dedicated Thread serialization mutex.

```cpp
LockReleaseResult ReleaseLock() noexcept
```

### `PublicStateFor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Projects the compact internal lifecycle state onto the stable public Dedicated Thread state.

```cpp
static ThreadState PublicStateFor(
                DedicatedThreadOperationalState state
            ) noexcept
```

### `WakeJoiners`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Wakes every managed context registered for the completed activation Phase.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult WakeJoiners(
                bool phase
            )
```

### `IsActivationStopped`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether the captured activation Phase has reached a stopped state.

```cpp
bool IsActivationStopped(
                bool capturedPhase
            ) const noexcept
```

### `IsStopRequestedThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bridges ThreadContext stop observation back to this runtime without exposing its concrete Type.

```cpp
static bool IsStopRequestedThunk(
                const void* resource
            ) noexcept
```

### `InvokeCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Invokes the bound application callable using the supported Dedicated Thread signature.

```cpp
void InvokeCallable()
```

### `PublishStopped`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Publishes terminal state for the completed activation and wakes matching joiners.

```cpp
DedicatedThreadStoppedPublicationResult PublishStopped(
                bool activationPhase
            ) noexcept
```

### `ValidateSynchronization`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Validates that the configured Mutex provider can support Dedicated Thread lifecycle serialization.

```cpp
DedicatedThreadSynchronizationResult ValidateSynchronization() noexcept
```

### `Entry`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Runs the persistent Platform trampoline that waits for semantic activations or infrastructure termination.

```cpp
static void Entry(
                void* parameter
            ) noexcept
```

### `JoinWithBudget`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Joins the activation captured at entry using one non-restarting canonical wait budget.

```cpp
ThreadJoinResult JoinWithBudget(
                const MonotonicWaitBudget& budget
            )
```

### `StateThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Type-erased Thread handle bridge for lifecycle-state observation.

```cpp
static ThreadState StateThunk(
                const void* resource
            ) noexcept
```

### `StartThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Type-erased Thread handle bridge for semantic activation.

```cpp
static ThreadStartResult StartThunk(
                void* resource
            ) noexcept
```

### `RequestStopThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Type-erased Thread handle bridge for cooperative stop requests.

```cpp
static ThreadStopRequestResult RequestStopThunk(
                void* resource
            ) noexcept
```

### `JoinThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Type-erased Thread handle bridge for indefinite Join.

```cpp
static ThreadJoinResult JoinThunk(
                void* resource
            )
```

### `JoinForThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Type-erased Thread handle bridge for relative-duration Join.

```cpp
static ThreadJoinResult JoinForThunk(
                void* resource,
                Duration duration
            )
```

### `JoinUntilThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Type-erased Thread handle bridge for monotonic-deadline Join.

```cpp
static ThreadJoinResult JoinUntilThunk(
                void* resource,
                MonotonicTimestamp deadline
            )
```

### `ProviderTraits`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Validated Platform execution-context provider contract traits.

```cpp
using ProviderTraits =
                ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<
                    TExecutionContextProvider
                >;
```

### `DedicatedThreadRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs one topology-owned Dedicated Thread runtime without starting its Platform context.

```cpp
DedicatedThreadRuntime(
                TCallable callable,
                TManagedContextRouter& router,
                ContextIndex contextIndex,
                const void* lifecycleContext,
                bool (*canActivate)(const void*) noexcept,
                bool (*shouldTerminate)(const void*) noexcept
            ) noexcept(
                std::is_nothrow_move_constructible_v<TCallable>
            ) :
                _callable(
                    std::move(
                        callable
                    )
                ),
```

### `Initialize`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Initializes synchronization and the persistent Platform execution context without starting execution.

```cpp
WorkerExecutionInitializationResult Initialize(
                ESPressio::Platform::Execution::ExecutionPriority priority,
                ESPressio::Platform::Execution::ProcessorAffinity affinity,
                const char* name = nullptr
            ) noexcept
```

### `StartInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts the persistent Platform execution context after the topology initialization barrier.

```cpp
ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept
```

### `RequestInfrastructureTermination`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Wakes the persistent trampoline so rollback/shutdown termination is re-evaluated.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult RequestInfrastructureTermination() noexcept
```

### `JoinInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Joins the persistent Platform execution context using the supplied native wait budget.

```cpp
ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

### `DestroyInfrastructure`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys the initialized Platform execution context and releases its native provider state.

```cpp
ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept
```

### `State`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the stable public lifecycle state under the Dedicated Thread serialization boundary.

```cpp
ThreadState State() noexcept
```

### `StartActivation`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts one semantic activation when global lifecycle and local state permit it.

```cpp
ThreadStartResult StartActivation() noexcept
```

### `RequestStop`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Requests cooperative stop of the current activation and wakes its persistent context.

```cpp
ThreadStopRequestResult RequestStop() noexcept
```

### `IsStopRequested`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether cooperative stop has been requested for the current activation.

```cpp
bool IsStopRequested() noexcept
```

### `Join`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Joins the activation captured when this call begins.

```cpp
ThreadJoinResult Join()
```

### `JoinFor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Joins the captured activation within one relative physical-time budget.

```cpp
ThreadJoinResult JoinFor(
                Duration duration
            )
```

### `JoinUntil`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Joins the captured activation until one canonical monotonic deadline.

```cpp
ThreadJoinResult JoinUntil(
                MonotonicTimestamp deadline
            )
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether no semantic Dedicated Thread activation remains active.

```cpp
bool IsExecutionQuiescent() noexcept
```

### `CurrentContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns this Dedicated Thread's dense context index when it owns the current Platform context.

```cpp
std::optional<ContextIndex> CurrentContextIndex() const noexcept
```

### `IsContextInterrupted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the addressed Dedicated Thread context currently has a cooperative interruption request.

```cpp
bool IsContextInterrupted(
                ContextIndex contextIndex
            ) noexcept
```

### `Handle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates a non-owning public control handle for this topology-owned Dedicated Thread.

```cpp
Thread<TThreadIdentity> Handle() noexcept
```

### `HandleOperations`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the static type-erased operation table shared by handles for this concrete runtime.

```cpp
static const ThreadHandleOperations& HandleOperations() noexcept
```

### `ProviderObjectBytes`

**Classification:** PRIVATE IMPLEMENTATION

Deterministic byte count reported for `ProviderObjectBytes`.

```cpp
static constexpr std::size_t ProviderObjectBytes() noexcept
```

### `ControlBackingBytes`

**Classification:** PRIVATE IMPLEMENTATION

Deterministic byte count reported for `ControlBackingBytes`.

```cpp
static constexpr std::size_t ControlBackingBytes() noexcept
```

### `StackBackingBytes`

**Classification:** PRIVATE IMPLEMENTATION

Deterministic byte count reported for `StackBackingBytes`.

```cpp
static constexpr std::size_t StackBackingBytes() noexcept
```

