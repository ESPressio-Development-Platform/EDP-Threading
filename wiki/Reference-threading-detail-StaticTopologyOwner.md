# src/threading/detail/StaticTopologyOwner.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/StaticTopologyOwner.hpp)

## Direct includes

- `array`
- `cstddef`
- `tuple`
- `type_traits`
- `utility`
- `ManagedContextRouter.hpp`
- `ManagedContextWakeSet.hpp`
- `ShutdownWaitRuntime.hpp`
- `StaticTopologyResourceStorage.hpp`
- `StructuralContextResolver.hpp`
- `ThreadingBootstrap.hpp`

## Documented declarations

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `StructuralResolverForTuple`.
- **Template parameter `TTuple`:** Tuple Type whose resources are transformed or traversed.

```cpp
template<class TTuple>
    struct StructuralResolverForTuple;
```

### `StructuralResolverForTuple`

**Classification:** PRIVATE IMPLEMENTATION

Specializes structural-resolver construction for a tuple of concrete runtime resources.
- **Template parameter `TResources`:** Concrete topology-owned runtime resource Types represented by the tuple.

```cpp
template<class... TResources>
    struct StructuralResolverForTuple<
        std::tuple<TResources...>
    >
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolves the concrete structural context-resolver Type for this resource tuple.
- **Template parameter `TContextCapacity`:** Number of managed execution contexts represented by the topology.

```cpp
template<std::size_t TContextCapacity>
        using Type = StructuralContextResolver<
            TContextCapacity,
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `StaticTopologyOwner`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TSignalProvider`:** Concrete Platform Signal provider Type used for targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TSpinLockProvider`:** Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.

```cpp
template<class TTopology, class TBindings, class TSignalProvider, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider>
    class StaticTopologyOwner final
```

### `ContextCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Number of managed execution contexts represented by this topology/runtime.

```cpp
static constexpr std::size_t ContextCapacity =
                TTopology::ManagedExecutionContextCount;
```

### `WakeSet`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Targeted-wake set Type owned by the runtime.

```cpp
using WakeSet = ManagedContextWakeSet<
                ContextCapacity,
```

### `Router`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Managed-context router Type owned by the runtime.

```cpp
using Router = ManagedContextRouter<
                ContextCapacity,
```

### `Bootstrap`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bootstrap coordinator Type owning infrastructure lifecycle state.

```cpp
using Bootstrap = ThreadingBootstrap<
                TSpinLockProvider
            >;
```

### `ShutdownWait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bounded shutdown-completion wait runtime Type.

```cpp
using ShutdownWait = ShutdownWaitRuntime<
                typename Bootstrap::Lifecycle,
```

### `Resources`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Recursive in-place storage Type owning all declared topology resources.

```cpp
using Resources = StaticTopologyResourceStorage<
                TTopology,
```

### `ResourceTypes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Tuple Type describing the concrete resource runtime Types.

```cpp
using ResourceTypes = OwnedResourceTuple<
                TTopology,
```

### `Resolver`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Structural context-resolver Type spanning all concrete resources.

```cpp
using Resolver = typename StructuralResolverForTuple<
                ResourceTypes
            >::template Type<ContextCapacity>;
```

### `_bootstrap`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Owns the authoritative Threading infrastructure lifecycle and start/shutdown coordinator.

```cpp
Bootstrap _bootstrap;
```

### `_wakeSet`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Owns one targeted wake provider for each managed execution context.

```cpp
WakeSet _wakeSet;
```

### `_router`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Routes current-context identity, interruption state, and targeted wake operations.

```cpp
Router _router;
```

### `_bindings`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Owns the application-supplied Dedicated Thread callable bindings for the runtime lifetime.

```cpp
TBindings _bindings;
```

### `_resources`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Owns every statically declared Task facility, Dedicated Worker, and Dedicated Thread runtime resource.

```cpp
Resources _resources;
```

