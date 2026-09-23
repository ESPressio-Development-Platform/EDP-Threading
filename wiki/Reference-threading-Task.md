# src/threading/Task.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/Task.hpp)

## Direct includes

- `cstddef`
- `new`
- `utility`
- `TaskResults.hpp`
- `ThreadingTypes.hpp`

## Documented declarations

### `TaskHandleOperations`

**Classification:** PUBLIC API

Type-erased operations bound to one concrete Task facility without dynamic allocation.

```cpp
struct TaskHandleOperations final
```

### `TaskState`

**Classification:** PUBLIC API · source access: `public`

Reads the stable public lifecycle state for one Task record.

```cpp
TaskState (*State)(const void*, std::uint32_t, bool) noexcept;
```

### `TaskWaitResult`

**Classification:** PUBLIC API · source access: `public`

Waits indefinitely for one Task record to become terminal.

```cpp
TaskWaitResult (*Wait)(void*, std::uint32_t, bool);
```

### `TaskWaitResult`

**Classification:** PUBLIC API · source access: `public`

Waits for one Task record to become terminal within a relative duration.

```cpp
TaskWaitResult (*WaitFor)(
                void*,
                std::uint32_t,
                bool,
                Duration
            );
```

### `TaskWaitResult`

**Classification:** PUBLIC API · source access: `public`

Waits for one Task record to become terminal by a canonical monotonic deadline.

```cpp
TaskWaitResult (*WaitUntil)(
                void*,
                std::uint32_t,
                bool,
                MonotonicTimestamp
            );
```

### `TaskCancelResult`

**Classification:** PUBLIC API · source access: `public`

Requests cooperative cancellation of one Task record.

```cpp
TaskCancelResult (*Cancel)(void*, std::uint32_t, bool) noexcept;
```

### `void`

**Classification:** PUBLIC API · source access: `public`

Releases the sole public Task ownership interest.

```cpp
void (*Release)(void*, std::uint32_t, bool) noexcept;
```

### `TaskTakeStatus`

**Classification:** PUBLIC API · source access: `public`

Moves a completed result into caller-provided typed storage.

```cpp
TaskTakeStatus (*TakeResult)(
                void*,
                std::uint32_t,
                bool,
                void*
            );
```

### `TResult`

**Classification:** PUBLIC API

Defines the compile-time contract for `Task`.
- **Template parameter `TResult`:** Result Type produced or carried by the Task.

```cpp
template<class TResult>
    class Task final
```

### `_owner`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Opaque address of the concrete facility owning this Task.

```cpp
void* _owner;
```

### `_recordIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Stable record slot index within the owning bounded facility.

```cpp
std::uint32_t _recordIndex;
```

### `_phase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

One-bit incarnation Phase captured when this Task was admitted.

```cpp
bool _phase;
```

### `_operations`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Static operations for the concrete facility owning the record.

```cpp
const Detail::TaskHandleOperations* _operations;
```

### `Release`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Releases this handle's public ownership interest when one remains.

```cpp
void Release() noexcept
```

### `Task`

**Classification:** PUBLIC API · source access: `public`

Creates a valid Task handle for one successfully admitted record.

```cpp
Task(
                void* owner,
                std::uint32_t recordIndex,
                bool phase,
                const Detail::TaskHandleOperations& operations
            ) noexcept :
                _owner(owner),
```

### `Task`

**Classification:** PUBLIC API · source access: `public`

Prevents shared public ownership through copying.

```cpp
Task(const Task&) = delete;
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Prevents shared public ownership through copy assignment.

```cpp
Task& operator =(const Task&) = delete;
```

### `Task`

**Classification:** PUBLIC API · source access: `public`

Transfers the sole public Task ownership interest.

```cpp
Task(
                Task&& other
            ) noexcept :
                _owner(other._owner),
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Transfers the sole public Task ownership interest after releasing the current one.

```cpp
Task& operator =(
                Task&& other
            ) noexcept
```

### `Task`

**Classification:** PUBLIC API · source access: `public`

Releases public ownership without cancelling or waiting.

```cpp
~Task()
```

### `IsValid`

**Classification:** PUBLIC API · source access: `public`

Indicates whether this handle still owns a valid Task observation/result interest.

```cpp
bool IsValid() const noexcept
```

### `State`

**Classification:** PUBLIC API · source access: `public`

Returns the stable public Task lifecycle state.

```cpp
TaskState State() const noexcept
```

### `IsCompleted`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the Task completed normally.

