# src/threading/TaskCompletion.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/TaskCompletion.hpp)

## Direct includes

- `optional`
- `utility`

## Documented declarations

### `TResult`

**Classification:** PUBLIC API

Defines the compile-time contract for `TaskCompletion`.
- **Template parameter `TResult`:** Result Type produced or carried by the Task.

```cpp
template<class TResult>
    class TaskCompletion final
```

### `_result`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Optional successfully completed result.

```cpp
std::optional<TResult> _result;
```

### `TaskCompletion`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates an acknowledged-cancellation completion.

```cpp
TaskCompletion() = default;
```

### `TaskCompletion`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates a successfully completed result.

```cpp
explicit TaskCompletion(
                TResult result
            ) :
                _result(
                    std::move(
                        result
                    )
                ) {}
```

### `Cancelled`

**Classification:** PUBLIC API · source access: `public`

Creates an acknowledged-cancellation completion.

```cpp
static TaskCompletion Cancelled()
```

### `Completed`

**Classification:** PUBLIC API · source access: `public`

Creates a normal completion carrying the callable result.

```cpp
static TaskCompletion Completed(
                TResult result
            )
```

### `IsCancelled`

**Classification:** PUBLIC API · source access: `public`

Indicates whether callable execution acknowledged cancellation.

```cpp
bool IsCancelled() const noexcept
```

### `TakeResult`

**Classification:** PUBLIC API · source access: `public`

Moves the successfully completed result from this completion object.

```cpp
TResult TakeResult()
```

### `_cancelled`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether callable execution acknowledged cancellation.

```cpp
bool _cancelled;
```

### `TaskCompletion`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates a void completion with the supplied cancellation state.

```cpp
explicit TaskCompletion(
                bool cancelled
            ) noexcept :
                _cancelled(cancelled) {}
```

### `Cancelled`

**Classification:** PUBLIC API · source access: `public`

Creates an acknowledged-cancellation completion.

```cpp
static TaskCompletion Cancelled() noexcept
```

### `Completed`

**Classification:** PUBLIC API · source access: `public`

Creates a normal void completion.

```cpp
static TaskCompletion Completed() noexcept
```

### `IsCancelled`

**Classification:** PUBLIC API · source access: `public`

Indicates whether callable execution acknowledged cancellation.

```cpp
bool IsCancelled() const noexcept
```

