# src/threading/Thread.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/Thread.hpp)

## Direct includes

- `utility`
- `ThreadingTypes.hpp`

## Documented declarations

### `ThreadHandleOperations`

**Classification:** PUBLIC API

Type-erased control operations bound to one topology-owned Dedicated Thread resource.

```cpp
struct ThreadHandleOperations final
```

### `ThreadState`

**Classification:** PUBLIC API · source access: `public`

Reads the current public Dedicated Thread state.

```cpp
ThreadState (*State)(const void*) noexcept;
```

### `ThreadStartResult`

**Classification:** PUBLIC API · source access: `public`

Starts a new semantic activation when no activation currently exists.

```cpp
ThreadStartResult (*Start)(void*) noexcept;
```

### `ThreadStopRequestResult`

**Classification:** PUBLIC API · source access: `public`

Requests cooperative stop of the current activation.

```cpp
ThreadStopRequestResult (*RequestStop)(void*) noexcept;
```

### `ThreadJoinResult`

**Classification:** PUBLIC API · source access: `public`

Joins the activation captured when this operation begins.

```cpp
ThreadJoinResult (*Join)(void*);
```

### `ThreadJoinResult`

**Classification:** PUBLIC API · source access: `public`

Joins the captured activation using one relative physical-time budget.

```cpp
ThreadJoinResult (*JoinFor)(
                void*,
                Duration
            );
```

### `ThreadJoinResult`

**Classification:** PUBLIC API · source access: `public`

Joins the captured activation until one canonical monotonic deadline.

```cpp
ThreadJoinResult (*JoinUntil)(
                void*,
                MonotonicTimestamp
            );
```

### `TThreadIdentity`

**Classification:** PUBLIC API

Defines the compile-time contract for `Thread`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.

```cpp
template<class TThreadIdentity>
    class Thread final
```

### `_resource`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Opaque topology-owned Dedicated Thread resource.

```cpp
void* _resource;
```

### `_operations`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Static operations for the concrete Dedicated Thread resource.

```cpp
const Detail::ThreadHandleOperations* _operations;
```

### `Thread`

**Classification:** PUBLIC API · source access: `public`

Creates a control handle for one topology-owned Dedicated Thread identity.

```cpp
Thread(
                void* resource,
                const Detail::ThreadHandleOperations& operations
            ) noexcept :
                _resource(resource),
```

### `Thread`

**Classification:** PUBLIC API · source access: `public`

Prevents copying of the control handle.

```cpp
Thread(const Thread&) = delete;
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Prevents copy assignment of the control handle.

```cpp
Thread& operator =(const Thread&) = delete;
```

### `Thread`

**Classification:** PUBLIC API · source access: `public`

Transfers this non-owning control handle.

```cpp
Thread(
                Thread&& other
            ) noexcept :
                _resource(other._resource),
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Transfers this non-owning control handle.

```cpp
Thread& operator =(
                Thread&& other
            ) noexcept
```

### `Thread`

**Classification:** PUBLIC API · source access: `public`

Destroys only this non-owning control handle.

```cpp
~Thread() = default;
```

### `IsValid`

**Classification:** PUBLIC API · source access: `public`

Indicates whether this handle remains bound to its topology-owned resource.

```cpp
bool IsValid() const noexcept
```

### `State`

**Classification:** PUBLIC API · source access: `public`

Returns the current public Dedicated Thread lifecycle state.

```cpp
ThreadState State() const noexcept
```

### `Start`

**Classification:** PUBLIC API · source access: `public`

Starts a semantic activation when the Dedicated Thread is not already running.

```cpp
ThreadStartResult Start() noexcept
```

### `RequestStop`

**Classification:** PUBLIC API · source access: `public`

Requests cooperative stop of the current semantic activation.

```cpp
ThreadStopRequestResult RequestStop() noexcept
```

### `Join`

**Classification:** PUBLIC API · source access: `public`

Waits indefinitely for the activation captured at Join entry to stop.

```cpp
ThreadJoinResult Join()
```

### `JoinFor`

**Classification:** PUBLIC API · source access: `public`

Waits for the captured activation using one relative physical-time budget.

```cpp
ThreadJoinResult JoinFor(
                Duration duration
            )
```

### `JoinUntil`

**Classification:** PUBLIC API · source access: `public`

Waits for the captured activation until one canonical monotonic deadline.

```cpp
ThreadJoinResult JoinUntil(
                MonotonicTimestamp deadline
            )
```

