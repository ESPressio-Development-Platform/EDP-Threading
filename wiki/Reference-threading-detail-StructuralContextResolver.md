# src/threading/detail/StructuralContextResolver.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/StructuralContextResolver.hpp)

## Direct includes

- `cstddef`
- `optional`
- `tuple`
- `utility`
- `TopologyIndex.hpp`
- `WaitRegistration.hpp`

## Documented declarations

### `StructuralContextResolver`

**Classification:** PRIVATE IMPLEMENTATION

Resolves current-context identity and interruption state structurally across topology-owned resources.
- **Template parameter `TContextCapacity`:** Number of managed execution contexts in the topology.
- **Template parameter `TResources`:** Concrete statically owned runtime resource Types participating in resolution.

```cpp
template<std::size_t TContextCapacity, class... TResources>
    class StructuralContextResolver;
```

### `StructuralContextResolver`

**Classification:** PRIVATE IMPLEMENTATION

Empty-topology resolver specialization retaining no resource references.

```cpp
template<>
    class StructuralContextResolver<0U> final
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compact Type used to identify one managed execution context.

```cpp
using ContextIndex = typename TopologyIndexTraits<ManagedContextIndexSpace, 1U>::Storage;
```

### `ResolveCurrentThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Reports that an empty topology has no currently executing managed context.

```cpp
static std::optional<ContextIndex> ResolveCurrentThunk(
                const void*
            ) noexcept
```

### `IsInterruptedThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Conservatively reports interruption for any impossible context query against an empty topology.

```cpp
static bool IsInterruptedThunk(
                const void*,
                ContextIndex
            ) noexcept
```

### `StructuralContextResolver`

**Classification:** PRIVATE IMPLEMENTATION

Resolves managed-context identity and interruption structurally across topology-owned runtime resources.
- **Template parameter `TContextCapacity`:** Number of managed execution contexts represented by the topology.
- **Template parameter `TResources`:** Concrete topology-owned runtime resource Types searched by the resolver.

```cpp
template<std::size_t TContextCapacity, class... TResources>
    class StructuralContextResolver final
```

### `ContextIndexStorage`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Internal compact Type used while resolving managed execution contexts.

```cpp
using ContextIndexStorage =
                typename ExecutionContextIndexTraits<TContextCapacity>::Type;
```

### `_resources`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning pointers to the topology-owned runtime resources searched for context identity/interruption.

```cpp
std::tuple<TResources*...> _resources;
```

### `ResolveCurrentNext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resolves the current managed execution context by scanning topology resources in declaration order.
- **Template parameter `TIndex`:** Compile-time resource index currently being inspected.

```cpp
template<std::size_t TIndex>
            std::optional<ContextIndexStorage> ResolveCurrentNext() const noexcept
```

### `IsInterruptedNext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resolves whether one managed execution context carries an authoritative interruption request.
- **Template parameter `TIndex`:** Compile-time resource index currently being inspected.

```cpp
template<std::size_t TIndex>
            bool IsInterruptedNext(
                ContextIndexStorage contextIndex
            ) const noexcept
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compact Type used to identify one managed execution context.

```cpp
using ContextIndex = ContextIndexStorage;
```

### `StructuralContextResolver`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Binds the resolver to the address-stable topology-owned runtime resources.

```cpp
explicit StructuralContextResolver(
                TResources&... resources
            ) noexcept :
                _resources(
                    &resources...
                ) {}
```

### `ResolveCurrentThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Type-erased router bridge that resolves the currently executing managed context.

```cpp
static std::optional<ContextIndex> ResolveCurrentThunk(
                const void* context
            ) noexcept
```

### `IsInterruptedThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Type-erased router bridge that resolves authoritative interruption for one managed context.

```cpp
static bool IsInterruptedThunk(
                const void* context,
                ContextIndex contextIndex
            ) noexcept
```

