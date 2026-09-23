# src/threading/ThreadingTypes.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/ThreadingTypes.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `ESPressio_Clock.hpp`
- `ESPressio_Platform.hpp`

## Documented declarations

### `ThreadPriority`

**Classification:** PUBLIC API

Logical execution-priority vocabulary shared with the Platform execution contract.

```cpp
using ThreadPriority = ESPressio::Platform::Execution::ExecutionPriority;
```

### `ProcessorAffinity`

**Classification:** PUBLIC API

Logical processor-affinity vocabulary shared with the Platform execution contract.

```cpp
using ProcessorAffinity = ESPressio::Platform::Execution::ProcessorAffinity;
```

### `Duration`

**Classification:** PUBLIC API

Canonical physical-duration Type supplied by EDP-Clock.

```cpp
using Duration = ESPressio::Clock::Duration;
```

### `MonotonicTimestamp`

**Classification:** PUBLIC API

Canonical monotonic-coordinate Type supplied by EDP-Clock.

```cpp
using MonotonicTimestamp = ESPressio::Clock::MonotonicTimestamp;
```

### `_context`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Opaque active Task record supplied by the facility.

```cpp
const void* _context;
```

### `bool`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Predicate used to inspect cancellation without duplicating control state.

```cpp
bool (*_isCancellationRequested)(const void*) noexcept;
```

### `TaskContext`

**Classification:** PUBLIC API · source access: `public`

Creates a lightweight view over one active Task's authoritative cancellation state.

```cpp
TaskContext(
                const void* context,
                bool (*isCancellationRequested)(const void*) noexcept
            ) noexcept :
                _context(context),
```

### `IsCancellationRequested`

**Classification:** PUBLIC API · source access: `public`

Indicates whether cooperative Task cancellation has been requested.

```cpp
bool IsCancellationRequested() const noexcept
```

### `_context`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Opaque Dedicated Thread resource supplied by the runtime.

```cpp
const void* _context;
```

### `bool`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Predicate used to inspect stop state without duplicating control state.

```cpp
bool (*_isStopRequested)(const void*) noexcept;
```

### `ThreadContext`

**Classification:** PUBLIC API · source access: `public`

Creates a lightweight view over one activation's authoritative stop state.

```cpp
ThreadContext(
                const void* context,
                bool (*isStopRequested)(const void*) noexcept
            ) noexcept :
                _context(context),
```

### `IsStopRequested`

**Classification:** PUBLIC API · source access: `public`

Indicates whether cooperative Dedicated Thread stop has been requested.

```cpp
bool IsStopRequested() const noexcept
```

