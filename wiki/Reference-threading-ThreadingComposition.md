# src/threading/ThreadingComposition.hpp

**Primary classification:** PUBLIC COMPOSITION / TOPOLOGY API

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/ThreadingComposition.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `type_traits`
- `tuple`
- `utility`
- `ESPressio_System.hpp`
- `ThreadingTypes.hpp`

## Documented declarations

### `TPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `TaskPool`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.

```cpp
template<class TPoolIdentity>
    struct TaskPool final {};
```

### `TaskRecordCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `TaskRecordCapacity`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
    struct TaskRecordCapacity final
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCapacity;
```

### `CallableCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `CallableCapacity`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
    struct CallableCapacity final
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCapacity;
```

### `ResultCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResultCapacity`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
    struct ResultCapacity final
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCapacity;
```

### `StackCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `StackCapacity`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
    struct StackCapacity final
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCapacity;
```

### `Priority`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `Priority`.
- **Template parameter `TPriority`:** Compile-time logical execution priority.

```cpp
template<ThreadPriority TPriority>
    struct Priority final
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ThreadPriority Value = TPriority;
```

### `Affinity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `Affinity`.
- **Template parameter `TProcessorIndex`:** Compile-time logical processor index.

```cpp
template<std::uint32_t TProcessorIndex>
    struct Affinity final
```

### `Specific`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ProcessorAffinity Value = ProcessorAffinity::Specific(TProcessorIndex);
```

### `Any`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ProcessorAffinity Value = ProcessorAffinity::Any();
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsStackCapacityProperty`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct IsStackCapacityProperty
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `IsStackCapacityProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsStackCapacityProperty`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
        struct IsStackCapacityProperty<StackCapacity<TCapacity>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsPriorityProperty`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct IsPriorityProperty
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `IsPriorityProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsPriorityProperty`.
- **Template parameter `TPriority`:** Compile-time logical execution priority.

```cpp
template<ThreadPriority TPriority>
        struct IsPriorityProperty<Priority<TPriority>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsAffinityProperty`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct IsAffinityProperty
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `IsAffinityProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsAffinityProperty`.
- **Template parameter `TProcessorIndex`:** Compile-time logical processor index.

```cpp
template<std::uint32_t TProcessorIndex>
        struct IsAffinityProperty<Affinity<TProcessorIndex>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `IsAffinityProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Classifies the AnyAffinity property as a valid execution-resource affinity declaration.

```cpp
template<>
        struct IsAffinityProperty<AnyAffinity>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsExecutionResourceProperty`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct IsExecutionResourceProperty
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                IsStackCapacityProperty<TProperty>::Value ||
                IsPriorityProperty<TProperty>::Value ||
                IsAffinityProperty<TProperty>::Value;
```

### `ValidExecutionResourceProperties`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class... TProperties>
        struct ValidExecutionResourceProperties
```

### `EveryPropertyRecognized`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Whether every supplied execution-resource property is recognized.

```cpp
static constexpr bool EveryPropertyRecognized =
                (
                    IsExecutionResourceProperty<TProperties>::Value &&
                    ... &&
                    true
                );
```

### `StackCapacityCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of StackCapacity properties supplied to the declaration.

```cpp
static constexpr std::size_t StackCapacityCount =
                (static_cast<std::size_t>(IsStackCapacityProperty<TProperties>::Value) + ... + 0U);
```

### `PriorityCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of Priority properties supplied to the declaration.

```cpp
static constexpr std::size_t PriorityCount =
                (static_cast<std::size_t>(IsPriorityProperty<TProperties>::Value) + ... + 0U);
```

### `AffinityCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of affinity properties supplied to the declaration.

```cpp
static constexpr std::size_t AffinityCount =
                (static_cast<std::size_t>(IsAffinityProperty<TProperties>::Value) + ... + 0U);
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                EveryPropertyRecognized &&
                StackCapacityCount <= 1U &&
                PriorityCount <= 1U &&
                AffinityCount <= 1U;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `StackCapacityValue`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct StackCapacityValue
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = 0U;
```

### `StackCapacityValue`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `StackCapacityValue`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
        struct StackCapacityValue<StackCapacity<TCapacity>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCapacity;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `PriorityValue`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct PriorityValue
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ThreadPriority Value = ThreadPriority::Normal;
```

### `PriorityValue`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `PriorityValue`.
- **Template parameter `TPriority`:** Compile-time logical execution priority.

```cpp
template<ThreadPriority TPriority>
        struct PriorityValue<Priority<TPriority>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ThreadPriority Value = TPriority;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `AffinityValue`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct AffinityValue
```

### `Any`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ProcessorAffinity Value = ProcessorAffinity::Any();
```

### `AffinityValue`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `AffinityValue`.
- **Template parameter `TProcessorIndex`:** Compile-time logical processor index.

