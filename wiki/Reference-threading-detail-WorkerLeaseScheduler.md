# src/threading/detail/WorkerLeaseScheduler.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/detail/WorkerLeaseScheduler.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `optional`
- `FacilityStorage.hpp`
- `TaskRecord.hpp`

## Documented declarations

### `WorkerLeaseScheduler`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `WorkerLeaseScheduler`.
- **Template parameter `TWorkerCount`:** Number of Workers represented by the lease scheduler.
- **Template parameter `TFirstContextIndex`:** First dense topology context index assigned to the Worker set.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<std::size_t TWorkerCount, std::size_t TFirstContextIndex, std::size_t TExecutionContextCapacity>
    class WorkerLeaseScheduler final
```

### `AvailabilityBitmap<TWorkerCount> _availableWorkers{false};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

One bit per Worker; one means available for a new Task grant.

```cpp
AvailabilityBitmap<TWorkerCount> _availableWorkers{false};
```

### `WorkerOrdinal`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Returns the zero-based facility Worker ordinal for one topology context index.

```cpp
std::size_t WorkerOrdinal(
                typename SmallestIndex<TExecutionContextCapacity>::Type contextIndex
            ) const noexcept
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Dense topology-wide managed execution-context index Type.

```cpp
using ContextIndex = typename SmallestIndex<TExecutionContextCapacity>::Type;
```

### `WorkerCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of Workers represented by this scheduler.

```cpp
static constexpr std::size_t WorkerCount = TWorkerCount;
```

### `FirstContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

First dense managed execution-context index owned by this facility.

```cpp
static constexpr std::size_t FirstContextIndex = TFirstContextIndex;
```

### `IsWorkerContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether one managed execution-context index belongs to this facility's Workers.

```cpp
bool IsWorkerContext(
                ContextIndex contextIndex
            ) const noexcept
```

### `ContextIndexForWorker`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Converts one zero-based Worker ordinal to its topology-wide dense context index.

```cpp
ContextIndex ContextIndexForWorker(
                std::size_t workerOrdinal
            ) const noexcept
```

### `MarkAvailable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Publishes one facility Worker as available for a new Task grant.

```cpp
WorkerAvailabilityResult MarkAvailable(
                ContextIndex contextIndex
            ) noexcept
```

### `TryClaimSpecific`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Attempts to reserve one specific Worker when it belongs to this facility and is available.

```cpp
std::optional<ContextIndex> TryClaimSpecific(
                ContextIndex contextIndex
            ) noexcept
```

### `TryClaimAvailable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Attempts to reserve the lowest-index currently available Worker.

```cpp
std::optional<ContextIndex> TryClaimAvailable() noexcept
```

### `IsAnyAvailable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether at least one facility Worker can accept a Task grant now.

```cpp
bool IsAnyAvailable() const noexcept
```

### `AvailableCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the number of currently available Workers.

```cpp
std::size_t AvailableCount() const noexcept
```

### `InUseCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the number of Workers currently granted to Tasks.

```cpp
std::size_t InUseCount() const noexcept
```

