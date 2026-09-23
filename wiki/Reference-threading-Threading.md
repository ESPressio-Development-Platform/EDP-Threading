# src/threading/Threading.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/Threading.hpp)

## Direct includes

- `Task.hpp`
- `TaskCompletion.hpp`
- `TaskResults.hpp`
- `Thread.hpp`
- `ThreadingComposition.hpp`
- `detail/StaticTopologyOwner.hpp`
- `ThreadingTypes.hpp`

## Documented declarations

### `TTopology`

**Classification:** PUBLIC API

Concrete statically owned runtime for one declared Threading topology.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TBindings`:** Tuple Type containing application Dedicated Thread callable bindings.
- **Template parameter `TSignalProvider`:** Concrete Platform Signal provider Type used for targeted wakes.
- **Template parameter `TExecutionContextProvider`:** Concrete Platform execution-context provider Type used for managed contexts.
- **Template parameter `TSpinLockProvider`:** Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.

```cpp
template<class TTopology, class TBindings, class TSignalProvider, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider>
```

### `StaticThreadingRuntime`

**Classification:** PUBLIC API

Concrete statically owned runtime Type realizing one complete Threading topology.

```cpp
using StaticThreadingRuntime = Detail::StaticTopologyOwner<
        TTopology,
```