```cpp
bool IsCompleted() const noexcept
```

### `IsCancelled`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the Task completed through cooperative cancellation.

```cpp
bool IsCancelled() const noexcept
```

### `IsFinished`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the Task has reached either terminal lifecycle state.

```cpp
bool IsFinished() const noexcept
```

### `Wait`

**Classification:** PUBLIC API · source access: `public`

Waits indefinitely until this Task becomes terminal or the waiting context is interrupted.

```cpp
TaskWaitResult Wait()
```

### `WaitFor`

**Classification:** PUBLIC API · source access: `public`

Waits for this Task using one relative physical-time budget.

```cpp
TaskWaitResult WaitFor(
                Duration duration
            )
```

### `WaitUntil`

**Classification:** PUBLIC API · source access: `public`

Waits for this Task until one canonical EDP monotonic deadline.

```cpp
TaskWaitResult WaitUntil(
                MonotonicTimestamp deadline
            )
```

### `Cancel`

**Classification:** PUBLIC API · source access: `public`

Requests cooperative cancellation of this Task.

```cpp
TaskCancelResult Cancel() noexcept
```

### `TakeResult`

**Classification:** PUBLIC API · source access: `public`

Moves the completed result from the Task record and consumes this handle on success.

```cpp
TaskTakeResult<TResult> TakeResult()
```

### `_owner`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Opaque address of the concrete facility owning this Task.

```cpp
void* _owner;
```

### `_recordIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Stable record slot index within the owning bounded facility.

```cpp
std::uint32_t _recordIndex;
```

### `_phase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

One-bit incarnation Phase captured when this Task was admitted.

```cpp
bool _phase;
```

### `_operations`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Static operations for the concrete facility owning the record.

```cpp
const Detail::TaskHandleOperations* _operations;
```

### `Release`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Releases this handle's public ownership interest when one remains.

```cpp
void Release() noexcept
```

### `Task`

**Classification:** PUBLIC API · source access: `public`

Creates a valid void Task handle for one successfully admitted record.

```cpp
Task(
                void* owner,
                std::uint32_t recordIndex,
                bool phase,
                const Detail::TaskHandleOperations& operations
            ) noexcept :
                _owner(owner),
```

### `Task`

**Classification:** PUBLIC API · source access: `public`

Prevents shared public ownership through copying.

```cpp
Task(const Task&) = delete;
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Prevents shared public ownership through copy assignment.

```cpp
Task& operator =(const Task&) = delete;
```

### `Task`

**Classification:** PUBLIC API · source access: `public`

Transfers the sole public Task ownership interest.

```cpp
Task(
                Task&& other
            ) noexcept :
                _owner(other._owner),
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Transfers the sole public Task ownership interest after releasing the current one.

```cpp
Task& operator =(
                Task&& other
            ) noexcept
```

### `Task`

**Classification:** PUBLIC API · source access: `public`

Releases public ownership without cancelling or waiting.

```cpp
~Task()
```

### `IsValid`

**Classification:** PUBLIC API · source access: `public`

Indicates whether this handle still owns a valid Task observation interest.

```cpp
bool IsValid() const noexcept
```

### `State`

**Classification:** PUBLIC API · source access: `public`

Returns the stable public Task lifecycle state.

```cpp
TaskState State() const noexcept
```

### `IsCompleted`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the Task completed normally.

```cpp
bool IsCompleted() const noexcept
```

### `IsCancelled`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the Task completed through cooperative cancellation.

```cpp
bool IsCancelled() const noexcept
```

### `IsFinished`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the Task has reached either terminal lifecycle state.

```cpp
bool IsFinished() const noexcept
```

### `Wait`

**Classification:** PUBLIC API · source access: `public`

Waits indefinitely until this Task becomes terminal or the waiting context is interrupted.

```cpp
TaskWaitResult Wait()
```

### `WaitFor`

**Classification:** PUBLIC API · source access: `public`

Waits for this Task using one relative physical-time budget.

```cpp
TaskWaitResult WaitFor(
                Duration duration
            )
```

### `WaitUntil`

**Classification:** PUBLIC API · source access: `public`

Waits for this Task until one canonical EDP monotonic deadline.

```cpp
TaskWaitResult WaitUntil(
                MonotonicTimestamp deadline
            )
```

### `Cancel`

**Classification:** PUBLIC API · source access: `public`

Requests cooperative cancellation of this Task.

```cpp
TaskCancelResult Cancel() noexcept
```

