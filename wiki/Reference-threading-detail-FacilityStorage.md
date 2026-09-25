# src/threading/detail/FacilityStorage.hpp

**Primary classification:** PRIVATE IMPLEMENTATION / INTERNAL DOMAIN ADAPTATION

**Source baseline:** `395c0fd1d9bc513bacce155705957af91689652e`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/395c0fd1d9bc513bacce155705957af91689652e/src/threading/detail/FacilityStorage.hpp)

## Direct includes

- `cstddef`
- `ESPressio_BoundedTopology.hpp`
- `TopologyIndex.hpp`

## Purpose

This header no longer implements a Threading-local bitmap or intrusive queue. It gives Threading-semantic names to the shared EDP-BoundedTopology primitives. The aliases introduce no runtime state and no additional representation.

## Documented declarations

### `TaskRecordAvailabilitySet<TCapacity>`

**Classification:** PRIVATE IMPLEMENTATION / INTERNAL DOMAIN ADAPTATION

Shared one-bit bounded membership set representing structural availability of Task records.

- **Template parameter `TCapacity`:** compile-time Task-record capacity; controls the number of membership bits and strong Task-record index capacity.
- **Underlying Type:** `EDP-BoundedTopology::BoundedIndexSet<TaskRecordIndexSpace, TCapacity>`.
- **Semantic rule:** a set bit means the corresponding Task record is structurally available for admission. BoundedTopology itself does not define that meaning.
- **Storage:** exactly one bit per represented Task record for positive capacity; no cached count or synchronization state.

```cpp
template<std::size_t TCapacity>
using TaskRecordAvailabilitySet = ESPressio::BoundedTopology::BoundedIndexSet<
    TaskRecordIndexSpace,
    TCapacity
>;
```

### `TaskRecordQueue<TCapacity>`

**Classification:** PRIVATE IMPLEMENTATION / INTERNAL DOMAIN ADAPTATION

Shared non-owning intrusive FIFO topology used to order Task records in Threading's Queued lifecycle state.

- **Template parameter `TCapacity`:** compile-time Task-record capacity; controls endpoint index width and bounded traversal.
- **Underlying Type:** `EDP-BoundedTopology::IntrusiveQueue<TaskRecordIndexSpace, TCapacity>`.
- **Record contract:** participating Task records expose `QueueNext()` and `SetQueueNext(Index)`; the next-link remains in `TaskRecord::QueueOrExecutionContext`.
- **Storage:** head and tail strong bounded indices only; no queue-size counter or owned link table.
- **Synchronization:** none; `TaskFacilityRuntime` retains serialization responsibility.

```cpp
template<std::size_t TCapacity>
using TaskRecordQueue = ESPressio::BoundedTopology::IntrusiveQueue<
    TaskRecordIndexSpace,
    TCapacity
>;
```

### `WorkerAvailabilitySet<TCapacity>`

**Classification:** PRIVATE IMPLEMENTATION / INTERNAL DOMAIN ADAPTATION

Shared one-bit bounded membership set representing facility-local Worker availability.

- **Template parameter `TCapacity`:** compile-time Worker count represented by the set.
- **Underlying Type:** `EDP-BoundedTopology::BoundedIndexSet<WorkerLeaseIndexSpace, TCapacity>`.
- **Semantic rule:** a set bit means the Worker can accept a new Task grant.
- **Initial state:** default empty; Workers publish themselves available through `WorkerLeaseScheduler::MarkAvailable()`.
- **Storage:** exactly one bit per represented Worker for positive capacity.

```cpp
template<std::size_t TCapacity>
using WorkerAvailabilitySet = ESPressio::BoundedTopology::BoundedIndexSet<
    WorkerLeaseIndexSpace,
    TCapacity
>;
```

## Removed local implementation

The migration removed `AvailabilityClaimResult`, `TaskQueueRemovalResult`, `AvailabilityBitmap` and `IntrusiveTaskQueue`. Their generic mechanics now come directly from EDP-BoundedTopology; Threading retains only its semantic aliases and domain operations.
