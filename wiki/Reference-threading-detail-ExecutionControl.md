# src/threading/detail/ExecutionControl.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `50dbad50650df9fc38dd17cd3b17a60a92a5d802`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/50dbad50650df9fc38dd17cd3b17a60a92a5d802/src/threading/detail/ExecutionControl.hpp)

## Direct includes

- `../ThreadingTypes.hpp`
- `TaskFacilityRuntime.hpp`

## Documented declarations

### `TExecutionContextProvider`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ExecutionControl`.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed execution.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.

```cpp
template<class TExecutionContextProvider, class TManagedContextRouter>
    class ExecutionControl final
```

### `SleepWithBudget`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Sleeps the current managed context against one canonical non-restarting monotonic budget.

```cpp
static SleepResult SleepWithBudget(
                TManagedContextRouter& router,
                const MonotonicWaitBudget& budget
            )
```

### `Yield`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Gives the concrete Platform scheduler an opportunity to run another runnable context.

```cpp
static void Yield() noexcept
```

### `SleepFor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Suspends the current managed context for one relative physical-time budget.

```cpp
static SleepResult SleepFor(
                TManagedContextRouter& router,
                Duration duration
            )
```

### `SleepUntil`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Suspends the current managed context until one canonical monotonic deadline.

```cpp
static SleepResult SleepUntil(
                TManagedContextRouter& router,
                MonotonicTimestamp deadline
            )
```

