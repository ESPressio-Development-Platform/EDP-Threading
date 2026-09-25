# src/threading/detail/WorkerLeaseScheduler.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `bce38a54cca268f3a25a435c2a378d4e8a25c622`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/bce38a54cca268f3a25a435c2a378d4e8a25c622/src/threading/detail/WorkerLeaseScheduler.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `optional`
- `FacilityStorage.hpp`
- `TaskRecord.hpp`

## Documented declarations

### `WorkerAvailabilityResult`

**Classification:** PRIVATE IMPLEMENTATION

Operation result for publishing Worker availability.

- `Available = 0` — the addressed Worker belongs to this scheduler and has been published available.
- `OutsideFacilityRange = 1` — the supplied topology context index does not belong to this facility's Worker range.
- `ProviderFailure = 2` — retained Threading result vocabulary for provider-mediated availability paths; the current bounded-set mutation itself cannot produce this outcome.

### `WorkerLeaseScheduler<TWorkerCount,TFirstContextIndex,TExecutionContextCapacity>`

**Classification:** PRIVATE IMPLEMENTATION

Owns facility-local Worker availability semantics over a shared `WorkerAvailabilitySet` while translating between topology-wide context indices and zero-based local Worker ordinals.

- **`TWorkerCount`** — number of Workers represented by the scheduler.
- **`TFirstContextIndex`** — first dense topology context index assigned to this Worker range.
- **`TExecutionContextCapacity`** — total managed execution-context capacity of the complete topology.

### `_availableWorkers`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

One shared bounded-set bit per facility Worker. A set bit means that Worker can accept a new Task grant. The set default-constructs empty, preserving the pre-migration rule that Workers are unavailable until they explicitly publish availability.

```cpp
WorkerAvailabilitySet<TWorkerCount> _availableWorkers;
```

### `WorkerOrdinal`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Converts one topology-wide managed execution-context index to this scheduler's zero-based Worker ordinal. The context scalar width is delegated to the shared bounded-index representation.

```cpp
std::size_t WorkerOrdinal(
    typename TopologyIndexTraits<
        ManagedContextIndexSpace,
        TExecutionContextCapacity
    >::Storage contextIndex
) const noexcept
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Dense topology-wide managed execution-context scalar Type, with width selected by `EDP-BoundedTopology::BoundedIndex` through `TopologyIndexTraits`.

```cpp
using ContextIndex = typename TopologyIndexTraits<
    ManagedContextIndexSpace,
    TExecutionContextCapacity
>::Storage;
```

### `WorkerCount`

Number of Workers represented by the scheduler.

### `FirstContextIndex`

First dense managed execution-context index owned by the facility.

### `IsWorkerContext(ContextIndex)`

Predicate testing whether one topology-wide context index belongs to this scheduler's Worker range.

### `ContextIndexForWorker(std::size_t)`

Converts a zero-based facility Worker ordinal back to its topology-wide context index.

### `MarkAvailable(ContextIndex)`

Validates facility membership, constructs the corresponding strong `WorkerAvailabilitySet::Index`, and sets its bit. No counter or additional state is updated.

### `TryClaimSpecific(ContextIndex)`

Returns no value if the context lies outside the facility or its Worker bit is clear. Otherwise clears the corresponding set bit and returns the same context index.

### `TryClaimAvailable()`

Finds the lowest set Worker bit, clears it, and returns the corresponding topology context index. If no bit is set, returns `std::nullopt`. Lowest-index selection is a bounded scan; no cached first-free index exists.

### `IsAnyAvailable()`

Delegates to `BoundedIndexSet::IsAnySet()` without retaining another Boolean.

### `AvailableCount()`

Delegates to the set's bounded `Count()` scan; no cached availability count is retained.

### `InUseCount()`

Computes `TWorkerCount - AvailableCount()`; no in-use counter is retained.

## Resource and concurrency contract

The only availability state is the shared one-bit-per-Worker set. Threading's existing facility synchronization remains responsible for serializing mutation. EDP-BoundedTopology adds no lock, allocation, provider object, or runtime capacity state.