```cpp
template<std::uint32_t TProcessorIndex>
        struct AffinityValue<Affinity<TProcessorIndex>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ProcessorAffinity Value = Affinity<TProcessorIndex>::Value;
```

### `AffinityValue`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolves AnyAffinity to the portable Platform any-processor affinity value.

```cpp
template<>
        struct AffinityValue<AnyAffinity>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ProcessorAffinity Value = AnyAffinity::Value;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `TaskRecordCapacityValue`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct TaskRecordCapacityValue
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = 0U;
```

### `TaskRecordCapacityValue`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `TaskRecordCapacityValue`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
        struct TaskRecordCapacityValue<TaskRecordCapacity<TCapacity>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCapacity;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `CallableCapacityValue`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct CallableCapacityValue
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = 0U;
```

### `CallableCapacityValue`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `CallableCapacityValue`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
        struct CallableCapacityValue<CallableCapacity<TCapacity>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCapacity;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResultCapacityValue`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct ResultCapacityValue
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = 0U;
```

### `ResultCapacityValue`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResultCapacityValue`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
        struct ResultCapacityValue<ResultCapacity<TCapacity>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCapacity;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsTaskRecordCapacityProperty`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct IsTaskRecordCapacityProperty
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `IsTaskRecordCapacityProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsTaskRecordCapacityProperty`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
        struct IsTaskRecordCapacityProperty<TaskRecordCapacity<TCapacity>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsCallableCapacityProperty`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct IsCallableCapacityProperty
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `IsCallableCapacityProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsCallableCapacityProperty`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
        struct IsCallableCapacityProperty<CallableCapacity<TCapacity>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsResultCapacityProperty`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
        struct IsResultCapacityProperty
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `IsResultCapacityProperty`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsResultCapacityProperty`.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<std::size_t TCapacity>
        struct IsResultCapacityProperty<ResultCapacity<TCapacity>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `ValidDedicatedWorkerStorage`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class... TProperties>
        struct ValidDedicatedWorkerStorage
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                (static_cast<std::size_t>(IsTaskRecordCapacityProperty<TProperties>::Value) + ... + 0U) <= 1U &&
                (static_cast<std::size_t>(IsCallableCapacityProperty<TProperties>::Value) + ... + 0U) <= 1U &&
                (static_cast<std::size_t>(IsResultCapacityProperty<TProperties>::Value) + ... + 0U) <= 1U;
```

### `ResolvedDedicatedWorkerStorage`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class... TProperties>
        struct ResolvedDedicatedWorkerStorage
```

### `RecordCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved bounded Task-record capacity.

```cpp
static constexpr std::size_t RecordCapacity =
                (TaskRecordCapacityValue<TProperties>::Value + ... + 0U);
```

### `CallableCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved callable-storage capacity in bytes.

```cpp
static constexpr std::size_t CallableCapacity =
                (CallableCapacityValue<TProperties>::Value + ... + 0U);
```

### `ResultCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved result-storage capacity in bytes.

```cpp
static constexpr std::size_t ResultCapacity =
                (ResultCapacityValue<TProperties>::Value + ... + 0U);
```

### `ResolvedExecutionResourceProperties`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class... TProperties>
        struct ResolvedExecutionResourceProperties
```

### `TProperty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `ApplyPriority`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
```

### `ApplyPriority`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Applies a supplied priority property to the compile-time resolution accumulator.

```cpp
static constexpr void ApplyPriority(
                    ThreadPriority& result
                ) noexcept
```

### `TProperty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Defines the compile-time contract for `ApplyAffinity`.
- **Template parameter `TProperty`:** Compile-time property Type being classified or resolved.

```cpp
template<class TProperty>
```

### `ApplyAffinity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Applies a supplied affinity property to the compile-time resolution accumulator.

```cpp
static constexpr void ApplyAffinity(
                    ProcessorAffinity& result
                ) noexcept
```

### `StackCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved semantic execution stack capacity in bytes.

```cpp
static constexpr std::size_t StackCapacity =
                    (StackCapacityValue<TProperties>::Value + ... + 0U);
```

### `Priority`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved logical execution priority.

```cpp
static constexpr ThreadPriority Priority = []() constexpr
```

### `Affinity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved processor affinity.

```cpp
static constexpr ProcessorAffinity Affinity = []() constexpr
```

### `ResourceProperties`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class... TProperties>
    struct ResourceProperties final
```

### `Count`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of declarations represented by this property/resource pack.

```cpp
static constexpr std::size_t Count = sizeof...(TProperties);
```

### `StackCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved semantic execution stack capacity in bytes.

```cpp
static constexpr std::size_t StackCapacity =
            Detail::ResolvedExecutionResourceProperties<TProperties...>::StackCapacity;
```

### `Priority`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved logical execution priority.

```cpp
static constexpr ThreadPriority Priority =
            Detail::ResolvedExecutionResourceProperties<TProperties...>::Priority;
```