### `_resolver`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resolves managed context identity structurally across the owned resource tree.

```cpp
Resolver _resolver;
```

### `_shutdownWait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Owns bounded wait registration for terminal Threading shutdown observation.

```cpp
ShutdownWait _shutdownWait;
```

### `MakeResolver`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Builds the structural context resolver from every topology-owned runtime resource.
- **Template parameter `TIndices`:** Compile-time resource indices expanded into the resolver constructor.

```cpp
template<std::size_t... TIndices>
            static Resolver MakeResolver(
                Resources& resources,
                std::index_sequence<TIndices...>
            ) noexcept
```

### `InitializeNext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Initializes topology resources recursively in declaration order and rolls back the initialized prefix on failure.
- **Template parameter `TIndex`:** Compile-time resource index currently being initialized.

```cpp
template<std::size_t TIndex>
            ThreadingInitializationResult InitializeNext() noexcept
```

### `DestroyInitializedPrefix`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Destroys an already-initialized prefix in reverse order during initialization rollback.
- **Template parameter `TIndex`:** Number of initialized resources remaining in the prefix.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInitializedPrefix() noexcept
```

### `InitializeResourceAt`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Locates and initializes one runtime resource selected by a runtime index.
- **Template parameter `TIndex`:** Compile-time resource index currently being inspected.

```cpp
template<std::size_t TIndex>
            WorkerExecutionInitializationResult InitializeResourceAt(
                std::size_t targetIndex
            ) noexcept
```

### `DestroyResourceAt`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Locates and destroys one runtime resource selected by a runtime index during ordered rollback.
- **Template parameter `TIndex`:** Compile-time resource index currently being inspected.

```cpp
template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyResourceAt(
                std::size_t targetIndex
            ) noexcept
```

### `InitializeInRuntimeOrder`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Initializes resources according to a validated application-specified permutation and rolls back that same order on failure.
- **Template parameter `TOrderIndex`:** Compile-time position currently being initialized within the supplied order.
- **Template parameter `TOrderCount`:** Total number of topology resources represented by the supplied order.

```cpp
template<std::size_t TOrderIndex, std::size_t TOrderCount>
            ThreadingInitializationResult InitializeInRuntimeOrder(
                const std::array<std::size_t, TOrderCount>& order
            ) noexcept
```

### `TPoolIdentity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resolves the topology-owned ordinary Task facility identified by a Pool identity.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the requested Task pool.

```cpp
template<class TPoolIdentity>
            auto& TaskFacility() noexcept
```

### `TTaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resolves the topology-owned Dedicated Worker identified by its Task identity.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the requested Dedicated Worker task.

```cpp
template<class TTaskIdentity>
            auto& DedicatedWorker() noexcept
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resolves the topology-owned Dedicated Thread runtime identified by Thread identity.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the requested Dedicated Thread.

```cpp
template<class TThreadIdentity>
            auto& DedicatedThreadResource() noexcept
```

### `StartAll`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Starts all topology resources in the supplied compile-time order through transactional Bootstrap coordination.
- **Template parameter `TIndices`:** Compile-time topology resource indices expanded in start order.

```cpp
template<std::size_t... TIndices>
            ThreadingStartResult StartAll(
                std::index_sequence<TIndices...>
            ) noexcept
```

### `StaticTopologyOwner`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Prevents copying because this owner contains address-stable Platform resources and internal cross-references.

```cpp
StaticTopologyOwner(
                const StaticTopologyOwner&
            ) = delete;
```

### `operator`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Prevents copy assignment for the address-stable topology owner.

```cpp
StaticTopologyOwner& operator =(
                const StaticTopologyOwner&
            ) = delete;
```

### `StaticTopologyOwner`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Prevents moving because internal resource/router addresses must remain stable after construction.

```cpp
StaticTopologyOwner(
                StaticTopologyOwner&&
            ) = delete;
