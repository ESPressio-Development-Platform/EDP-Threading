# src/threading/detail/StaticTopologyResourceStorage.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/StaticTopologyResourceStorage.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `tuple`
- `type_traits`
- `utility`
- `InfrastructureLifecycle.hpp`
- `StaticTopologyResourceTypes.hpp`

## Documented declarations

### `TopologyShutdownApplicationResult`

**Classification:** PRIVATE IMPLEMENTATION

Outcome of applying terminal semantic shutdown across recursive topology resource storage.

```cpp
enum class TopologyShutdownApplicationResult : std::uint8_t
```

### `TDeclaration`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `DeclarationThreadIdentity`.
- **Template parameter `TDeclaration`:** Static topology declaration Type being realized.

```cpp
template<class TDeclaration>
    struct DeclarationThreadIdentity;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Extracts the semantic Thread identity from a Dedicated Thread declaration.
- **Template parameter `TThreadIdentity`:** Semantic identity Type carried by the Dedicated Thread declaration.
- **Template parameter `TProperties`:** Compile-time execution-property Types carried by the declaration.

```cpp
template<class TThreadIdentity, class... TProperties>
    struct DeclarationThreadIdentity<
        DedicatedThread<TThreadIdentity, TProperties...>
    >
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved Type produced by this compile-time helper.

```cpp
using Type = TThreadIdentity;
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `OwnedResourceAt`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.
Resolves the concrete owned runtime Type corresponding to one topology resource.

```cpp
template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TResourceIndex>
    using OwnedResourceAt = typename OwnedResourceType<
        typename StaticTopologyPlan<TTopology>::template Resource<TResourceIndex>::Resource,
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ConstructOwnedResource`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TSpinLockProvider`:** Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.

```cpp
template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider, std::size_t TResourceIndex>
    auto ConstructOwnedResource(
        TBindings& bindings,
        TManagedContextRouter& router,
        InfrastructureLifecycle<TSpinLockProvider>& lifecycle
    )
```

### `Descriptor`

**Classification:** PRIVATE IMPLEMENTATION

Compile-time descriptor for the resource being constructed.

```cpp
using Descriptor =
            typename StaticTopologyPlan<TTopology>::template Resource<TResourceIndex>;
```

### `Declaration`

**Classification:** PRIVATE IMPLEMENTATION

Static resource declaration Type represented by this storage node.

```cpp
using Declaration = typename Descriptor::Resource;
```

### `Resource`

**Classification:** PRIVATE IMPLEMENTATION

Topology resource Type described at this compile-time position.

```cpp
using Resource = OwnedResourceAt<
            TTopology,
```

### `ThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Semantic identity Type of a Dedicated Thread declaration.

```cpp
using ThreadIdentity =
                typename DeclarationThreadIdentity<Declaration>::Type;
```

### `Binding`

**Classification:** PRIVATE IMPLEMENTATION

Application callable-binding Type matched to a Dedicated Thread.

```cpp
using Binding = typename DedicatedThreadBindingTypeFromTuple<
                ThreadIdentity,
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `StaticTopologyResourceStorage`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TSpinLockProvider`:** Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.
- **Template parameter `TComplete`:** Whether recursive static resource storage has reached its terminal specialization.

```cpp
template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider, std::size_t TResourceIndex, bool TComplete = (TResourceIndex == TTopology::ResourceCount)>
    class StaticTopologyResourceStorage;
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `StaticTopologyResourceStorage`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TSpinLockProvider`:** Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.

```cpp
template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider, std::size_t TResourceIndex>
    class StaticTopologyResourceStorage<
        TTopology,
```

### `Resource`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Topology resource Type described at this compile-time position.

```cpp
using Resource = OwnedResourceAt<
                TTopology,
```

### `Tail`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Recursive lookup result for the remaining topology resource pack.

```cpp
using Tail = StaticTopologyResourceStorage<
                TTopology,
```

### `_resource`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Runtime resource owned at this compile-time topology position.

```cpp
Resource _resource;
```

### `_tail`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Recursively owned storage for every later topology resource.

```cpp
Tail _tail;
```

### `StaticTopologyResourceStorage`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs this resource and the recursive tail directly in topology storage order.

```cpp
StaticTopologyResourceStorage(
                TBindings& bindings,
                TManagedContextRouter& router,
                InfrastructureLifecycle<TSpinLockProvider>& lifecycle
            ) :
                _resource(
                    ConstructOwnedResource<
                        TTopology,
                        TBindings,
                        TManagedContextRouter,
                        TExecutionContextProvider,
                        TSpinLockProvider,
                        TMutexProvider,
                        TResourceIndex
                    >(
                        bindings,
                        router,
                        lifecycle
                    )
                ),
```

### `Get`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `Get`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            auto& Get() noexcept
```

### `BeginShutdown`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Applies terminal semantic shutdown behavior to this resource and then the recursive tail.

```cpp
TopologyShutdownApplicationResult BeginShutdown() noexcept
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this resource and every recursive tail resource are execution-quiescent.

```cpp
bool IsExecutionQuiescent() noexcept
```

### `FinalizeShutdown`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Terminates, joins, and destroys this infrastructure resource before finalizing the recursive tail.

```cpp
ThreadingFinalizationResult FinalizeShutdown() noexcept
```

### `Get`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `Get`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<std::size_t TIndex>
            const auto& Get() const noexcept
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `StaticTopologyResourceStorage`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TSpinLockProvider`:** Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.

```cpp
template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider, std::size_t TResourceIndex>
    class StaticTopologyResourceStorage<
        TTopology,
```

### `StaticTopologyResourceStorage`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs the terminal recursive storage node without owning any resource state.

```cpp
StaticTopologyResourceStorage(
                TBindings&,
                TManagedContextRouter&,
                InfrastructureLifecycle<TSpinLockProvider>&
            ) noexcept {}
```

### `BeginShutdown`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Performs no semantic shutdown work because the terminal node owns no resource.

```cpp
TopologyShutdownApplicationResult BeginShutdown() noexcept
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Reports quiescence because the terminal node owns no execution resource.

```cpp
bool IsExecutionQuiescent() noexcept
```

### `FinalizeShutdown`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Completes finalization because the terminal node owns no infrastructure resource.

```cpp
ThreadingFinalizationResult FinalizeShutdown() noexcept
```