### `Affinity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved processor affinity.

```cpp
static constexpr ProcessorAffinity Affinity =
            Detail::ResolvedExecutionResourceProperties<TProperties...>::Affinity;
```

### `Worker`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TWorkerProperties`:** Compile-time execution-property Types declared for the Worker.

```cpp
template<class... TWorkerProperties>
    struct Worker final
```

### `Properties`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved execution-property metadata for this declaration.

```cpp
using Properties = ResourceProperties<TWorkerProperties...>;
```

### `Workers`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TWorkers`:** Worker declaration Types contained by this Worker/resource pack.

```cpp
template<class... TWorkers>
    struct Workers final
```

### `Count`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of declarations represented by this property/resource pack.

```cpp
static constexpr std::size_t Count = sizeof...(TWorkers);
```

### `TPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `TaskExecutionFacility`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type.
- **Template parameter `TWorkers`:** Declared Worker set Type.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
    struct TaskExecutionFacility final
```

### `PoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Semantic identity Type of this Task pool.

```cpp
using PoolIdentity = TPoolIdentity;
```

### `RecordCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Task-record capacity declaration Type.

```cpp
using RecordCapacity = TRecordCapacity;
```

### `CallableStorageCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Callable-storage capacity declaration Type.

```cpp
using CallableStorageCapacity = TCallableCapacity;
```

### `ResultStorageCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Result-storage capacity declaration Type.

```cpp
using ResultStorageCapacity = TResultCapacity;
```

### `WorkerSet`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Declared Worker-set Type.

```cpp
using WorkerSet = TWorkers;
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `DedicatedWorkerLease`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TWorkerProperties`:** Compile-time execution-property Types declared for the Worker.

```cpp
template<class TTaskIdentity, class... TWorkerProperties>
    struct DedicatedWorkerLease final
```

### `TaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Semantic identity Type of this Dedicated Worker task.

```cpp
using TaskIdentity = TTaskIdentity;
```

### `Properties`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved execution-only properties; bounded Task storage declarations are handled separately below.

```cpp
using Properties = Detail::ResolvedExecutionResourceProperties<TWorkerProperties...>;
```

### `WorkerCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of Workers owned by this execution resource.

```cpp
static constexpr std::size_t WorkerCount = 1U;
```

### `RecordCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved bounded Task-record capacity.

```cpp
static constexpr std::size_t RecordCapacity =
            Detail::ResolvedDedicatedWorkerStorage<TWorkerProperties...>::RecordCapacity;
```

### `CallableStorageCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved callable-storage capacity in bytes.

```cpp
static constexpr std::size_t CallableStorageCapacity =
            Detail::ResolvedDedicatedWorkerStorage<TWorkerProperties...>::CallableCapacity;
```

### `ResultStorageCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved result-storage capacity in bytes.

```cpp
static constexpr std::size_t ResultStorageCapacity =
            Detail::ResolvedDedicatedWorkerStorage<TWorkerProperties...>::ResultCapacity;
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `DedicatedThreadBinding`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.

```cpp
template<class TThreadIdentity, class TCallable>
    class DedicatedThreadBinding final
```

### `_callable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Application callable owned by this Dedicated Thread binding.

```cpp
TCallable _callable;
```

### `ThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Semantic identity Type of this Dedicated Thread.

```cpp
using ThreadIdentity = TThreadIdentity;
```

### `Callable`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Callable Type bound to this Dedicated Thread.

```cpp
using Callable = TCallable;
```

### `DedicatedThreadBinding`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Stores the application callable bound to this Dedicated Thread identity.

```cpp
explicit DedicatedThreadBinding(
                TCallable callable
            ) noexcept(
                std::is_nothrow_move_constructible_v<TCallable>
            ) :
                _callable(
                    std::move(
                        callable
                    )
                ) {}
```

### `TakeCallable`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Transfers the bound callable into the statically owned Dedicated Thread runtime.

```cpp
TCallable&& TakeCallable() noexcept
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `BindDedicatedThread`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TCallable`:** Callable Type whose invocation/storage contract is being adapted.

```cpp
template<class TThreadIdentity, class TCallable>
    auto BindDedicatedThread(
        TCallable&& callable
    )
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `DedicatedThread`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TThreadProperties`:** Compile-time execution-property Types declared for the Dedicated Thread.

```cpp
template<class TThreadIdentity, class... TThreadProperties>
    struct DedicatedThread final
```

### `ThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Semantic identity Type of this Dedicated Thread.

```cpp
using ThreadIdentity = TThreadIdentity;
```

### `Properties`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Resolved execution-property metadata for this declaration.

```cpp
using Properties = ResourceProperties<TThreadProperties...>;
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsTaskFacility`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.

