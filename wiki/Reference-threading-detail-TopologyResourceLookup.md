# src/threading/detail/TopologyResourceLookup.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/detail/TopologyResourceLookup.hpp)

## Direct includes

- `cstddef`
- `tuple`
- `type_traits`
- `../ThreadingComposition.hpp`

## Documented declarations

### `TResource`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsTaskFacilityIdentity`.
- **Template parameter `TResource`:** Topology resource Type being classified.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.

```cpp
template<class TResource, class TPoolIdentity>
    struct IsTaskFacilityIdentity
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or topology index produced by this trait/specialization.

```cpp
static constexpr bool Value = false;
```

### `TPoolIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsTaskFacilityIdentity`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type.
- **Template parameter `TWorkers`:** Declared Worker set Type.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
    struct IsTaskFacilityIdentity<
        TaskExecutionFacility<
            TPoolIdentity,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or topology index produced by this trait/specialization.

```cpp
static constexpr bool Value = true;
```

### `TResource`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsDedicatedWorkerIdentity`.
- **Template parameter `TResource`:** Topology resource Type being classified.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.

```cpp
template<class TResource, class TTaskIdentity>
    struct IsDedicatedWorkerIdentity
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or topology index produced by this trait/specialization.

```cpp
static constexpr bool Value = false;
```

### `TTaskIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsDedicatedWorkerIdentity`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
Matches one Dedicated Worker lease declaration against a requested Task identity.
- **Template parameter `TTaskIdentity`:** Semantic Task identity Type carried by the Dedicated Worker lease.
- **Template parameter `TProperties`:** Compile-time capacity/execution-property Types carried by the lease.

```cpp
template<class TTaskIdentity, class... TProperties>
    struct IsDedicatedWorkerIdentity<
        DedicatedWorkerLease<
            TTaskIdentity,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or topology index produced by this trait/specialization.

```cpp
static constexpr bool Value = true;
```

### `TResource`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsDedicatedThreadIdentity`.
- **Template parameter `TResource`:** Topology resource Type being classified.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.

```cpp
template<class TResource, class TThreadIdentity>
    struct IsDedicatedThreadIdentity
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or topology index produced by this trait/specialization.

```cpp
static constexpr bool Value = false;
```

### `TThreadIdentity`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IsDedicatedThreadIdentity`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
Matches one Dedicated Thread declaration against a requested Thread identity.
- **Template parameter `TThreadIdentity`:** Semantic identity Type carried by the Dedicated Thread declaration.
- **Template parameter `TProperties`:** Compile-time execution-property Types carried by the declaration.

```cpp
template<class TThreadIdentity, class... TProperties>
    struct IsDedicatedThreadIdentity<
        DedicatedThread<
            TThreadIdentity,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or topology index produced by this trait/specialization.

```cpp
static constexpr bool Value = true;
```

### `TMatcher`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TopologyResourceIndex`.
- **Template parameter `TMatcher`:** Compile-time identity matcher template.
- **Template parameter `TIdentity`:** Semantic identity Type being located.
- **Template parameter `TResources`:** Remaining topology resource pack searched recursively.
- **Template parameter `TIndex`:** Compile-time topology resource index.

```cpp
template<template<class, class> class TMatcher, class TIdentity, class TResources, std::size_t TIndex = 0U>
    struct TopologyResourceIndex;
```

### `TMatcher`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TopologyResourceIndex`.
- **Template parameter `TMatcher`:** Compile-time identity matcher template.
- **Template parameter `TIdentity`:** Semantic identity Type being located.
- **Template parameter `TIndex`:** Compile-time topology resource index.

```cpp
template<template<class, class> class TMatcher, class TIdentity, std::size_t TIndex>
    struct TopologyResourceIndex<
        TMatcher,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or topology index produced by this trait/specialization.

```cpp
static constexpr std::size_t Value = TopologyResourceNotFound;
```

### `TMatcher`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TopologyResourceIndex`.
- **Template parameter `TMatcher`:** Compile-time identity matcher template.
- **Template parameter `TIdentity`:** Semantic identity Type being located.
- **Template parameter `TFirstResource`:** First topology resource Type in the recursive lookup.
- **Template parameter `TIndex`:** Compile-time topology resource index.
Recursively locates a topology resource whose semantic identity satisfies the supplied matcher.
- **Template parameter `TMatcher`:** Compile-time identity matcher applied to each resource.
- **Template parameter `TIdentity`:** Semantic identity Type being searched for.
- **Template parameter `TFirstResource`:** First topology resource Type inspected by this recursion step.
- **Template parameter `TRestResources`:** Remaining topology resource Types searched when the first does not match.
- **Template parameter `TIndex`:** Compile-time topology index corresponding to the first resource in this recursion step.

```cpp
template<template<class, class> class TMatcher, class TIdentity, class TFirstResource, class... TRestResources, std::size_t TIndex>
    struct TopologyResourceIndex<
        TMatcher,
```

### `Tail`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Recursive lookup result for the remaining topology resource pack.

```cpp
static constexpr std::size_t Tail = TopologyResourceIndex<
            TMatcher,
```

### `Value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or topology index produced by this trait/specialization.

```cpp
static constexpr std::size_t Value =
            TMatcher<TFirstResource, TIdentity>::Value
                ? TIndex
                : Tail;
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `templated declaration`.
- **Template parameter `TTopology`:** Compile-time Threading topology being searched.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.

```cpp
template<class TTopology, class TPoolIdentity>
    inline constexpr std::size_t TaskFacilityResourceIndex =
        TopologyResourceIndex<
            IsTaskFacilityIdentity,
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `templated declaration`.
- **Template parameter `TTopology`:** Compile-time Threading topology being searched.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.

```cpp
template<class TTopology, class TTaskIdentity>
    inline constexpr std::size_t DedicatedWorkerResourceIndex =
        TopologyResourceIndex<
            IsDedicatedWorkerIdentity,
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `templated declaration`.
- **Template parameter `TTopology`:** Compile-time Threading topology being searched.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.

```cpp
template<class TTopology, class TThreadIdentity>
    inline constexpr std::size_t DedicatedThreadResourceIndex =
        TopologyResourceIndex<
            IsDedicatedThreadIdentity,
```

