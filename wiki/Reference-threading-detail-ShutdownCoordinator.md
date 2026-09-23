# src/threading/detail/ShutdownCoordinator.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `97ba7417b30b402423f2ac50b444c88f1a4e4066`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/97ba7417b30b402423f2ac50b444c88f1a4e4066/src/threading/detail/ShutdownCoordinator.hpp)

## Direct includes

- `cstddef`
- `tuple`
- `../ThreadingTypes.hpp`
- `TaskFacilityRuntime.hpp`

## Documented declarations

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `CancelTaskResources`.
- **Template parameter `TTuple`:** Tuple Type containing the resources traversed by this helper.
- **Template parameter `TIndex`:** Compile-time tuple/resource index used by the recursive traversal.

```cpp
template<std::size_t TIndex, class TTuple>
            static TaskFacilityShutdownCancellationResult CancelTaskResources(
                TTuple& resources
            ) noexcept
```

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `StopDedicatedThreads`.
- **Template parameter `TTuple`:** Tuple Type containing the resources traversed by this helper.
- **Template parameter `TIndex`:** Compile-time tuple/resource index used by the recursive traversal.

```cpp
template<std::size_t TIndex, class TTuple>
            static void StopDedicatedThreads(
                TTuple& resources
            ) noexcept
```

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `TaskResourcesQuiescent`.
- **Template parameter `TTuple`:** Tuple Type containing the resources traversed by this helper.
- **Template parameter `TIndex`:** Compile-time tuple/resource index used by the recursive traversal.

```cpp
template<std::size_t TIndex, class TTuple>
            static bool TaskResourcesQuiescent(
                TTuple& resources
            ) noexcept
```

### `TTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `DedicatedThreadsQuiescent`.
- **Template parameter `TTuple`:** Tuple Type containing the resources traversed by this helper.
- **Template parameter `TIndex`:** Compile-time tuple/resource index used by the recursive traversal.

```cpp
template<std::size_t TIndex, class TTuple>
            static bool DedicatedThreadsQuiescent(
                TTuple& resources
            ) noexcept
```

### `TLifecycle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Initiates terminal semantic shutdown without waiting for arbitrary user callables.
- **Template parameter `TLifecycle`:** Authoritative Threading lifecycle Type coordinated during shutdown.
- **Template parameter `TTaskResourceTuple`:** Tuple Type containing Task-execution resources coordinated during shutdown.
- **Template parameter `TDedicatedThreadTuple`:** Tuple Type containing Dedicated Thread resources coordinated during shutdown.

```cpp
template<class TLifecycle, class TTaskResourceTuple, class TDedicatedThreadTuple>
            static ThreadingShutdownResult Begin(
                TLifecycle& lifecycle,
                TTaskResourceTuple& taskResources,
                TDedicatedThreadTuple& dedicatedThreads
            ) noexcept
```

### `TTaskResourceTuple`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether all semantic execution has cooperatively ceased.
- **Template parameter `TTaskResourceTuple`:** Tuple Type containing Task-execution resources coordinated during shutdown.
- **Template parameter `TDedicatedThreadTuple`:** Tuple Type containing Dedicated Thread resources coordinated during shutdown.

```cpp
template<class TTaskResourceTuple, class TDedicatedThreadTuple>
            static bool IsExecutionQuiescent(
                TTaskResourceTuple& taskResources,
                TDedicatedThreadTuple& dedicatedThreads
            ) noexcept
```

