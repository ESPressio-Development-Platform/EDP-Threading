# src/threading/detail/StaticTopologyResourceTypes.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/StaticTopologyResourceTypes.hpp)

## Direct includes

- `cstddef`
- `tuple`
- `type_traits`
- `../ThreadingComposition.hpp`
- `DedicatedThreadOwnedRuntime.hpp`
- `DedicatedWorkerOwnedRuntime.hpp`
- `StaticTopologyPlan.hpp`
- `TaskFacilityOwnedRuntime.hpp`
- `TopologyResourceLookup.hpp`

## Documented declarations

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Determines whether one binding targets a specific Dedicated Thread identity.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread being matched.
- **Template parameter `TBinding`:** Dedicated Thread binding Type being inspected.

```cpp
template<class TThreadIdentity, class TBinding>
    struct IsDedicatedThreadBindingFor
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = false;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsDedicatedThreadBindingFor`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.

```cpp
template<class TThreadIdentity, class TCallable>
    struct IsDedicatedThreadBindingFor<
        TThreadIdentity,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = true;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Counts bindings that target one Dedicated Thread identity.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread being counted.
- **Template parameter `TBindings`:** Binding Types searched for matching Thread identities.

```cpp
template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingCount
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of matching Dedicated Thread bindings found in the supplied binding pack.

```cpp
static constexpr std::size_t Value =
            (
                static_cast<std::size_t>(
                    IsDedicatedThreadBindingFor<
                        TThreadIdentity,
                        TBindings
                    >::Value
                ) +
                ... +
                0U
            );
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Resolves the binding Type associated with one Dedicated Thread identity.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread being resolved.
- **Template parameter `TBindings`:** Binding Types searched for the matching Thread identity.

```cpp
template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingType;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Resolves one Dedicated Thread binding from an application binding tuple.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread being resolved.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.

```cpp
template<class TThreadIdentity, class TBindings>
    struct DedicatedThreadBindingTypeFromTuple;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Resolves one Dedicated Thread binding from a concrete tuple specialization.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread being resolved.
- **Template parameter `TBindings`:** Binding Types contained by the application tuple.

```cpp
template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingTypeFromTuple<
        TThreadIdentity,
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved Type produced by this compile-time helper.

```cpp
using Type = typename DedicatedThreadBindingType<
            TThreadIdentity,
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Terminates binding lookup when no matching Dedicated Thread binding exists.
- **Template parameter `TThreadIdentity`:** Semantic identity Type whose binding lookup reached the empty tail.

```cpp
template<class TThreadIdentity>
    struct DedicatedThreadBindingType<TThreadIdentity>
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved Type produced by this compile-time helper.

```cpp
using Type = void;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Resolves a Dedicated Thread binding recursively from the supplied binding pack.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread being resolved.
- **Template parameter `TFirstBinding`:** First binding Type inspected by this recursive specialization.
- **Template parameter `TRestBindings`:** Remaining binding Types searched when the first binding does not match.

```cpp
template<class TThreadIdentity, class TFirstBinding, class... TRestBindings>
    struct DedicatedThreadBindingType<
        TThreadIdentity,
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved Type produced by this compile-time helper.

```cpp
using Type = std::conditional_t<
            IsDedicatedThreadBindingFor<
                TThreadIdentity,
```

### `TResource`

**Classification:** PRIVATE IMPLEMENTATION

Validates application binding requirements for one topology resource.
- **Template parameter `TResource`:** Topology resource Type whose binding requirements are checked.
- **Template parameter `TBindings`:** Application Dedicated Thread binding Types available to the topology.

```cpp
template<class TResource, class... TBindings>
    struct ResourceBindingIsValid
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = true;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Validates that a Dedicated Thread declaration has exactly one matching application binding.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread being validated.
- **Template parameter `TProperties`:** Compile-time execution properties declared for the Dedicated Thread.
- **Template parameter `TBindings`:** Application binding Types searched for the matching Thread identity.

```cpp
template<class TThreadIdentity, class... TProperties, class... TBindings>
    struct ResourceBindingIsValid<
        DedicatedThread<TThreadIdentity, TProperties...>,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value =
            DedicatedThreadBindingCount<
                TThreadIdentity,
```

### `TBinding`

**Classification:** PRIVATE IMPLEMENTATION

Determines whether one supplied binding targets a Dedicated Thread declared by the topology.
- **Template parameter `TBinding`:** Dedicated Thread binding Type being inspected.
- **Template parameter `TResources`:** Topology resource Types searched for the binding's Thread identity.

```cpp
template<class TBinding, class... TResources>
    struct BindingMatchesDeclaredThread
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = false;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Matches one concrete Dedicated Thread binding against the topology resource pack.
- **Template parameter `TThreadIdentity`:** Semantic identity Type carried by the binding.
- **Template parameter `TCallable`:** Callable Type carried by the binding.
- **Template parameter `TResources`:** Topology resource Types searched for the bound Thread identity.

```cpp
template<class TThreadIdentity, class TCallable, class... TResources>
    struct BindingMatchesDeclaredThread<
        DedicatedThreadBinding<TThreadIdentity, TCallable>,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value =
            (
                IsDedicatedThreadIdentity<
                    TResources,
                    TThreadIdentity
                >::Value ||
                ... ||
                false
            );
```

### `TResource`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsTaskExecutionResource`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.

```cpp
template<class TResource>
    struct IsTaskExecutionResource
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = false;
```

