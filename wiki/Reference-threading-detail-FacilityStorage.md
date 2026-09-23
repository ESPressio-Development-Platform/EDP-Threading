# src/threading/detail/FacilityStorage.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `50dbad50650df9fc38dd17cd3b17a60a92a5d802`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/50dbad50650df9fc38dd17cd3b17a60a92a5d802/src/threading/detail/FacilityStorage.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `TaskRecord.hpp`

## Documented declarations

### `AvailabilityClaimResult`

**Classification:** PRIVATE IMPLEMENTATION

Outcome of claiming one bounded availability slot.

```cpp
enum class AvailabilityClaimResult : std::uint8_t
```

### `TaskQueueRemovalResult`

**Classification:** PRIVATE IMPLEMENTATION

Outcome of removing one record from an intrusive Task queue.

```cpp
enum class TaskQueueRemovalResult : std::uint8_t
```

### `AvailabilityBitmap`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `AvailabilityBitmap`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
    class AvailabilityBitmap final
```

### `ByteBits`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Number of availability bits stored in each byte.

```cpp
static constexpr std::size_t ByteBits = 8U;
```

### `ByteCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Number of bytes required to represent every bounded record.

```cpp
static constexpr std::size_t ByteCount =
                (TCapacity + ByteBits - 1U) / ByteBits;
```

### `std::uint8_t _bytes[ByteCount] = {};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

One bit per Task record; one means available.

```cpp
std::uint8_t _bytes[ByteCount] = {};
```

### `ValidMask`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Returns the mask of valid record bits in one bitmap byte.

```cpp
static constexpr std::uint8_t ValidMask(
                std::size_t byteIndex
            ) noexcept
```

### `AvailabilityBitmap`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates a bounded bitmap with every entry either available or unavailable.

```cpp
explicit AvailabilityBitmap(
                bool initiallyAvailable = true
            ) noexcept
```

### `IsAnyAvailable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether at least one bounded entry is currently available.

```cpp
bool IsAnyAvailable() const noexcept
```

### `AvailableCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the number of currently available bounded entries.

```cpp
std::size_t AvailableCount() const noexcept
```

### `TryClaim`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Attempts to claim the lowest-index available record.

The owning Task facility must serialize this operation with all other bitmap
and queue mutation. The bitmap deliberately carries no duplicated lock state.

```cpp
AvailabilityClaimResult TryClaim(
                std::size_t& recordIndex
            ) noexcept
```

### `TryClaimSpecific`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Attempts to claim one specific bounded entry when it is currently available.

```cpp
AvailabilityClaimResult TryClaimSpecific(
                std::size_t recordIndex
            ) noexcept
```

### `Release`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Republishes one fully reclaimed record as available.

The owning Task facility must serialize this operation with admission and reclamation.

```cpp
void Release(
                std::size_t recordIndex
            ) noexcept
```

### `IsAvailable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether one record is currently published as available.

```cpp
bool IsAvailable(
                std::size_t recordIndex
            ) const noexcept
```

### `IntrusiveTaskQueue`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `IntrusiveTaskQueue`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
    class IntrusiveTaskQueue final
```

### `StorageIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Internal smallest index Type satisfying the configured Task-record capacity.

```cpp
using StorageIndex = typename SmallestIndex<TCapacity>::Type;
```

### `InvalidStorageIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Internal sentinel which cannot identify a valid record.

```cpp
static constexpr StorageIndex InvalidStorageIndex = SmallestIndex<TCapacity>::Invalid;
```

### `_head`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

First queued record.

```cpp
StorageIndex _head = InvalidStorageIndex;
```

### `_tail`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Last queued record.

```cpp
StorageIndex _tail = InvalidStorageIndex;
```

### `Index`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Smallest public index Type satisfying the configured Task-record capacity.

```cpp
using Index = StorageIndex;
```

### `InvalidIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Public sentinel which cannot identify a valid record.

```cpp
static constexpr Index InvalidIndex = InvalidStorageIndex;
```

### `IsEmpty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the queue contains no Task records.

```cpp
bool IsEmpty() const noexcept
```

### `Head`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the first queued record index or the invalid sentinel.

```cpp
Index Head() const noexcept
```

### `TRecords`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Appends one record whose queue-link field is supplied by the caller.
- **Template parameter `TRecords`:** Task-record collection Type whose intrusive queue links are manipulated.

```cpp
template<class TRecords>
            void Push(
                TRecords& records,
                Index recordIndex
            ) noexcept
```

### `TRecords`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Removes and returns the first queued record or the invalid sentinel.
- **Template parameter `TRecords`:** Task-record collection Type whose intrusive queue links are manipulated.

```cpp
template<class TRecords>
            Index Pop(
                TRecords& records
            ) noexcept
```

### `TRecords`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Removes one specific queued record while preserving FIFO order of all others.
- **Template parameter `TRecords`:** Task-record collection Type whose intrusive queue links are manipulated.

```cpp
template<class TRecords>
            TaskQueueRemovalResult Remove(
                TRecords& records,
                Index recordIndex
            ) noexcept
```

