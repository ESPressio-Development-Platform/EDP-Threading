# src/threading/detail/ManagedContextWakeSet.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/ManagedContextWakeSet.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `ESPressio_Platform.hpp`
- `TaskRecord.hpp`

## Documented declarations

### `TSignalProvider`

**Classification:** PRIVATE IMPLEMENTATION

Owns one reusable targeted wake primitive per managed execution context.

- **Template parameter `TContextCapacity`:** Number of managed execution contexts in the static topology.
- **Template parameter `TSignalProvider`:** Concrete Platform Signal provider used for targeted wake delivery.

```cpp
template<std::size_t TContextCapacity, class TSignalProvider>
    class ManagedContextWakeSet;
```

### `TSignalProvider`

**Classification:** PRIVATE IMPLEMENTATION

Empty-topology wake-set specialization retaining no Signal provider storage.

- **Template parameter `TSignalProvider`:** Concrete Platform Signal provider Type that would otherwise back wakes.

```cpp
template<class TSignalProvider>
    class ManagedContextWakeSet<0U, TSignalProvider> final
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compact Type used to identify one managed execution context.

```cpp
using ContextIndex = typename SmallestIndex<1U>::Type;
```

### `ContextCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of managed execution contexts represented by this runtime.

```cpp
static constexpr std::size_t ContextCapacity = 0U;
```

### `Validate`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Validates every statically owned targeted wake provider.

```cpp
ManagedContextWakeValidationResult Validate() noexcept
```

### `Wake`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Sends a targeted notification to one managed execution context.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                ContextIndex
            ) noexcept = delete;
```

### `Wait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waits on one managed execution context's reusable targeted wake primitive.

```cpp
ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                ContextIndex,
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept = delete;
```

### `TSignalProvider`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ManagedContextWakeSet`.
- **Template parameter `TSignalProvider`:** Concrete Platform Signal provider Type backing targeted wakes.
- **Template parameter `TContextCapacity`:** Number of managed execution contexts represented by the topology.

```cpp
template<std::size_t TContextCapacity, class TSignalProvider>
    class ManagedContextWakeSet final
```

### `TSignalProvider _signals[TContextCapacity];`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Exactly one reusable latched wake primitive per managed sequential execution context.

```cpp
TSignalProvider _signals[TContextCapacity];
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Smallest dense index Type able to address every managed execution context.

```cpp
using ContextIndex = typename SmallestIndex<TContextCapacity>::Type;
```

### `ContextCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of managed execution contexts represented by this wake set.

```cpp
static constexpr std::size_t ContextCapacity = TContextCapacity;
```

### `Validate`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Validates every statically constructed wake provider before managed execution starts.

```cpp
ManagedContextWakeValidationResult Validate() noexcept
```

### `Wake`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Latches the reusable wake primitive belonging to one managed execution context.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                ContextIndex contextIndex
            ) noexcept
```

### `Wait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Blocks one managed execution context until its targeted signal fires or the wait expires.

```cpp
ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                ContextIndex contextIndex,
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

