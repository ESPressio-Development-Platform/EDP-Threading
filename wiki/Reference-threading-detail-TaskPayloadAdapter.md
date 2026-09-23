# src/threading/detail/TaskPayloadAdapter.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/TaskPayloadAdapter.hpp)

## Direct includes

- `new`
- `type_traits`
- `utility`
- `../TaskCompletion.hpp`
- `../ThreadingTypes.hpp`
- `TaskRecord.hpp`

## Documented declarations

### `TRecord`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskPayloadAdapter`.
- **Template parameter `TRecord`:** Task-record Type whose callable/result payload is adapted.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.
- **Template parameter `TResult`:** Result Type stored in the Task record.

```cpp
template<class TRecord, class TCallable, class TResult>
    struct TaskPayloadAdapter final
```

### `Invoke`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Invokes the stored callable and establishes any result payload without publishing terminal lifecycle state.

```cpp
static TaskInvocationOutcome Invoke(
            TRecord& record,
            TaskContext& context
        )
```

### `DestroyCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys a callable payload which has not been consumed by execution.

```cpp
static void DestroyCallable(
            TRecord& record
        ) noexcept
```

### `DestroyResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys a completed result payload which will not be consumed by an owner.

```cpp
static void DestroyResult(
            TRecord& record
        ) noexcept
```

### `MoveResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Moves the completed result into caller-provided typed storage and destroys the in-record result.

```cpp
static void MoveResult(
            TRecord& record,
            void* destination
        )
```

### `inline static const TaskPayloadOperations<TRecord> Operations`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Shared immutable operation table for this callable/result pairing.

```cpp
inline static const TaskPayloadOperations<TRecord> Operations
```

### `TRecord`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskPayloadAdapter`.
- **Template parameter `TRecord`:** Task-record Type whose callable/result payload is adapted.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.

```cpp
template<class TRecord, class TCallable>
    struct TaskPayloadAdapter<TRecord, TCallable, void> final
```

### `Invoke`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Invokes the stored void callable without publishing terminal lifecycle state.

```cpp
static TaskInvocationOutcome Invoke(
            TRecord& record,
            TaskContext& context
        )
```

### `DestroyCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys a callable payload which has not been consumed by execution.

```cpp
static void DestroyCallable(
            TRecord& record
        ) noexcept
```

### `DestroyResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Void Tasks have no result payload to destroy.

```cpp
static void DestroyResult(
            TRecord&
        ) noexcept {}
```

### `MoveResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Void Tasks have no result payload to move.

```cpp
static void MoveResult(
            TRecord&,
            void*
        ) {}
```

### `inline static const TaskPayloadOperations<TRecord> Operations`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Shared immutable operation table for this callable/void pairing.

```cpp
inline static const TaskPayloadOperations<TRecord> Operations
```