```cpp
template<class TResource>
        struct IsTaskFacility
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsTaskFacility`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type.
- **Template parameter `TWorkers`:** Declared Worker set Type.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
        struct IsTaskFacility<TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ManagedContextCount`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.

```cpp
template<class TResource>
        struct ManagedContextCount
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = 0U;
```

### `TPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ManagedContextCount`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type.
- **Template parameter `TWorkers`:** Declared Worker set Type.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
        struct ManagedContextCount<
            TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>
        >
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TWorkers::Count;
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ManagedContextCount`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TTaskIdentity, class... TProperties>
        struct ManagedContextCount<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>
        >
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = 1U;
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ManagedContextCount`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TThreadIdentity, class... TProperties>
        struct ManagedContextCount<
            DedicatedThread<TThreadIdentity, TProperties...>
        >
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = 1U;
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsDedicatedWorkerLease`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.

```cpp
template<class TResource>
        struct IsDedicatedWorkerLease
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsDedicatedWorkerLease`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TTaskIdentity, class... TProperties>
        struct IsDedicatedWorkerLease<DedicatedWorkerLease<TTaskIdentity, TProperties...>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsDedicatedThread`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.

```cpp
template<class TResource>
        struct IsDedicatedThread
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `IsDedicatedThread`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TThreadProperties`:** Compile-time execution-property Types declared for the Dedicated Thread.

```cpp
template<class TThreadIdentity, class... TThreadProperties>
        struct IsDedicatedThread<DedicatedThread<TThreadIdentity, TThreadProperties...>>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TLeftResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceIdentityConflict`.
- **Template parameter `TLeftResource`:** Left topology resource Type in the identity-conflict comparison.
- **Template parameter `TRightResource`:** Right topology resource Type in the identity-conflict comparison.

```cpp
template<class TLeftResource, class TRightResource>
        struct ResourceIdentityConflict
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TLeftPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceIdentityConflict`.
- **Template parameter `TLeftPoolIdentity`:** Semantic identity Type of the left Task pool.
- **Template parameter `TLeftRecordCapacity`:** Record-capacity declaration Type of the left Task facility.
- **Template parameter `TLeftCallableCapacity`:** Callable-capacity declaration Type of the left Task facility.
- **Template parameter `TLeftResultCapacity`:** Result-capacity declaration Type of the left Task facility.
- **Template parameter `TLeftWorkers`:** Worker-set Type of the left Task facility.
- **Template parameter `TRightPoolIdentity`:** Semantic identity Type of the right Task pool.
- **Template parameter `TRightRecordCapacity`:** Record-capacity declaration Type of the right Task facility.
- **Template parameter `TRightCallableCapacity`:** Callable-capacity declaration Type of the right Task facility.
- **Template parameter `TRightResultCapacity`:** Result-capacity declaration Type of the right Task facility.
- **Template parameter `TRightWorkers`:** Worker-set Type of the right Task facility.

```cpp
template<class TLeftPoolIdentity, class TLeftRecordCapacity, class TLeftCallableCapacity, class TLeftResultCapacity, class TLeftWorkers, class TRightPoolIdentity, class TRightRecordCapacity, class TRightCallableCapacity, class TRightResultCapacity, class TRightWorkers>
        struct ResourceIdentityConflict<
            TaskExecutionFacility<TLeftPoolIdentity, TLeftRecordCapacity, TLeftCallableCapacity, TLeftResultCapacity, TLeftWorkers>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = std::is_same_v<TLeftPoolIdentity, TRightPoolIdentity>;
```

### `TLeftThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceIdentityConflict`.
- **Template parameter `TLeftThreadIdentity`:** Semantic identity Type of the left Dedicated Thread.
- **Template parameter `TRightThreadIdentity`:** Semantic identity Type of the right Dedicated Thread.
- **Template parameter `TLeftProperties`:** Compile-time property Types declared by the left resource in this comparison.
- **Template parameter `TRightProperties`:** Compile-time property Types declared by the right resource in this comparison.

```cpp
template<class TLeftThreadIdentity, class... TLeftProperties, class TRightThreadIdentity, class... TRightProperties>
        struct ResourceIdentityConflict<
            DedicatedThread<TLeftThreadIdentity, TLeftProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = std::is_same_v<TLeftThreadIdentity, TRightThreadIdentity>;
```

### `TLeftTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceIdentityConflict`.
- **Template parameter `TLeftTaskIdentity`:** Semantic identity Type of the left Dedicated Worker task.
- **Template parameter `TRightTaskIdentity`:** Semantic identity Type of the right Dedicated Worker task.
- **Template parameter `TLeftProperties`:** Compile-time property Types declared by the left resource in this comparison.
- **Template parameter `TRightProperties`:** Compile-time property Types declared by the right resource in this comparison.

```cpp
template<class TLeftTaskIdentity, class... TLeftProperties, class TRightTaskIdentity, class... TRightProperties>
        struct ResourceIdentityConflict<
            DedicatedWorkerLease<TLeftTaskIdentity, TLeftProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = std::is_same_v<TLeftTaskIdentity, TRightTaskIdentity>;
