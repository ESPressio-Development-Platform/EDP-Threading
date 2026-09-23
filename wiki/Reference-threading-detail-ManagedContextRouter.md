# src/threading/detail/ManagedContextRouter.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/ManagedContextRouter.hpp)

## Direct includes

- `cstddef`
- `optional`
- `ManagedContextWakeSet.hpp`
- `WaitRegistration.hpp`

## Documented declarations

### `TSignalProvider`

**Classification:** PRIVATE IMPLEMENTATION

Routes targeted wakes and structural context inspection for one static Threading topology.

- **Template parameter `TContextCapacity`:** Number of managed execution contexts in the topology.
- **Template parameter `TSignalProvider`:** Concrete Platform Signal provider used by the wake set.

```cpp
template<std::size_t TContextCapacity, class TSignalProvider>
    class ManagedContextRouter;
```

### `TSignalProvider`

**Classification:** PRIVATE IMPLEMENTATION

Empty-topology router specialization retaining no structural routing state.

- **Template parameter `TSignalProvider`:** Concrete Platform Signal provider Type selected by Bootstrap.

```cpp
template<class TSignalProvider>
    class ManagedContextRouter<0U, TSignalProvider> final
```

### `WakeSet`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Targeted-wake set Type used by this router.

```cpp
using WakeSet = ManagedContextWakeSet<
                0U,
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

### `ManagedContextRouter`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs a router bound only to the topology-owned wake set.

```cpp
explicit ManagedContextRouter(
                WakeSet&
            ) noexcept {}
```

### `BindTopology`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Rejects structural binding for an empty topology because no managed context can be resolved.
- **Template parameter `TArguments`:** Constructor argument Types that would otherwise describe a structural resolver binding.

```cpp
template<class... TArguments>
            void BindTopology(
                TArguments&&...
            ) noexcept = delete;
```

### `IsTopologyBound`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether structural current-context and interruption resolution are bound.

```cpp
bool IsTopologyBound() const noexcept
```

### `CurrentContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the dense index of the currently executing managed context when one exists.

```cpp
std::optional<ContextIndex> CurrentContextIndex() const noexcept
```

### `IsInterrupted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the addressed managed context currently carries an authoritative interruption request.

```cpp
bool IsInterrupted(
                ContextIndex
            ) const noexcept
```

### `Wake`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Sends a targeted wake to one managed execution context.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                ContextIndex
            ) noexcept = delete;
```

### `Wait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waits on one managed context's targeted wake primitive using the supplied Platform timeout.

```cpp
ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                ContextIndex,
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept = delete;
```

### `TSignalProvider`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ManagedContextRouter`.
- **Template parameter `TSignalProvider`:** Concrete Platform Signal provider Type backing targeted wakes.
- **Template parameter `TContextCapacity`:** Number of managed execution contexts represented by the topology.

```cpp
template<std::size_t TContextCapacity, class TSignalProvider>
    class ManagedContextRouter final
```

### `WakeSetStorage`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Internal targeted-wake set Type used by this router.

```cpp
using WakeSetStorage = ManagedContextWakeSet<
                TContextCapacity,
```

### `ContextIndexStorage`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Internal compact Type used to identify one managed execution context.

```cpp
using ContextIndexStorage =
                typename ExecutionContextIndexTraits<TContextCapacity>::Type;
```

### `_wakeSet`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Topology-owned dense targeted wake mechanisms.

```cpp
WakeSetStorage* _wakeSet;
```

### `_topologyContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning topology realization used for structural current-context/interruption scans.

```cpp
const void* _topologyContext;
```

### `noexcept`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Structural resolver for the currently executing managed context.

```cpp
std::optional<ContextIndexStorage> (*_currentContextIndex)(const void*) noexcept;
```

### `bool`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Structural authoritative interruption resolver for one managed context.

```cpp
bool (*_isInterrupted)(
                const void*,
                ContextIndexStorage
            ) noexcept;
```

### `WakeSet`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Targeted-wake set Type used by this router.

```cpp
using WakeSet = WakeSetStorage;
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compact Type used to identify one managed execution context.

```cpp
using ContextIndex = ContextIndexStorage;
```

### `ContextCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Number of managed execution contexts represented by this runtime.

```cpp
static constexpr std::size_t ContextCapacity = TContextCapacity;
```

### `ManagedContextRouter`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs a router bound only to the topology-owned wake set.

```cpp
explicit ManagedContextRouter(
                WakeSet& wakeSet
            ) noexcept :
                _wakeSet(&wakeSet),
```

### `ManagedContextRouter`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Constructs a router with both wake storage and an already completed structural resolver.

```cpp
ManagedContextRouter(
                WakeSet& wakeSet,
                const void* topologyContext,
                std::optional<ContextIndex> (*currentContextIndex)(const void*) noexcept,
                bool (*isInterrupted)(const void*, ContextIndex) noexcept
            ) noexcept :
                _wakeSet(&wakeSet),
```

### `BindTopology`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Binds the completed structural resolver after topology resource construction.

```cpp
void BindTopology(
                const void* topologyContext,
                std::optional<ContextIndex> (*currentContextIndex)(const void*) noexcept,
                bool (*isInterrupted)(const void*, ContextIndex) noexcept
            ) noexcept
```

### `IsTopologyBound`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether structural current-context and interruption resolution are bound.

```cpp
bool IsTopologyBound() const noexcept
```

### `CurrentContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the dense index of the currently executing managed context when one exists.

```cpp
std::optional<ContextIndex> CurrentContextIndex() const noexcept
```

### `IsInterrupted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the addressed managed context currently carries an authoritative interruption request.

```cpp
bool IsInterrupted(
                ContextIndex contextIndex
            ) const noexcept
```

### `Wake`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Sends a targeted wake to one managed execution context.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                ContextIndex contextIndex
            ) noexcept
```

### `Wait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waits on one managed context's targeted wake primitive using the supplied Platform timeout.

```cpp
ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                ContextIndex contextIndex,
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