```

### `operator`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Prevents move assignment for the address-stable topology owner.

```cpp
StaticTopologyOwner& operator =(
                StaticTopologyOwner&&
            ) = delete;
```

### `StaticTopologyOwner`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs the complete static topology, retaining Dedicated Thread bindings without starting execution.

```cpp
explicit StaticTopologyOwner(
                TBindings bindings
            ) :
                _bootstrap(),
```

### `Initialize`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Initializes every Platform-backed topology resource in declaration order without starting managed execution.

```cpp
ThreadingInitializationResult Initialize() noexcept
```

### `InitializeInOrder`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Initializes every topology resource using an application-specified compile-time permutation.
- **Template parameter `TResourceIndices`:** Topology resource indices naming every declared resource exactly once.

```cpp
template<std::size_t... TResourceIndices>
            ThreadingInitializationResult InitializeInOrder() noexcept
```

### `Start`

**Classification:** PRIVATE IMPLEMENTATION

Starts all initialized infrastructure contexts transactionally in topology declaration order.

```cpp
ThreadingStartResult Start() noexcept
```

### `StartInOrder`

**Classification:** PRIVATE IMPLEMENTATION

Starts all initialized topology resources transactionally using an application-specified compile-time permutation.
- **Template parameter `TResourceIndices`:** Topology resource indices naming every declared resource exactly once in start order.

```cpp
template<std::size_t... TResourceIndices>
            ThreadingStartResult StartInOrder() noexcept
```

### `TPoolIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Dispatches finite work to the ordinary Task facility identified by Pool identity.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the destination Task pool.
- **Template parameter `TCallable`:** Callable Type admitted into the bounded facility.

```cpp
template<class TPoolIdentity, class TCallable>
            auto Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            )
```

### `TTaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Dispatches finite work to the isolated Dedicated Worker identified by Task identity.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the destination Dedicated Worker task.
- **Template parameter `TCallable`:** Callable Type admitted into the isolated bounded facility.

```cpp
template<class TTaskIdentity, class TCallable>
            auto DispatchDedicated(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            )
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Returns a move-only non-owning control handle for the requested topology-owned Dedicated Thread.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the requested Dedicated Thread.

```cpp
template<class TThreadIdentity>
            Thread<TThreadIdentity> ThreadHandle() noexcept
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Starts one topology-owned Dedicated Thread through the lifecycle-gated Bootstrap surface.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread to activate.

```cpp
template<class TThreadIdentity>
            ThreadStartResult StartThread() noexcept
```

### `BeginShutdown`

**Classification:** PRIVATE IMPLEMENTATION

Begins terminal shutdown, publishing the global lifecycle transition before cancelling/stopping resource work.

```cpp
ThreadingShutdownResult BeginShutdown() noexcept
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION

Indicates whether every topology-owned execution resource has ceased active execution work.

```cpp
bool IsExecutionQuiescent() noexcept
```

### `FinalizeShutdown`

**Classification:** PRIVATE IMPLEMENTATION

Tears down quiescent infrastructure, publishes terminal completion, and wakes shutdown waiters.

```cpp
ThreadingFinalizationResult FinalizeShutdown() noexcept
```

### `WaitForShutdown`

**Classification:** PRIVATE IMPLEMENTATION

Waits indefinitely for terminal shutdown completion from a managed Threading context.

```cpp
ShutdownWaitResult WaitForShutdown()
```

### `WaitForShutdownFor`

**Classification:** PRIVATE IMPLEMENTATION

Waits for terminal shutdown completion using one relative canonical monotonic-time budget.

```cpp
ShutdownWaitResult WaitForShutdownFor(
                Duration duration
            )
```

### `WaitForShutdownUntil`

**Classification:** PRIVATE IMPLEMENTATION

Waits for terminal shutdown completion until a canonical EDP-Clock monotonic deadline.

```cpp
ShutdownWaitResult WaitForShutdownUntil(
                MonotonicTimestamp deadline
            )
```