```

### `UniqueResourceIdentities`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TResources`:** Topology resource Types inspected or represented by this compile-time operation.

```cpp
template<class... TResources>
        struct UniqueResourceIdentities;
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TFirstResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `UniqueResourceIdentities`.
- **Template parameter `TFirstResource`:** First topology resource Type in the recursive uniqueness check.
- **Template parameter `TRestResources`:** Remaining topology resource Types inspected by the recursive operation.

```cpp
template<class TFirstResource, class... TRestResources>
        struct UniqueResourceIdentities<TFirstResource, TRestResources...>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                ((!ResourceIdentityConflict<TFirstResource, TRestResources>::Value) && ...) &&
                UniqueResourceIdentities<TRestResources...>::Value;
```

### `TopologyProviderBase`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `TopologyProviderBase`.
- **Template parameter `THasTaskExecution`:** Whether the topology exposes Task execution capability.
- **Template parameter `THasDedicatedThreadExecution`:** Whether the topology exposes Dedicated Thread execution capability.

```cpp
template<bool THasTaskExecution, bool THasDedicatedThreadExecution>
        struct TopologyProviderBase;
```

### `TopologyProviderBase`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Empty capability base used when a topology exposes neither Task nor Dedicated Thread execution.

```cpp
template<>
        struct TopologyProviderBase<false, false> {};
```

### `TopologyProviderBase`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Capability base exposing Task execution only.

```cpp
template<>
        struct TopologyProviderBase<true, false> : Framework::Provider<
            Domain,
```

### `TopologyProviderBase`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Capability base exposing Dedicated Thread execution only.

```cpp
template<>
        struct TopologyProviderBase<false, true> : Framework::Provider<
            Domain,
```

### `TopologyProviderBase`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Capability base exposing both Task and Dedicated Thread execution.

```cpp
template<>
        struct TopologyProviderBase<true, true> : Framework::Provider<
            Domain,
```

### `ThreadingTopology`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

- **Template parameter `TResources`:** Topology resource Types inspected or represented by this compile-time operation.

```cpp
template<class... TResources>
    struct ThreadingTopology final : Detail::TopologyProviderBase<
        (Detail::IsTaskFacility<TResources>::Value || ... || false),
```

### `ResourceCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of resources declared by this topology.

```cpp
static constexpr std::size_t ResourceCount = sizeof...(TResources);
```

### `HasTaskExecution`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Whether this topology provides ordinary Task execution.

```cpp
static constexpr bool HasTaskExecution =
            (Detail::IsTaskFacility<TResources>::Value || ... || false);
```

### `HasDedicatedThreadExecution`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Whether this topology provides Dedicated Thread execution.

```cpp
static constexpr bool HasDedicatedThreadExecution =
            (Detail::IsDedicatedThread<TResources>::Value || ... || false);
```

### `HasDedicatedWorkerLease`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Whether this topology declares any Dedicated Worker lease.

```cpp
static constexpr bool HasDedicatedWorkerLease =
            (Detail::IsDedicatedWorkerLease<TResources>::Value || ... || false);
```

### `TaskFacilityCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of ordinary Task facilities in this topology.

```cpp
static constexpr std::size_t TaskFacilityCount =
            (static_cast<std::size_t>(Detail::IsTaskFacility<TResources>::Value) + ... + 0U);
```

### `DedicatedWorkerLeaseCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of Dedicated Worker leases in this topology.

```cpp
static constexpr std::size_t DedicatedWorkerLeaseCount =
            (static_cast<std::size_t>(Detail::IsDedicatedWorkerLease<TResources>::Value) + ... + 0U);
```

### `DedicatedThreadCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Number of Dedicated Threads in this topology.

```cpp
static constexpr std::size_t DedicatedThreadCount =
            (static_cast<std::size_t>(Detail::IsDedicatedThread<TResources>::Value) + ... + 0U);
```

### `ManagedExecutionContextCount`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Total number of topology-owned managed execution contexts.

```cpp
static constexpr std::size_t ManagedExecutionContextCount =
            (Detail::ManagedContextCount<TResources>::Value + ... + 0U);
```

### `Resources`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Tuple Type containing every topology resource declaration.

```cpp
using Resources = std::tuple<TResources...>;
```

### `TPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `TaskPoolRequirement`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class TPoolIdentity, class... TConstraints>
    struct TaskPoolRequirement final
```

### `PoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Semantic identity Type of this Task pool.

```cpp
using PoolIdentity = TPoolIdentity;
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `DedicatedThreadRequirement`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class TThreadIdentity, class... TConstraints>
    struct DedicatedThreadRequirement final
```

### `ThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Semantic identity Type of this Dedicated Thread.