### `TPoolIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsTaskExecutionResource`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type or bounded capacity.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type or byte capacity.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type or byte capacity.
- **Template parameter `TWorkers`:** Declared Worker set Type.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
    struct IsTaskExecutionResource<
        TaskExecutionFacility<
            TPoolIdentity,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = true;
```

### `TTaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Identifies a Dedicated Worker lease as a Task-execution resource.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TProperties`:** Compile-time capacities and execution properties declared by the lease.

```cpp
template<class TTaskIdentity, class... TProperties>
    struct IsTaskExecutionResource<
        DedicatedWorkerLease<
            TTaskIdentity,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = true;
```

### `TBinding`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsDedicatedThreadBinding`.
- **Template parameter `TBinding`:** Dedicated Thread binding Type being inspected.

```cpp
template<class TBinding>
    struct IsDedicatedThreadBinding
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = false;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsDedicatedThreadBinding`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.

```cpp
template<class TThreadIdentity, class TCallable>
    struct IsDedicatedThreadBinding<
        DedicatedThreadBinding<TThreadIdentity, TCallable>
    >
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value = true;
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ValidDedicatedThreadBindings`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.

```cpp
template<class TTopology, class TBindings>
    struct ValidDedicatedThreadBindings;
```

### `ValidDedicatedThreadBindings`

**Classification:** PRIVATE IMPLEMENTATION

Validates all Dedicated Thread declarations and supplied bindings as a complete topology relationship.
- **Template parameter `TResources`:** Topology resource Types whose Dedicated Threads require bindings.
- **Template parameter `TBindings`:** Application binding Types that must each target a declared Dedicated Thread.

```cpp
template<class... TResources, class... TBindings>
    struct ValidDedicatedThreadBindings<
        ThreadingTopology<TResources...>,
```

### `EveryThreadBound`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Whether every declared Dedicated Thread has exactly one matching binding.

```cpp
static constexpr bool EveryThreadBound =
            (
                ResourceBindingIsValid<
                    TResources,
                    TBindings...
                >::Value &&
                ... &&
                true
            );
```

### `EveryBindingDeclared`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Whether every supplied Dedicated Thread binding targets a declared thread.

```cpp
static constexpr bool EveryBindingDeclared =
            (
                (
                    IsDedicatedThreadBinding<TBindings>::Value &&
                    BindingMatchesDeclaredThread<
                        TBindings,
                        TResources...
                    >::Value
                ) &&
                ... &&
                true
            );
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this compile-time trait specialization matches its requested condition.

```cpp
static constexpr bool Value =
            EveryThreadBound &&
            EveryBindingDeclared;
```

### `TDeclaration`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `OwnedResourceType`.
- **Template parameter `TDeclaration`:** Static topology declaration Type being realized.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TDeclaration, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType;
```

### `TPoolIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Maps one ordinary Task facility declaration to its concrete statically owned runtime Type.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type or bounded capacity.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type or byte capacity.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type or byte capacity.
- **Template parameter `TWorkers`:** Declared Worker set Type.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TBindings`:** Application Dedicated Thread binding Types carried through the uniform resource-mapping interface.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType<
        TaskExecutionFacility<
            TPoolIdentity,
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved Type produced by this compile-time helper.

```cpp
using Type = TaskFacilityOwnedRuntime<
            TaskExecutionFacility<
                TPoolIdentity,
```

### `TTaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Maps one Dedicated Worker lease declaration to its concrete statically owned runtime Type.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TProperties`:** Compile-time capacities and execution properties declared by the Dedicated Worker lease.
- **Template parameter `TBindings`:** Application Dedicated Thread binding Types carried through the uniform resource-mapping interface.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TTaskIdentity, class... TProperties, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType<
        DedicatedWorkerLease<TTaskIdentity, TProperties...>,
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved Type produced by this compile-time helper.

```cpp
using Type = DedicatedWorkerOwnedRuntime<
            DedicatedWorkerLease<
                TTaskIdentity,
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Maps one Dedicated Thread declaration and its binding to the concrete statically owned runtime Type.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TProperties`:** Compile-time execution properties declared by the Dedicated Thread.
- **Template parameter `TBindings`:** Application binding Types used to resolve the Thread callable.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TContextIndex`:** Dense topology execution-context index assigned to the resource.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<class TThreadIdentity, class... TProperties, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType<
        DedicatedThread<TThreadIdentity, TProperties...>,
```

### `Binding`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Application callable-binding Type matched to a Dedicated Thread.

```cpp
using Binding = typename DedicatedThreadBindingType<
            TThreadIdentity,
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved Type produced by this compile-time helper.

```cpp
using Type = DedicatedThreadOwnedRuntime<
            DedicatedThread<
                TThreadIdentity,
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Constructs the tuple Type containing every concrete topology-owned runtime resource.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TIndices`:** Compile-time topology resource indices expanded into the resulting tuple Type.

```cpp
template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t... TIndices>
    auto OwnedResourceTupleType(
        std::index_sequence<TIndices...>
    ) -> std::tuple<
        typename OwnedResourceType<
            typename StaticTopologyPlan<TTopology>::template Resource<TIndices>::Resource,
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Resolves the concrete owned-resource tuple Type for a complete Threading topology.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.

```cpp
template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider>
    using OwnedResourceTuple = decltype(
        OwnedResourceTupleType<
            TTopology,
            TBindings,
            TManagedContextRouter,
            TExecutionContextProvider,
            TMutexProvider
        >(
            std::make_index_sequence<TTopology::ResourceCount>{}
        )
    );
```

