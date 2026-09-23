# src/threading/TaskResults.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/TaskResults.hpp)

## Direct includes

- `optional`
- `utility`
- `ThreadingTypes.hpp`

## Documented declarations

### `TTask`

**Classification:** PUBLIC API

Defines the compile-time contract for `TaskDispatchResult`.
- **Template parameter `TTask`:** Concrete Task Type being inspected.

```cpp
template<class TTask>
    class TaskDispatchResult final
```

### `_status`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Operational dispatch status.

```cpp
TaskDispatchStatus _status;
```

### `_task`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Task handle present only after successful dispatch.

```cpp
std::optional<TTask> _task;
```

### `TaskDispatchResult`

**Classification:** PUBLIC API · source access: `public`

Creates a failed dispatch result.

```cpp
explicit TaskDispatchResult(
                TaskDispatchStatus status
            ) noexcept :
                _status(status) {}
```

### `TaskDispatchResult`

**Classification:** PUBLIC API · source access: `public`

Creates a successful dispatch result carrying the new Task handle.

```cpp
explicit TaskDispatchResult(
                TTask task
            ) :
                _status(TaskDispatchStatus::Succeeded),
```

### `Status`

**Classification:** PUBLIC API · source access: `public`

Returns the operational dispatch status.

```cpp
TaskDispatchStatus Status() const noexcept
```

### `IsSucceeded`

**Classification:** PUBLIC API · source access: `public`

Indicates whether dispatch succeeded and a Task handle is available.

```cpp
bool IsSucceeded() const noexcept
```

### `TakeTask`

**Classification:** PUBLIC API · source access: `public`

Moves the successfully dispatched Task handle to the caller.

```cpp
TTask TakeTask()
```

### `TResult`

**Classification:** PUBLIC API

Defines the compile-time contract for `TaskTakeResult`.
- **Template parameter `TResult`:** Result Type produced or carried by the Task.

```cpp
template<class TResult>
    class TaskTakeResult final
```

### `_status`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Operational extraction status.

```cpp
TaskTakeStatus _status;
```

### `_result`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Result payload present only after successful extraction.

```cpp
std::optional<TResult> _result;
```

### `TaskTakeResult`

**Classification:** PUBLIC API · source access: `public`

Creates a failed extraction result.

```cpp
explicit TaskTakeResult(
                TaskTakeStatus status
            ) noexcept :
                _status(status) {}
```

### `TaskTakeResult`

**Classification:** PUBLIC API · source access: `public`

Creates a successful extraction result carrying the Task result.

```cpp
explicit TaskTakeResult(
                TResult result
            ) :
                _status(TaskTakeStatus::Succeeded),
```

### `Status`

**Classification:** PUBLIC API · source access: `public`

Returns the operational extraction status.

```cpp
TaskTakeStatus Status() const noexcept
```

### `IsSucceeded`

**Classification:** PUBLIC API · source access: `public`

Indicates whether extraction succeeded and a result is available.

```cpp
bool IsSucceeded() const noexcept
```

### `TakeResult`

**Classification:** PUBLIC API · source access: `public`

Moves the extracted Task result to the caller.

```cpp
TResult TakeResult()
```