```cpp
using ThreadIdentity = TThreadIdentity;
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `DedicatedWorkerRequirement`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class TTaskIdentity, class... TConstraints>
    struct DedicatedWorkerRequirement final
```

### `TaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Semantic identity Type of this Dedicated Worker task.

```cpp
using TaskIdentity = TTaskIdentity;
```

### `TIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MatchesTaskPoolIdentity`.
- **Template parameter `TIdentity`:** Semantic identity Type being matched against the inspected topology resource.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.

```cpp
template<class TIdentity, class TResource>
        struct MatchesTaskPoolIdentity
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MatchesTaskPoolIdentity`.
- **Template parameter `TIdentity`:** Semantic identity Type being matched against the inspected topology resource.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type.
- **Template parameter `TWorkers`:** Declared Worker set Type.

```cpp
template<class TIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
        struct MatchesTaskPoolIdentity<
            TIdentity,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MatchesDedicatedThreadIdentity`.
- **Template parameter `TIdentity`:** Semantic identity Type being matched against the inspected topology resource.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.

```cpp
template<class TIdentity, class TResource>
        struct MatchesDedicatedThreadIdentity
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MatchesDedicatedThreadIdentity`.
- **Template parameter `TIdentity`:** Semantic identity Type being matched against the inspected topology resource.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TIdentity, class... TProperties>
        struct MatchesDedicatedThreadIdentity<
            TIdentity,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MatchesDedicatedWorkerIdentity`.
- **Template parameter `TIdentity`:** Semantic identity Type being matched against the inspected topology resource.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.

```cpp
template<class TIdentity, class TResource>
        struct MatchesDedicatedWorkerIdentity
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MatchesDedicatedWorkerIdentity`.
- **Template parameter `TIdentity`:** Semantic identity Type being matched against the inspected topology resource.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TIdentity, class... TProperties>
        struct MatchesDedicatedWorkerIdentity<
            TIdentity,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = true;
```

### `TTopology`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `RequirementSatisfied`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TRequirement`:** Compile-time Threading requirement Type being evaluated.

```cpp
template<class TTopology, class TRequirement>
        struct RequirementSatisfied;
```

### `TPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `RequirementSatisfied`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TResources`:** Topology resource Types inspected or represented by this compile-time operation.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class... TResources, class TPoolIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                (MatchesTaskPoolIdentity<TPoolIdentity, TResources>::Value || ... || false);
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `RequirementSatisfied`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TResources`:** Topology resource Types inspected or represented by this compile-time operation.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class... TResources, class TThreadIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                (MatchesDedicatedThreadIdentity<TThreadIdentity, TResources>::Value || ... || false);
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `RequirementSatisfied`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TResources`:** Topology resource Types inspected or represented by this compile-time operation.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class... TResources, class TTaskIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                (MatchesDedicatedWorkerIdentity<TTaskIdentity, TResources>::Value || ... || false);
```

### `MinimumStackCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MinimumStackCapacity`.
- **Template parameter `TBytes`:** Compile-time byte requirement.

```cpp
template<std::size_t TBytes>
    struct MinimumStackCapacity final
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TBytes;
```

### `MinimumWorkerConcurrency`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MinimumWorkerConcurrency`.
- **Template parameter `TCount`:** Compile-time count requirement.

```cpp
template<std::size_t TCount>
    struct MinimumWorkerConcurrency final
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr std::size_t Value = TCount;
```

### `MinimumPriority`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `MinimumPriority`.
- **Template parameter `TPriority`:** Compile-time logical execution priority.

```cpp
template<ThreadPriority TPriority>
    struct MinimumPriority final
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr ThreadPriority Value = TPriority;
```

### `TAffinity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `RequiredAffinity`.
- **Template parameter `TAffinity`:** Affinity requirement Type.

```cpp
template<class TAffinity>
    struct RequiredAffinity final
```

### `AffinityType`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Affinity Type required by this compile-time constraint.

```cpp
using AffinityType = TAffinity;
```

### `HasMinimumStackCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasMinimumStackCapacity`.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<std::size_t TMinimum, class... TProperties>
        struct HasMinimumStackCapacity;
```

### `HasMinimumStackCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasMinimumStackCapacity`.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.

```cpp
template<std::size_t TMinimum>
        struct HasMinimumStackCapacity<TMinimum>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `HasMinimumStackCapacity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasMinimumStackCapacity`.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.
- **Template parameter `TRest`:** Remaining property/value Types inspected by the recursive compile-time operation.

```cpp
template<std::size_t TMinimum, std::size_t TCapacity, class... TRest>
        struct HasMinimumStackCapacity<
            TMinimum,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = TCapacity >= TMinimum;
```

### `TFirst`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasMinimumStackCapacity`.
- **Template parameter `TFirst`:** First property Type in the recursive compile-time search.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TRest`:** Remaining property/value Types inspected by the recursive compile-time operation.

```cpp
template<std::size_t TMinimum, class TFirst, class... TRest>
        struct HasMinimumStackCapacity<
            TMinimum,
