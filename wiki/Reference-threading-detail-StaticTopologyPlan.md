# src/threading/detail/StaticTopologyPlan.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/StaticTopologyPlan.hpp)

## Direct includes

- `cstddef`
- `tuple`
- `type_traits`
- `../ThreadingComposition.hpp`

## Documented declarations

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TopologyContextOffset`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.

```cpp
template<class TTopology, std::size_t TResourceIndex>
    struct TopologyContextOffset
```

### `Sum`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Computes the managed-context prefix sum preceding one topology resource.
- **Template parameter `TIndices`:** Topology resource indices whose managed-context counts contribute to the prefix sum.

```cpp
template<std::size_t... TIndices>
            static constexpr std::size_t Sum(
                std::index_sequence<TIndices...>
            ) noexcept
```

### `Sum`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time result or index produced by this trait/specialization.

```cpp
static constexpr std::size_t Value = Sum(
                std::make_index_sequence<TResourceIndex>{}
            );
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TopologyResourceDescriptor`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.

```cpp
template<class TTopology, std::size_t TResourceIndex>
    struct TopologyResourceDescriptor final
```

### `Resource`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Topology resource Type described at this compile-time position.

```cpp
using Resource = std::tuple_element_t<
            TResourceIndex,
```

### `FirstContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

First dense managed-context index assigned to this resource.

```cpp
static constexpr std::size_t FirstContextIndex =
            TopologyContextOffset<TTopology, TResourceIndex>::Value;
```

### `ContextCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of managed execution contexts owned by this resource.

```cpp
static constexpr std::size_t ContextCount =
            ManagedContextCount<Resource>::Value;
```

### `EndContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

One-past-last dense managed-context index owned by this resource.

```cpp
static constexpr std::size_t EndContextIndex =
            FirstContextIndex + ContextCount;
```

### `TWorkers`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `WorkerDescriptor`.
- **Template parameter `TWorkers`:** Declared Worker set Type.
- **Template parameter `TWorkerIndex`:** Compile-time index of a Worker within its facility.

```cpp
template<class TWorkers, std::size_t TWorkerIndex>
    struct WorkerDescriptor;
```

### `WorkerDescriptor`

**Classification:** PRIVATE IMPLEMENTATION

Resolves one Worker declaration from a Worker-set pack by compile-time index.
- **Template parameter `TWorkers`:** Worker declaration Types contained by the Worker set.
- **Template parameter `TWorkerIndex`:** Compile-time index of the Worker being resolved.

```cpp
template<class... TWorkers, std::size_t TWorkerIndex>
    struct WorkerDescriptor<
        Workers<TWorkers...>,
```

### `WorkerType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Worker declaration Type selected from the facility Worker pack.

```cpp
using WorkerType = std::tuple_element_t<
            TWorkerIndex,
```

### `Properties`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved execution properties belonging to the selected Worker/resource.

```cpp
using Properties = typename WorkerType::Properties;
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `FacilityWorkerDescriptor`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.
- **Template parameter `TWorkerIndex`:** Compile-time index of a Worker within its facility.

```cpp
template<class TTopology, std::size_t TResourceIndex, std::size_t TWorkerIndex>
    struct FacilityWorkerDescriptor final
```

### `ResourceDescriptor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Compile-time descriptor for the owning topology resource.

```cpp
using ResourceDescriptor = TopologyResourceDescriptor<
                TTopology,
```

### `Facility`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Concrete Task facility Type represented by this descriptor.

```cpp
using Facility = typename ResourceDescriptor::Resource;
```

### `Worker`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compile-time descriptor for the selected Worker.

```cpp
using Worker = WorkerDescriptor<
                typename Facility::WorkerSet,
```

### `Properties`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Resolved execution properties belonging to the selected Worker/resource.

```cpp
using Properties = typename Worker::Properties;
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compact Type used to identify one managed execution context.

```cpp
static constexpr std::size_t ContextIndex =
                ResourceDescriptor::FirstContextIndex +
                TWorkerIndex;
```

### `TTopology`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `StaticTopologyPlan`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.

```cpp
template<class TTopology>
    class StaticTopologyPlan final
```

### `ResourceCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of resources declared by the topology.

```cpp
static constexpr std::size_t ResourceCount =
                TTopology::ResourceCount;
```

### `ManagedExecutionContextCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Total number of managed execution contexts in the topology.

```cpp
static constexpr std::size_t ManagedExecutionContextCount =
                TTopology::ManagedExecutionContextCount;
```

### `template<std::size_t TResourceIndex>`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Defines the compile-time contract for `Resource`.
- **Template parameter `TResourceIndex`:** Compile-time topology resource index.

```cpp
template<std::size_t TResourceIndex>
```

### `Resource`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Topology resource Type described at this compile-time position.

```cpp
using Resource = TopologyResourceDescriptor<
                TTopology,
```

