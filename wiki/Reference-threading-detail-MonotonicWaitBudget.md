# src/threading/detail/MonotonicWaitBudget.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/MonotonicWaitBudget.hpp)

## Direct includes

- `cstdint`
- `ESPressio_Clock.hpp`
- `ESPressio_Platform.hpp`

## Documented declarations

### `_mode`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Selected wait-budget mode.

```cpp
MonotonicWaitMode _mode;
```

### `_start`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Canonical monotonic coordinate captured when a relative budget begins.

```cpp
ESPressio::Clock::MonotonicTimestamp _start;
```

### `_duration`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Relative physical-time budget.

```cpp
ESPressio::Clock::Duration _duration;
```

### `_deadline`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Canonical absolute deadline for deadline-based waits.

```cpp
ESPressio::Clock::MonotonicTimestamp _deadline;
```

### `MonotonicWaitBudget`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates a normalized monotonic wait budget from its complete internal representation.

```cpp
constexpr MonotonicWaitBudget(
                MonotonicWaitMode mode,
                ESPressio::Clock::MonotonicTimestamp start,
                ESPressio::Clock::Duration duration,
                ESPressio::Clock::MonotonicTimestamp deadline
            ) noexcept :
                _mode(mode),
```

### `Forever`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates an indefinite wait budget.

```cpp
static MonotonicWaitBudget Forever() noexcept
```

### `For`

**Classification:** PRIVATE IMPLEMENTATION

Creates one relative wait budget beginning at the current canonical monotonic coordinate.

```cpp
static MonotonicWaitBudget For(
                ESPressio::Clock::Duration duration
            ) noexcept
```

### `Until`

**Classification:** PRIVATE IMPLEMENTATION

Creates one absolute canonical monotonic deadline budget.

```cpp
static MonotonicWaitBudget Until(
                ESPressio::Clock::MonotonicTimestamp deadline
            ) noexcept
```

### `Mode`

**Classification:** PRIVATE IMPLEMENTATION

Returns the selected wait-budget mode.

```cpp
MonotonicWaitMode Mode() const noexcept
```

### `Remaining`

**Classification:** PRIVATE IMPLEMENTATION

Computes the remaining relative Platform wait without restarting the original budget.

```cpp
ESPressio::Platform::Synchronization::WaitTimeout Remaining() const noexcept
```