```

### `HasMinimumPriority`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasMinimumPriority`.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<ThreadPriority TMinimum, class... TProperties>
        struct HasMinimumPriority;
```

### `HasMinimumPriority`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasMinimumPriority`.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.

```cpp
template<ThreadPriority TMinimum>
        struct HasMinimumPriority<TMinimum>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `HasMinimumPriority`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasMinimumPriority`.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TValue`:** Compile-time value being compared with the minimum.
- **Template parameter `TRest`:** Remaining property/value Types inspected by the recursive compile-time operation.

```cpp
template<ThreadPriority TMinimum, ThreadPriority TValue, class... TRest>
        struct HasMinimumPriority<
            TMinimum,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                static_cast<std::uint8_t>(TValue) >=
                static_cast<std::uint8_t>(TMinimum);
```

### `TFirst`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasMinimumPriority`.
- **Template parameter `TFirst`:** First property Type in the recursive compile-time search.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TRest`:** Remaining property/value Types inspected by the recursive compile-time operation.

```cpp
template<ThreadPriority TMinimum, class TFirst, class... TRest>
        struct HasMinimumPriority<
            TMinimum,
```

### `TRequiredAffinity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasRequiredAffinity`.
- **Template parameter `TRequiredAffinity`:** Affinity Type required by the consumer constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TRequiredAffinity, class... TProperties>
        struct HasRequiredAffinity;
```

### `TRequiredAffinity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasRequiredAffinity`.
- **Template parameter `TRequiredAffinity`:** Affinity Type required by the consumer constraint.

```cpp
template<class TRequiredAffinity>
        struct HasRequiredAffinity<TRequiredAffinity>
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TRequiredAffinity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `HasRequiredAffinity`.
- **Template parameter `TRequiredAffinity`:** Affinity Type required by the consumer constraint.
- **Template parameter `TFirst`:** First property Type in the recursive compile-time search.
- **Template parameter `TRest`:** Remaining property/value Types inspected by the recursive compile-time operation.

```cpp
template<class TRequiredAffinity, class TFirst, class... TRest>
        struct HasRequiredAffinity<
            TRequiredAffinity,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                std::is_same_v<TRequiredAffinity, TFirst> ||
                HasRequiredAffinity<TRequiredAffinity, TRest...>::Value;
```

### `TWorker`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `WorkerConstraintSatisfied`.
- **Template parameter `TWorker`:** Worker declaration Type being validated.
- **Template parameter `TConstraint`:** Constraint Type being evaluated.

```cpp
template<class TWorker, class TConstraint>
        struct WorkerConstraintSatisfied
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `WorkerConstraintSatisfied`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `WorkerConstraintSatisfied`.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class... TProperties, std::size_t TMinimum>
        struct WorkerConstraintSatisfied<
            Worker<TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasMinimumStackCapacity<TMinimum, TProperties...>::Value;
```

### `WorkerConstraintSatisfied`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `WorkerConstraintSatisfied`.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class... TProperties, ThreadPriority TMinimum>
        struct WorkerConstraintSatisfied<
            Worker<TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasMinimumPriority<TMinimum, TProperties...>::Value;
```

### `TAffinity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `WorkerConstraintSatisfied`.
- **Template parameter `TAffinity`:** Affinity requirement Type.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class... TProperties, class TAffinity>
        struct WorkerConstraintSatisfied<
            Worker<TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasRequiredAffinity<TAffinity, TProperties...>::Value;
```

### `TWorkers`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `EveryWorkerSatisfies`.
- **Template parameter `TWorkers`:** Declared Worker set Type.
- **Template parameter `TConstraint`:** Constraint Type being evaluated.

```cpp
template<class TWorkers, class TConstraint>
        struct EveryWorkerSatisfies;
```

### `TConstraint`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `EveryWorkerSatisfies`.
- **Template parameter `TConstraint`:** Constraint Type being evaluated.
- **Template parameter `TWorkers`:** Worker declaration Types contained by this Worker/resource pack.

```cpp
template<class... TWorkers, class TConstraint>
        struct EveryWorkerSatisfies<
            Workers<TWorkers...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                (WorkerConstraintSatisfied<TWorkers, TConstraint>::Value && ... && true);
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.
- **Template parameter `TConstraint`:** Constraint Type being evaluated.

```cpp
template<class TResource, class TConstraint>
        struct ResourceConstraintSatisfied
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type.
- **Template parameter `TWorkers`:** Declared Worker set Type.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers, std::size_t TMinimum>
        struct ResourceConstraintSatisfied<
            TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = TWorkers::Count >= TMinimum;
```

### `TPoolIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type.
- **Template parameter `TWorkers`:** Declared Worker set Type.
- **Template parameter `TConstraint`:** Constraint Type being evaluated.

