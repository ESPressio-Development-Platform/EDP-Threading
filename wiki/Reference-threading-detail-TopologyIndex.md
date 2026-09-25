# src/threading/detail/TopologyIndex.hpp

**Primary classification:** PRIVATE IMPLEMENTATION / INTERNAL DOMAIN ADAPTATION

**Source baseline:** `ced4c770ac9b3c6078b9b694c9e63a27da343d82`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ced4c770ac9b3c6078b9b694c9e63a27da343d82/src/threading/detail/TopologyIndex.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `limits`
- `ESPressio_BoundedTopology.hpp`

## Purpose

`TopologyIndex.hpp` is the zero-state adaptation boundary between EDP-BoundedTopology's strong bounded index identity and Threading's existing raw compact scalar fields. It delegates width and sentinel selection rather than reimplementing those mechanics.

## Documented declarations

### `TaskRecordIndexSpace`

**Classification:** PRIVATE IMPLEMENTATION

Empty semantic tag distinguishing Task-record bounded identities from unrelated bounded spaces. The tag contributes no runtime state.

### `ManagedContextIndexSpace`

**Classification:** PRIVATE IMPLEMENTATION

Empty semantic tag for the dense managed execution-context index space. It contributes no runtime state.

### `WorkerLeaseIndexSpace`

**Classification:** PRIVATE IMPLEMENTATION

Empty semantic tag for facility-local Worker availability identities. It contributes no runtime state.

### `TaskScratchIndexSpace`

**Classification:** PRIVATE IMPLEMENTATION

Empty semantic tag used only to select the smallest shared bounded-index storage width capable of representing `TaskRecord`'s mutually exclusive queue-link or execution-context scratch values. No strong scratch object is retained.

### `TopologyIndexTraits<TIndexSpace, TCapacity>`

**Classification:** PRIVATE IMPLEMENTATION / INTERNAL DOMAIN ADAPTATION

Adapts one positive Threading index capacity to the shared `EDP-BoundedTopology::BoundedIndex` representation while retaining Threading's established maximum 32-bit compact-index contract.

- **Template parameter `TIndexSpace`:** semantic compile-time bounded-index tag.
- **Template parameter `TCapacity`:** positive compile-time capacity. Values above the maximum `uint32_t` bounded-index range are rejected to preserve the pre-migration Threading contract.
- **Runtime state:** none.

#### `Strong`

Strong `EDP-BoundedTopology::BoundedIndex<TIndexSpace, TCapacity>` Type used at topology boundaries.

#### `Storage`

Smallest raw unsigned scalar selected by `Strong::Storage`. Threading retains this scalar where lifecycle packing/reuse makes storing a strong object inappropriate.

#### `Invalid`

Raw scalar invalid sentinel delegated from `Strong::InvalidValue`. No separate validity byte or sentinel algorithm is retained by Threading.
