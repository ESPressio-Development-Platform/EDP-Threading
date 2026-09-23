# src/threading/detail/CallableTraits.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `50dbad50650df9fc38dd17cd3b17a60a92a5d802`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/50dbad50650df9fc38dd17cd3b17a60a92a5d802/src/threading/detail/CallableTraits.hpp)

## Direct includes

- `type_traits`
- `../TaskCompletion.hpp`
- `../ThreadingTypes.hpp`

## Documented declarations

### `TCompletion`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskCompletionResult`.
- **Template parameter `TCompletion`:** Task-completion Type being classified.

```cpp
template<class TCompletion>
    struct TaskCompletionResult;
```

### `TResult`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskCompletionResult`.
- **Template parameter `TResult`:** Result Type produced or carried by the Task.

```cpp
template<class TResult>
    struct TaskCompletionResult<TaskCompletion<TResult>>
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Result Type represented by this TaskCompletion specialization.

```cpp
using Type = TResult;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `CallableResultSelector`.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.
- **Template parameter `TAcceptsContext`:** Whether the callable accepts a TaskContext parameter.

```cpp
template<class TCallable, bool TAcceptsContext>
    struct CallableResultSelector;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `CallableResultSelector`.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.

```cpp
template<class TCallable>
    struct CallableResultSelector<TCallable, false>
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Result Type produced by invoking the callable without a TaskContext.

```cpp
using Type = std::invoke_result_t<TCallable&>;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `CallableResultSelector`.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.

```cpp
template<class TCallable>
    struct CallableResultSelector<TCallable, true>
```

### `Completion`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

TaskCompletion wrapper Type produced by invoking the callable with a TaskContext.

```cpp
using Completion = std::invoke_result_t<TCallable&, TaskContext&>;
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Result Type carried by the TaskCompletion wrapper.

```cpp
using Type = typename TaskCompletionResult<Completion>::Type;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `CallableResult`.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.

```cpp
template<class TCallable>
    struct CallableResult final
```

### `AcceptsContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Whether the callable accepts a TaskContext reference.

```cpp
static constexpr bool AcceptsContext =
            std::is_invocable_v<TCallable&, TaskContext&>;
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Result Type selected from the callable's context-aware or context-free invocation form.

```cpp
using Type = typename CallableResultSelector<
            TCallable,
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `CallableResultT`.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.

```cpp
template<class TCallable>
```

### `CallableResultT`

**Classification:** PRIVATE IMPLEMENTATION

Convenience alias exposing the result Type produced by a supported Task callable.

```cpp
using CallableResultT = typename CallableResult<TCallable>::Type;
```