```cpp
template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers, class TConstraint>
        struct ResourceConstraintSatisfied<
            TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                EveryWorkerSatisfies<TWorkers, TConstraint>::Value;
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TTaskIdentity, class... TProperties, std::size_t TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = 1U >= TMinimum;
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TTaskIdentity, class... TProperties, std::size_t TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasMinimumStackCapacity<TMinimum, TProperties...>::Value;
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TTaskIdentity, class... TProperties, ThreadPriority TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasMinimumPriority<TMinimum, TProperties...>::Value;
```

### `TTaskIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TAffinity`:** Affinity requirement Type.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TTaskIdentity, class... TProperties, class TAffinity>
        struct ResourceConstraintSatisfied<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasRequiredAffinity<TAffinity, TProperties...>::Value;
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TThreadIdentity, class... TProperties, std::size_t TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedThread<TThreadIdentity, TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasMinimumStackCapacity<TMinimum, TProperties...>::Value;
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TMinimum`:** Compile-time minimum value required by the constraint.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TThreadIdentity, class... TProperties, ThreadPriority TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedThread<TThreadIdentity, TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasMinimumPriority<TMinimum, TProperties...>::Value;
```

### `TThreadIdentity`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceConstraintSatisfied`.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TAffinity`:** Affinity requirement Type.
- **Template parameter `TProperties`:** Compile-time resource/property Types supplied to this declaration or trait specialization.

```cpp
template<class TThreadIdentity, class... TProperties, class TAffinity>
        struct ResourceConstraintSatisfied<
            DedicatedThread<TThreadIdentity, TProperties...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                HasRequiredAffinity<TAffinity, TProperties...>::Value;
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceSatisfiesRequirement`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.
- **Template parameter `TRequirement`:** Compile-time Threading requirement Type being evaluated.

```cpp
template<class TResource, class TRequirement>
        struct ResourceSatisfiesRequirement
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value = false;
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceSatisfiesRequirement`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.
- **Template parameter `TPoolIdentity`:** Semantic identity Type of the Task pool.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class TResource, class TPoolIdentity, class... TConstraints>
        struct ResourceSatisfiesRequirement<
            TResource,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                MatchesTaskPoolIdentity<TPoolIdentity, TResource>::Value &&
                (ResourceConstraintSatisfied<TResource, TConstraints>::Value && ... && true);
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceSatisfiesRequirement`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.
- **Template parameter `TThreadIdentity`:** Semantic identity Type of the Dedicated Thread.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class TResource, class TThreadIdentity, class... TConstraints>
        struct ResourceSatisfiesRequirement<
            TResource,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                MatchesDedicatedThreadIdentity<TThreadIdentity, TResource>::Value &&
                (ResourceConstraintSatisfied<TResource, TConstraints>::Value && ... && true);
```

### `TResource`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ResourceSatisfiesRequirement`.
- **Template parameter `TResource`:** Topology resource Type being classified or inspected.
- **Template parameter `TTaskIdentity`:** Semantic identity Type of the dedicated Task.
- **Template parameter `TConstraints`:** Compile-time requirement constraint Types evaluated by this requirement.

```cpp
template<class TResource, class TTaskIdentity, class... TConstraints>
        struct ResourceSatisfiesRequirement<
            TResource,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                MatchesDedicatedWorkerIdentity<TTaskIdentity, TResource>::Value &&
                (ResourceConstraintSatisfied<TResource, TConstraints>::Value && ... && true);
```

### `TTopology`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ConstrainedRequirementSatisfied`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TRequirement`:** Compile-time Threading requirement Type being evaluated.

```cpp
template<class TTopology, class TRequirement>
        struct ConstrainedRequirementSatisfied;
```

### `TRequirement`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `ConstrainedRequirementSatisfied`.
- **Template parameter `TRequirement`:** Compile-time Threading requirement Type being evaluated.
- **Template parameter `TResources`:** Topology resource Types inspected or represented by this compile-time operation.

```cpp
template<class... TResources, class TRequirement>
        struct ConstrainedRequirementSatisfied<
            ThreadingTopology<TResources...>,
```

### `Value`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Compile-time result produced by this trait or property specialization.

```cpp
static constexpr bool Value =
                (ResourceSatisfiesRequirement<TResources, TRequirement>::Value || ... || false);
```

### `TTopology`

**Classification:** PUBLIC COMPOSITION / TOPOLOGY API · source access: `public`

Defines the compile-time contract for `templated declaration`.
- **Template parameter `TTopology`:** Compile-time Threading topology being realized or inspected.
- **Template parameter `TRequirement`:** Compile-time Threading requirement Type being evaluated.

```cpp
template<class TTopology, class TRequirement>
    inline constexpr bool SatisfiesThreadingRequirement =
        Detail::ConstrainedRequirementSatisfied<
            TTopology,
```

