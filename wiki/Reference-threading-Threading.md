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




## Current optional binding integration

The original reference inventory above remains the unselected baseline unless amended here. The current additive contract is [optional context binding](Context-Binding), implemented on `feature/profiling-v1`. Existing six-argument runtimes retain the original router and default behavior. Selected registration uses exact Platform contracts and one canonical Clock startup budget. It introduces no SDK/Profiling dependency, registry, scheduler, allocator, probe scan or new monotonic source. See [normative owner contract](../docs/CONTEXT-BINDING.MD).

`StaticThreadingRuntime` now has a seventh `TContextBindingPolicy = Detail::UnselectedContextBinding` parameter, forwarded to `StaticTopologyOwner`. This is a PUBLIC optional configuration Type. The selected policy is explicitly included through `ESPressio_Threading_ContextBinding.hpp`; ordinary consumers need no binding header. `StartFor`/`StartInOrderFor` and `FinalizeShutdownFor` require caller-supplied finite durations.
