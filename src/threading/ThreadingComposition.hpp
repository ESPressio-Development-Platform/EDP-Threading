#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <tuple>
#include <utility>

#include <ESPressio_System.hpp>

#include "ThreadingTypes.hpp"

namespace ESPressio::Threading {

    namespace Framework = ESPressio::System::CompositionFramework;

    struct Domain final : Framework::Domain {};


    struct TaskExecution final : Framework::SharedCapability<Domain> {};


    struct DedicatedThreadExecution final : Framework::SharedCapability<Domain> {};


    struct StackCapacityBytes final : Framework::Property<TaskExecution, std::size_t> {};


    struct WorkerConcurrency final : Framework::Property<TaskExecution, std::size_t> {};


    /// Defines the compile-time contract for `TaskPool`.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    template<class TPoolIdentity>
    struct TaskPool final {};


    /// Defines the compile-time contract for `TaskRecordCapacity`.
    /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
    template<std::size_t TCapacity>
    struct TaskRecordCapacity final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr std::size_t Value = TCapacity;

    };


    /// Defines the compile-time contract for `CallableCapacity`.
    /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
    template<std::size_t TCapacity>
    struct CallableCapacity final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr std::size_t Value = TCapacity;

    };


    /// Defines the compile-time contract for `ResultCapacity`.
    /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
    template<std::size_t TCapacity>
    struct ResultCapacity final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr std::size_t Value = TCapacity;

    };


    /// Defines the compile-time contract for `StackCapacity`.
    /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
    template<std::size_t TCapacity>
    struct StackCapacity final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr std::size_t Value = TCapacity;

    };


    /// Defines the compile-time contract for `Priority`.
    /// @tparam TPriority Compile-time logical execution priority.
    template<ThreadPriority TPriority>
    struct Priority final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr ThreadPriority Value = TPriority;

    };


    /// Defines the compile-time contract for `Affinity`.
    /// @tparam TProcessorIndex Compile-time logical processor index.
    template<std::uint32_t TProcessorIndex>
    struct Affinity final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr ProcessorAffinity Value = ProcessorAffinity::Specific(TProcessorIndex);

    };


    struct AnyAffinity final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr ProcessorAffinity Value = ProcessorAffinity::Any();

    };


    namespace Detail {

        /// Defines the compile-time contract for `IsStackCapacityProperty`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct IsStackCapacityProperty {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsStackCapacityProperty`.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TCapacity>
        struct IsStackCapacityProperty<StackCapacity<TCapacity>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `IsPriorityProperty`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct IsPriorityProperty {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsPriorityProperty`.
        /// @tparam TPriority Compile-time logical execution priority.
        template<ThreadPriority TPriority>
        struct IsPriorityProperty<Priority<TPriority>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `IsAffinityProperty`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct IsAffinityProperty {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsAffinityProperty`.
        /// @tparam TProcessorIndex Compile-time logical processor index.
        template<std::uint32_t TProcessorIndex>
        struct IsAffinityProperty<Affinity<TProcessorIndex>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        template<>
        struct IsAffinityProperty<AnyAffinity> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `IsExecutionResourceProperty`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct IsExecutionResourceProperty {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                IsStackCapacityProperty<TProperty>::Value ||
                IsPriorityProperty<TProperty>::Value ||
                IsAffinityProperty<TProperty>::Value;

        };


        template<class... TProperties>
        struct ValidExecutionResourceProperties {

            /// Whether every supplied execution-resource property is recognized.
            static constexpr bool EveryPropertyRecognized =
                (
                    IsExecutionResourceProperty<TProperties>::Value &&
                    ... &&
                    true
                );

            /// Number of StackCapacity properties supplied to the declaration.
            static constexpr std::size_t StackCapacityCount =
                (static_cast<std::size_t>(IsStackCapacityProperty<TProperties>::Value) + ... + 0U);

            /// Number of Priority properties supplied to the declaration.
            static constexpr std::size_t PriorityCount =
                (static_cast<std::size_t>(IsPriorityProperty<TProperties>::Value) + ... + 0U);

            /// Number of affinity properties supplied to the declaration.
            static constexpr std::size_t AffinityCount =
                (static_cast<std::size_t>(IsAffinityProperty<TProperties>::Value) + ... + 0U);

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                EveryPropertyRecognized &&
                StackCapacityCount <= 1U &&
                PriorityCount <= 1U &&
                AffinityCount <= 1U;

        };

    } // ESPressio::Threading::Detail


    namespace Detail {

        /// Defines the compile-time contract for `StackCapacityValue`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct StackCapacityValue {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = 0U;

        };


        /// Defines the compile-time contract for `StackCapacityValue`.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TCapacity>
        struct StackCapacityValue<StackCapacity<TCapacity>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = TCapacity;

        };


        /// Defines the compile-time contract for `PriorityValue`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct PriorityValue {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr ThreadPriority Value = ThreadPriority::Normal;

        };


        /// Defines the compile-time contract for `PriorityValue`.
        /// @tparam TPriority Compile-time logical execution priority.
        template<ThreadPriority TPriority>
        struct PriorityValue<Priority<TPriority>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr ThreadPriority Value = TPriority;

        };


        /// Defines the compile-time contract for `AffinityValue`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct AffinityValue {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr ProcessorAffinity Value = ProcessorAffinity::Any();

        };


        /// Defines the compile-time contract for `AffinityValue`.
        /// @tparam TProcessorIndex Compile-time logical processor index.
        template<std::uint32_t TProcessorIndex>
        struct AffinityValue<Affinity<TProcessorIndex>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr ProcessorAffinity Value = Affinity<TProcessorIndex>::Value;

        };


        template<>
        struct AffinityValue<AnyAffinity> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr ProcessorAffinity Value = AnyAffinity::Value;

        };


        /// Defines the compile-time contract for `TaskRecordCapacityValue`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct TaskRecordCapacityValue {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = 0U;

        };


        /// Defines the compile-time contract for `TaskRecordCapacityValue`.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TCapacity>
        struct TaskRecordCapacityValue<TaskRecordCapacity<TCapacity>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = TCapacity;

        };


        /// Defines the compile-time contract for `CallableCapacityValue`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct CallableCapacityValue {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = 0U;

        };


        /// Defines the compile-time contract for `CallableCapacityValue`.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TCapacity>
        struct CallableCapacityValue<CallableCapacity<TCapacity>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = TCapacity;

        };


        /// Defines the compile-time contract for `ResultCapacityValue`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct ResultCapacityValue {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = 0U;

        };


        /// Defines the compile-time contract for `ResultCapacityValue`.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TCapacity>
        struct ResultCapacityValue<ResultCapacity<TCapacity>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = TCapacity;

        };


        /// Defines the compile-time contract for `IsTaskRecordCapacityProperty`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct IsTaskRecordCapacityProperty {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsTaskRecordCapacityProperty`.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TCapacity>
        struct IsTaskRecordCapacityProperty<TaskRecordCapacity<TCapacity>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `IsCallableCapacityProperty`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct IsCallableCapacityProperty {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsCallableCapacityProperty`.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TCapacity>
        struct IsCallableCapacityProperty<CallableCapacity<TCapacity>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `IsResultCapacityProperty`.
        /// @tparam TProperty Compile-time property Type being classified or resolved.
        template<class TProperty>
        struct IsResultCapacityProperty {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsResultCapacityProperty`.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TCapacity>
        struct IsResultCapacityProperty<ResultCapacity<TCapacity>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        template<class... TProperties>
        struct ValidDedicatedWorkerStorage {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                (static_cast<std::size_t>(IsTaskRecordCapacityProperty<TProperties>::Value) + ... + 0U) <= 1U &&
                (static_cast<std::size_t>(IsCallableCapacityProperty<TProperties>::Value) + ... + 0U) <= 1U &&
                (static_cast<std::size_t>(IsResultCapacityProperty<TProperties>::Value) + ... + 0U) <= 1U;

        };


        template<class... TProperties>
        struct ResolvedDedicatedWorkerStorage {

            /// Resolved bounded Task-record capacity.
            static constexpr std::size_t RecordCapacity =
                (TaskRecordCapacityValue<TProperties>::Value + ... + 0U);

            /// Resolved callable-storage capacity in bytes.
            static constexpr std::size_t CallableCapacity =
                (CallableCapacityValue<TProperties>::Value + ... + 0U);

            /// Resolved result-storage capacity in bytes.
            static constexpr std::size_t ResultCapacity =
                (ResultCapacityValue<TProperties>::Value + ... + 0U);

        };


        template<class... TProperties>
        struct ResolvedExecutionResourceProperties {

            private:

                /// Defines the compile-time contract for `ApplyPriority`.
                /// @tparam TProperty Compile-time property Type being classified or resolved.
                template<class TProperty>
                /// Applies a supplied priority property to the compile-time resolution accumulator.
                static constexpr void ApplyPriority(
                    ThreadPriority& result
                ) noexcept {
                    if constexpr (
                        IsPriorityProperty<TProperty>::Value
                    ) {
                        result = PriorityValue<TProperty>::Value;
                    }
                }

                /// Defines the compile-time contract for `ApplyAffinity`.
                /// @tparam TProperty Compile-time property Type being classified or resolved.
                template<class TProperty>
                /// Applies a supplied affinity property to the compile-time resolution accumulator.
                static constexpr void ApplyAffinity(
                    ProcessorAffinity& result
                ) noexcept {
                    if constexpr (
                        IsAffinityProperty<TProperty>::Value
                    ) {
                        result = AffinityValue<TProperty>::Value;
                    }
                }

            public:

                /// Resolved semantic execution stack capacity in bytes.
                static constexpr std::size_t StackCapacity =
                    (StackCapacityValue<TProperties>::Value + ... + 0U);

                /// Resolved logical execution priority.
                static constexpr ThreadPriority Priority = []() constexpr {
                    ThreadPriority result = ThreadPriority::Normal;
                    (ApplyPriority<TProperties>(result), ...);
                    return result;
                }();

                /// Resolved processor affinity.
                static constexpr ProcessorAffinity Affinity = []() constexpr {
                    ProcessorAffinity result = ProcessorAffinity::Any();
                    (ApplyAffinity<TProperties>(result), ...);
                    return result;
                }();

        };

    } // ESPressio::Threading::Detail


    template<class... TProperties>
    struct ResourceProperties final {

        static_assert(
            Detail::ValidExecutionResourceProperties<TProperties...>::Value,
            "Execution resource properties must be recognized and contain at most one StackCapacity, Priority and affinity declaration"
        );

        /// Number of declarations represented by this property/resource pack.
        static constexpr std::size_t Count = sizeof...(TProperties);
        /// Resolved semantic execution stack capacity in bytes.
        static constexpr std::size_t StackCapacity =
            Detail::ResolvedExecutionResourceProperties<TProperties...>::StackCapacity;
        /// Resolved logical execution priority.
        static constexpr ThreadPriority Priority =
            Detail::ResolvedExecutionResourceProperties<TProperties...>::Priority;
        /// Resolved processor affinity.
        static constexpr ProcessorAffinity Affinity =
            Detail::ResolvedExecutionResourceProperties<TProperties...>::Affinity;

    };


    template<class... TWorkerProperties>
    struct Worker final {

        static_assert(
            Detail::ValidExecutionResourceProperties<TWorkerProperties...>::Value,
            "Execution resource properties must be recognized and contain at most one StackCapacity, Priority and affinity declaration"
        );

        /// Resolved execution-property metadata for this declaration.
        using Properties = ResourceProperties<TWorkerProperties...>;

    };


    template<class... TWorkers>
    struct Workers final {

        /// Number of declarations represented by this property/resource pack.
        static constexpr std::size_t Count = sizeof...(TWorkers);

    };


    /// Defines the compile-time contract for `TaskExecutionFacility`.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    /// @tparam TRecordCapacity Task-record capacity declaration Type.
    /// @tparam TCallableCapacity Callable-storage capacity declaration Type.
    /// @tparam TResultCapacity Result-storage capacity declaration Type.
    /// @tparam TWorkers Declared Worker set Type.
    template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
    struct TaskExecutionFacility final {

        static_assert(
            (TRecordCapacity::Value == 0U && TWorkers::Count == 0U) ||
            (TRecordCapacity::Value > 0U && TWorkers::Count > 0U),
            "TaskExecutionFacility must either compile away at zero records/workers or provide both record and Worker capacity"
        );

        static_assert(
            TRecordCapacity::Value == 0U ||
            TCallableCapacity::Value > 0U,
            "An active TaskExecutionFacility requires positive callable capacity; result capacity may be zero for void-only work"
        );

        /// Semantic identity Type of this Task pool.
        using PoolIdentity = TPoolIdentity;
        /// Task-record capacity declaration Type.
        using RecordCapacity = TRecordCapacity;
        /// Callable-storage capacity declaration Type.
        using CallableStorageCapacity = TCallableCapacity;
        /// Result-storage capacity declaration Type.
        using ResultStorageCapacity = TResultCapacity;
        /// Declared Worker-set Type.
        using WorkerSet = TWorkers;

    };


    /// Defines the compile-time contract for `DedicatedWorkerLease`.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    template<class TTaskIdentity, class... TWorkerProperties>
    struct DedicatedWorkerLease final {

        static_assert(
            (
                (
                    Detail::IsExecutionResourceProperty<TWorkerProperties>::Value ||
                    Detail::IsTaskRecordCapacityProperty<TWorkerProperties>::Value ||
                    Detail::IsCallableCapacityProperty<TWorkerProperties>::Value ||
                    Detail::IsResultCapacityProperty<TWorkerProperties>::Value
                ) &&
                ... &&
                true
            ),
            "DedicatedWorkerLease contains an unrecognized execution/storage property"
        );

        static_assert(
            (
                static_cast<std::size_t>(
                    Detail::IsStackCapacityProperty<TWorkerProperties>::Value
                ) +
                ... +
                0U
            ) <= 1U &&
            (
                static_cast<std::size_t>(
                    Detail::IsPriorityProperty<TWorkerProperties>::Value
                ) +
                ... +
                0U
            ) <= 1U &&
            (
                static_cast<std::size_t>(
                    Detail::IsAffinityProperty<TWorkerProperties>::Value
                ) +
                ... +
                0U
            ) <= 1U,
            "DedicatedWorkerLease contains duplicate StackCapacity, Priority or affinity properties"
        );

        static_assert(
            Detail::ValidDedicatedWorkerStorage<TWorkerProperties...>::Value,
            "DedicatedWorkerLease contains duplicate TaskRecordCapacity, CallableCapacity or ResultCapacity properties"
        );

        /// Semantic identity Type of this Dedicated Worker task.
        using TaskIdentity = TTaskIdentity;

        /// Resolved execution-only properties; bounded Task storage declarations are handled separately below.
        using Properties = Detail::ResolvedExecutionResourceProperties<TWorkerProperties...>;

        /// Number of Workers owned by this execution resource.
        static constexpr std::size_t WorkerCount = 1U;
        /// Resolved bounded Task-record capacity.
        static constexpr std::size_t RecordCapacity =
            Detail::ResolvedDedicatedWorkerStorage<TWorkerProperties...>::RecordCapacity;
        /// Resolved callable-storage capacity in bytes.
        static constexpr std::size_t CallableStorageCapacity =
            Detail::ResolvedDedicatedWorkerStorage<TWorkerProperties...>::CallableCapacity;
        /// Resolved result-storage capacity in bytes.
        static constexpr std::size_t ResultStorageCapacity =
            Detail::ResolvedDedicatedWorkerStorage<TWorkerProperties...>::ResultCapacity;

        static_assert(
            RecordCapacity > 0U && CallableStorageCapacity > 0U,
            "DedicatedWorkerLease requires positive TaskRecordCapacity and CallableCapacity"
        );

    };


    /// Defines the compile-time contract for `DedicatedThreadBinding`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    template<class TThreadIdentity, class TCallable>
    class DedicatedThreadBinding final {

        static_assert(
            std::is_move_constructible_v<TCallable>,
            "Dedicated Thread callable bindings require move-constructible callables"
        );

        static_assert(
            std::is_nothrow_destructible_v<TCallable>,
            "Dedicated Thread callable bindings require nothrow-destructible callables"
        );

        private:

            TCallable _callable;

        public:

            /// Semantic identity Type of this Dedicated Thread.
            using ThreadIdentity = TThreadIdentity;
            /// Callable Type bound to this Dedicated Thread.
            using Callable = TCallable;


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


            TCallable&& TakeCallable() noexcept {
                return std::move(
                    _callable
                );
            }

    };


    /// Defines the compile-time contract for `BindDedicatedThread`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    template<class TThreadIdentity, class TCallable>
    auto BindDedicatedThread(
        TCallable&& callable
    ) {
        return DedicatedThreadBinding<
            TThreadIdentity,
            std::decay_t<TCallable>
        >(
            std::forward<TCallable>(
                callable
            )
        );
    }


    /// Defines the compile-time contract for `DedicatedThread`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    template<class TThreadIdentity, class... TThreadProperties>
    struct DedicatedThread final {

        static_assert(
            Detail::ValidExecutionResourceProperties<TThreadProperties...>::Value,
            "Execution resource properties must be recognized and contain at most one StackCapacity, Priority and affinity declaration"
        );

        /// Semantic identity Type of this Dedicated Thread.
        using ThreadIdentity = TThreadIdentity;
        /// Resolved execution-property metadata for this declaration.
        using Properties = ResourceProperties<TThreadProperties...>;

    };


    namespace Detail {

        /// Defines the compile-time contract for `IsTaskFacility`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        template<class TResource>
        struct IsTaskFacility {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsTaskFacility`.
        /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
        /// @tparam TRecordCapacity Task-record capacity declaration Type.
        /// @tparam TCallableCapacity Callable-storage capacity declaration Type.
        /// @tparam TResultCapacity Result-storage capacity declaration Type.
        /// @tparam TWorkers Declared Worker set Type.
        template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
        struct IsTaskFacility<TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `ManagedContextCount`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        template<class TResource>
        struct ManagedContextCount {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = 0U;

        };


        /// Defines the compile-time contract for `ManagedContextCount`.
        /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
        /// @tparam TRecordCapacity Task-record capacity declaration Type.
        /// @tparam TCallableCapacity Callable-storage capacity declaration Type.
        /// @tparam TResultCapacity Result-storage capacity declaration Type.
        /// @tparam TWorkers Declared Worker set Type.
        template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
        struct ManagedContextCount<
            TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = TWorkers::Count;

        };


        /// Defines the compile-time contract for `ManagedContextCount`.
        /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
        template<class TTaskIdentity, class... TProperties>
        struct ManagedContextCount<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = 1U;

        };


        /// Defines the compile-time contract for `ManagedContextCount`.
        /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
        template<class TThreadIdentity, class... TProperties>
        struct ManagedContextCount<
            DedicatedThread<TThreadIdentity, TProperties...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr std::size_t Value = 1U;

        };


        /// Defines the compile-time contract for `IsDedicatedWorkerLease`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        template<class TResource>
        struct IsDedicatedWorkerLease {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsDedicatedWorkerLease`.
        /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
        template<class TTaskIdentity, class... TProperties>
        struct IsDedicatedWorkerLease<DedicatedWorkerLease<TTaskIdentity, TProperties...>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `IsDedicatedThread`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        template<class TResource>
        struct IsDedicatedThread {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `IsDedicatedThread`.
        /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
        template<class TThreadIdentity, class... TThreadProperties>
        struct IsDedicatedThread<DedicatedThread<TThreadIdentity, TThreadProperties...>> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `ResourceIdentityConflict`.
        /// @tparam TLeftResource Left topology resource Type in the identity-conflict comparison.
        /// @tparam TRightResource Right topology resource Type in the identity-conflict comparison.
        template<class TLeftResource, class TRightResource>
        struct ResourceIdentityConflict {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `ResourceIdentityConflict`.
        /// @tparam TLeftPoolIdentity Semantic identity Type of the left Task pool.
        /// @tparam TLeftRecordCapacity Record-capacity declaration Type of the left Task facility.
        /// @tparam TLeftCallableCapacity Callable-capacity declaration Type of the left Task facility.
        /// @tparam TLeftResultCapacity Result-capacity declaration Type of the left Task facility.
        /// @tparam TLeftWorkers Worker-set Type of the left Task facility.
        /// @tparam TRightPoolIdentity Semantic identity Type of the right Task pool.
        /// @tparam TRightRecordCapacity Record-capacity declaration Type of the right Task facility.
        /// @tparam TRightCallableCapacity Callable-capacity declaration Type of the right Task facility.
        /// @tparam TRightResultCapacity Result-capacity declaration Type of the right Task facility.
        /// @tparam TRightWorkers Worker-set Type of the right Task facility.
        template<class TLeftPoolIdentity, class TLeftRecordCapacity, class TLeftCallableCapacity, class TLeftResultCapacity, class TLeftWorkers, class TRightPoolIdentity, class TRightRecordCapacity, class TRightCallableCapacity, class TRightResultCapacity, class TRightWorkers>
        struct ResourceIdentityConflict<
            TaskExecutionFacility<TLeftPoolIdentity, TLeftRecordCapacity, TLeftCallableCapacity, TLeftResultCapacity, TLeftWorkers>,
            TaskExecutionFacility<TRightPoolIdentity, TRightRecordCapacity, TRightCallableCapacity, TRightResultCapacity, TRightWorkers>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = std::is_same_v<TLeftPoolIdentity, TRightPoolIdentity>;

        };


        /// Defines the compile-time contract for `ResourceIdentityConflict`.
        /// @tparam TLeftThreadIdentity Semantic identity Type of the left Dedicated Thread.
        /// @tparam TRightThreadIdentity Semantic identity Type of the right Dedicated Thread.
        template<class TLeftThreadIdentity, class... TLeftProperties, class TRightThreadIdentity, class... TRightProperties>
        struct ResourceIdentityConflict<
            DedicatedThread<TLeftThreadIdentity, TLeftProperties...>,
            DedicatedThread<TRightThreadIdentity, TRightProperties...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = std::is_same_v<TLeftThreadIdentity, TRightThreadIdentity>;

        };


        /// Defines the compile-time contract for `ResourceIdentityConflict`.
        /// @tparam TLeftTaskIdentity Semantic identity Type of the left Dedicated Worker task.
        /// @tparam TRightTaskIdentity Semantic identity Type of the right Dedicated Worker task.
        template<class TLeftTaskIdentity, class... TLeftProperties, class TRightTaskIdentity, class... TRightProperties>
        struct ResourceIdentityConflict<
            DedicatedWorkerLease<TLeftTaskIdentity, TLeftProperties...>,
            DedicatedWorkerLease<TRightTaskIdentity, TRightProperties...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = std::is_same_v<TLeftTaskIdentity, TRightTaskIdentity>;

        };


        template<class... TResources>
        struct UniqueResourceIdentities;


        template<>
        struct UniqueResourceIdentities<> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `UniqueResourceIdentities`.
        /// @tparam TFirstResource First topology resource Type in the recursive uniqueness check.
        template<class TFirstResource, class... TRestResources>
        struct UniqueResourceIdentities<TFirstResource, TRestResources...> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                ((!ResourceIdentityConflict<TFirstResource, TRestResources>::Value) && ...) &&
                UniqueResourceIdentities<TRestResources...>::Value;

        };


        /// Defines the compile-time contract for `TopologyProviderBase`.
        /// @tparam THasTaskExecution Whether the topology exposes Task execution capability.
        /// @tparam THasDedicatedThreadExecution Whether the topology exposes Dedicated Thread execution capability.
        template<bool THasTaskExecution, bool THasDedicatedThreadExecution>
        struct TopologyProviderBase;


        template<>
        struct TopologyProviderBase<false, false> {};


        template<>
        struct TopologyProviderBase<true, false> : Framework::Provider<
            Domain,
            Framework::Provides<
                Framework::Offer<TaskExecution>
            >
        > {};


        template<>
        struct TopologyProviderBase<false, true> : Framework::Provider<
            Domain,
            Framework::Provides<
                Framework::Offer<DedicatedThreadExecution>
            >
        > {};


        template<>
        struct TopologyProviderBase<true, true> : Framework::Provider<
            Domain,
            Framework::Provides<
                Framework::Offer<TaskExecution>,
                Framework::Offer<DedicatedThreadExecution>
            >
        > {};

    } // ESPressio::Threading::Detail


    template<class... TResources>
    struct ThreadingTopology final : Detail::TopologyProviderBase<
        (Detail::IsTaskFacility<TResources>::Value || ... || false),
        (Detail::IsDedicatedThread<TResources>::Value || ... || false)
    > {

        static_assert(
            Detail::UniqueResourceIdentities<TResources...>::Value,
            "ThreadingTopology contains duplicate Task Pool, Dedicated Thread, or Dedicated Worker Lease semantic identity"
        );

        /// Number of resources declared by this topology.
        static constexpr std::size_t ResourceCount = sizeof...(TResources);

        /// Whether this topology provides ordinary Task execution.
        static constexpr bool HasTaskExecution =
            (Detail::IsTaskFacility<TResources>::Value || ... || false);

        /// Whether this topology provides Dedicated Thread execution.
        static constexpr bool HasDedicatedThreadExecution =
            (Detail::IsDedicatedThread<TResources>::Value || ... || false);

        /// Whether this topology declares any Dedicated Worker lease.
        static constexpr bool HasDedicatedWorkerLease =
            (Detail::IsDedicatedWorkerLease<TResources>::Value || ... || false);

        /// Number of ordinary Task facilities in this topology.
        static constexpr std::size_t TaskFacilityCount =
            (static_cast<std::size_t>(Detail::IsTaskFacility<TResources>::Value) + ... + 0U);

        /// Number of Dedicated Worker leases in this topology.
        static constexpr std::size_t DedicatedWorkerLeaseCount =
            (static_cast<std::size_t>(Detail::IsDedicatedWorkerLease<TResources>::Value) + ... + 0U);

        /// Number of Dedicated Threads in this topology.
        static constexpr std::size_t DedicatedThreadCount =
            (static_cast<std::size_t>(Detail::IsDedicatedThread<TResources>::Value) + ... + 0U);

        /// Total number of topology-owned managed execution contexts.
        static constexpr std::size_t ManagedExecutionContextCount =
            (Detail::ManagedContextCount<TResources>::Value + ... + 0U);

        /// Tuple Type containing every topology resource declaration.
        using Resources = std::tuple<TResources...>;

    };


    /// Defines the compile-time contract for `TaskPoolRequirement`.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    template<class TPoolIdentity, class... TConstraints>
    struct TaskPoolRequirement final {

        /// Semantic identity Type of this Task pool.
        using PoolIdentity = TPoolIdentity;

    };


    /// Defines the compile-time contract for `DedicatedThreadRequirement`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    template<class TThreadIdentity, class... TConstraints>
    struct DedicatedThreadRequirement final {

        /// Semantic identity Type of this Dedicated Thread.
        using ThreadIdentity = TThreadIdentity;

    };


    /// Defines the compile-time contract for `DedicatedWorkerRequirement`.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    template<class TTaskIdentity, class... TConstraints>
    struct DedicatedWorkerRequirement final {

        /// Semantic identity Type of this Dedicated Worker task.
        using TaskIdentity = TTaskIdentity;

    };


    namespace Detail {

        /// Defines the compile-time contract for `MatchesTaskPoolIdentity`.
        /// @tparam TIdentity Compile-time template parameter `TIdentity` used by this specialization.
        /// @tparam TResource Topology resource Type being classified or inspected.
        template<class TIdentity, class TResource>
        struct MatchesTaskPoolIdentity {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `MatchesTaskPoolIdentity`.
        /// @tparam TIdentity Compile-time template parameter `TIdentity` used by this specialization.
        /// @tparam TRecordCapacity Task-record capacity declaration Type.
        /// @tparam TCallableCapacity Callable-storage capacity declaration Type.
        /// @tparam TResultCapacity Result-storage capacity declaration Type.
        /// @tparam TWorkers Declared Worker set Type.
        template<class TIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
        struct MatchesTaskPoolIdentity<
            TIdentity,
            TaskExecutionFacility<TIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `MatchesDedicatedThreadIdentity`.
        /// @tparam TIdentity Compile-time template parameter `TIdentity` used by this specialization.
        /// @tparam TResource Topology resource Type being classified or inspected.
        template<class TIdentity, class TResource>
        struct MatchesDedicatedThreadIdentity {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `MatchesDedicatedThreadIdentity`.
        /// @tparam TIdentity Compile-time template parameter `TIdentity` used by this specialization.
        template<class TIdentity, class... TProperties>
        struct MatchesDedicatedThreadIdentity<
            TIdentity,
            DedicatedThread<TIdentity, TProperties...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `MatchesDedicatedWorkerIdentity`.
        /// @tparam TIdentity Compile-time template parameter `TIdentity` used by this specialization.
        /// @tparam TResource Topology resource Type being classified or inspected.
        template<class TIdentity, class TResource>
        struct MatchesDedicatedWorkerIdentity {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `MatchesDedicatedWorkerIdentity`.
        /// @tparam TIdentity Compile-time template parameter `TIdentity` used by this specialization.
        template<class TIdentity, class... TProperties>
        struct MatchesDedicatedWorkerIdentity<
            TIdentity,
            DedicatedWorkerLease<TIdentity, TProperties...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = true;

        };


        /// Defines the compile-time contract for `RequirementSatisfied`.
        /// @tparam TTopology Compile-time Threading topology being realized or inspected.
        /// @tparam TRequirement Compile-time Threading requirement Type being evaluated.
        template<class TTopology, class TRequirement>
        struct RequirementSatisfied;


        /// Defines the compile-time contract for `RequirementSatisfied`.
        /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
        template<class... TResources, class TPoolIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
            TaskPoolRequirement<TPoolIdentity, TConstraints...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                (MatchesTaskPoolIdentity<TPoolIdentity, TResources>::Value || ... || false);

        };


        /// Defines the compile-time contract for `RequirementSatisfied`.
        /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
        template<class... TResources, class TThreadIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
            DedicatedThreadRequirement<TThreadIdentity, TConstraints...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                (MatchesDedicatedThreadIdentity<TThreadIdentity, TResources>::Value || ... || false);

        };


        /// Defines the compile-time contract for `RequirementSatisfied`.
        /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
        template<class... TResources, class TTaskIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
            DedicatedWorkerRequirement<TTaskIdentity, TConstraints...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                (MatchesDedicatedWorkerIdentity<TTaskIdentity, TResources>::Value || ... || false);

        };

    } // ESPressio::Threading::Detail


    /// Defines the compile-time contract for `MinimumStackCapacity`.
    /// @tparam TBytes Compile-time byte requirement.
    template<std::size_t TBytes>
    struct MinimumStackCapacity final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr std::size_t Value = TBytes;

    };


    /// Defines the compile-time contract for `MinimumWorkerConcurrency`.
    /// @tparam TCount Compile-time count requirement.
    template<std::size_t TCount>
    struct MinimumWorkerConcurrency final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr std::size_t Value = TCount;

    };


    /// Defines the compile-time contract for `MinimumPriority`.
    /// @tparam TPriority Compile-time logical execution priority.
    template<ThreadPriority TPriority>
    struct MinimumPriority final {

        /// Compile-time result produced by this trait or property specialization.
        static constexpr ThreadPriority Value = TPriority;

    };


    /// Defines the compile-time contract for `RequiredAffinity`.
    /// @tparam TAffinity Affinity requirement Type.
    template<class TAffinity>
    struct RequiredAffinity final {

        /// Affinity Type required by this compile-time constraint.
        using AffinityType = TAffinity;

    };


    namespace Detail {

        /// Defines the compile-time contract for `HasMinimumStackCapacity`.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<std::size_t TMinimum, class... TProperties>
        struct HasMinimumStackCapacity;


        /// Defines the compile-time contract for `HasMinimumStackCapacity`.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<std::size_t TMinimum>
        struct HasMinimumStackCapacity<TMinimum> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `HasMinimumStackCapacity`.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
        template<std::size_t TMinimum, std::size_t TCapacity, class... TRest>
        struct HasMinimumStackCapacity<
            TMinimum,
            StackCapacity<TCapacity>,
            TRest...
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = TCapacity >= TMinimum;

        };


        /// Defines the compile-time contract for `HasMinimumStackCapacity`.
        /// @tparam TFirst First property Type in the recursive compile-time search.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<std::size_t TMinimum, class TFirst, class... TRest>
        struct HasMinimumStackCapacity<
            TMinimum,
            TFirst,
            TRest...
        > : HasMinimumStackCapacity<TMinimum, TRest...> {};


        /// Defines the compile-time contract for `HasMinimumPriority`.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<ThreadPriority TMinimum, class... TProperties>
        struct HasMinimumPriority;


        /// Defines the compile-time contract for `HasMinimumPriority`.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<ThreadPriority TMinimum>
        struct HasMinimumPriority<TMinimum> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `HasMinimumPriority`.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        /// @tparam TValue Compile-time value being compared with the minimum.
        template<ThreadPriority TMinimum, ThreadPriority TValue, class... TRest>
        struct HasMinimumPriority<
            TMinimum,
            Priority<TValue>,
            TRest...
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                static_cast<std::uint8_t>(TValue) >=
                static_cast<std::uint8_t>(TMinimum);

        };


        /// Defines the compile-time contract for `HasMinimumPriority`.
        /// @tparam TFirst First property Type in the recursive compile-time search.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<ThreadPriority TMinimum, class TFirst, class... TRest>
        struct HasMinimumPriority<
            TMinimum,
            TFirst,
            TRest...
        > : HasMinimumPriority<TMinimum, TRest...> {};


        /// Defines the compile-time contract for `HasRequiredAffinity`.
        /// @tparam TRequiredAffinity Affinity Type required by the consumer constraint.
        template<class TRequiredAffinity, class... TProperties>
        struct HasRequiredAffinity;


        /// Defines the compile-time contract for `HasRequiredAffinity`.
        /// @tparam TRequiredAffinity Affinity Type required by the consumer constraint.
        template<class TRequiredAffinity>
        struct HasRequiredAffinity<TRequiredAffinity> {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `HasRequiredAffinity`.
        /// @tparam TRequiredAffinity Affinity Type required by the consumer constraint.
        /// @tparam TFirst First property Type in the recursive compile-time search.
        template<class TRequiredAffinity, class TFirst, class... TRest>
        struct HasRequiredAffinity<
            TRequiredAffinity,
            TFirst,
            TRest...
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                std::is_same_v<TRequiredAffinity, TFirst> ||
                HasRequiredAffinity<TRequiredAffinity, TRest...>::Value;

        };


        /// Defines the compile-time contract for `WorkerConstraintSatisfied`.
        /// @tparam TWorker Worker declaration Type being validated.
        /// @tparam TConstraint Constraint Type being evaluated.
        template<class TWorker, class TConstraint>
        struct WorkerConstraintSatisfied {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `WorkerConstraintSatisfied`.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<class... TProperties, std::size_t TMinimum>
        struct WorkerConstraintSatisfied<
            Worker<TProperties...>,
            MinimumStackCapacity<TMinimum>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasMinimumStackCapacity<TMinimum, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `WorkerConstraintSatisfied`.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<class... TProperties, ThreadPriority TMinimum>
        struct WorkerConstraintSatisfied<
            Worker<TProperties...>,
            MinimumPriority<TMinimum>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasMinimumPriority<TMinimum, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `WorkerConstraintSatisfied`.
        /// @tparam TAffinity Affinity requirement Type.
        template<class... TProperties, class TAffinity>
        struct WorkerConstraintSatisfied<
            Worker<TProperties...>,
            RequiredAffinity<TAffinity>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasRequiredAffinity<TAffinity, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `EveryWorkerSatisfies`.
        /// @tparam TWorkers Declared Worker set Type.
        /// @tparam TConstraint Constraint Type being evaluated.
        template<class TWorkers, class TConstraint>
        struct EveryWorkerSatisfies;


        /// Defines the compile-time contract for `EveryWorkerSatisfies`.
        /// @tparam TConstraint Constraint Type being evaluated.
        template<class... TWorkers, class TConstraint>
        struct EveryWorkerSatisfies<
            Workers<TWorkers...>,
            TConstraint
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                (WorkerConstraintSatisfied<TWorkers, TConstraint>::Value && ... && true);

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        /// @tparam TConstraint Constraint Type being evaluated.
        template<class TResource, class TConstraint>
        struct ResourceConstraintSatisfied {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
        /// @tparam TRecordCapacity Task-record capacity declaration Type.
        /// @tparam TCallableCapacity Callable-storage capacity declaration Type.
        /// @tparam TResultCapacity Result-storage capacity declaration Type.
        /// @tparam TWorkers Declared Worker set Type.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers, std::size_t TMinimum>
        struct ResourceConstraintSatisfied<
            TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>,
            MinimumWorkerConcurrency<TMinimum>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = TWorkers::Count >= TMinimum;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
        /// @tparam TRecordCapacity Task-record capacity declaration Type.
        /// @tparam TCallableCapacity Callable-storage capacity declaration Type.
        /// @tparam TResultCapacity Result-storage capacity declaration Type.
        /// @tparam TWorkers Declared Worker set Type.
        /// @tparam TConstraint Constraint Type being evaluated.
        template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers, class TConstraint>
        struct ResourceConstraintSatisfied<
            TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>,
            TConstraint
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                EveryWorkerSatisfies<TWorkers, TConstraint>::Value;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<class TTaskIdentity, class... TProperties, std::size_t TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>,
            MinimumWorkerConcurrency<TMinimum>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = 1U >= TMinimum;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<class TTaskIdentity, class... TProperties, std::size_t TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>,
            MinimumStackCapacity<TMinimum>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasMinimumStackCapacity<TMinimum, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<class TTaskIdentity, class... TProperties, ThreadPriority TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>,
            MinimumPriority<TMinimum>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasMinimumPriority<TMinimum, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
        /// @tparam TAffinity Affinity requirement Type.
        template<class TTaskIdentity, class... TProperties, class TAffinity>
        struct ResourceConstraintSatisfied<
            DedicatedWorkerLease<TTaskIdentity, TProperties...>,
            RequiredAffinity<TAffinity>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasRequiredAffinity<TAffinity, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<class TThreadIdentity, class... TProperties, std::size_t TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedThread<TThreadIdentity, TProperties...>,
            MinimumStackCapacity<TMinimum>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasMinimumStackCapacity<TMinimum, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
        /// @tparam TMinimum Compile-time minimum value required by the constraint.
        template<class TThreadIdentity, class... TProperties, ThreadPriority TMinimum>
        struct ResourceConstraintSatisfied<
            DedicatedThread<TThreadIdentity, TProperties...>,
            MinimumPriority<TMinimum>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasMinimumPriority<TMinimum, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `ResourceConstraintSatisfied`.
        /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
        /// @tparam TAffinity Affinity requirement Type.
        template<class TThreadIdentity, class... TProperties, class TAffinity>
        struct ResourceConstraintSatisfied<
            DedicatedThread<TThreadIdentity, TProperties...>,
            RequiredAffinity<TAffinity>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                HasRequiredAffinity<TAffinity, TProperties...>::Value;

        };


        /// Defines the compile-time contract for `ResourceSatisfiesRequirement`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        /// @tparam TRequirement Compile-time Threading requirement Type being evaluated.
        template<class TResource, class TRequirement>
        struct ResourceSatisfiesRequirement {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value = false;

        };


        /// Defines the compile-time contract for `ResourceSatisfiesRequirement`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
        template<class TResource, class TPoolIdentity, class... TConstraints>
        struct ResourceSatisfiesRequirement<
            TResource,
            TaskPoolRequirement<TPoolIdentity, TConstraints...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                MatchesTaskPoolIdentity<TPoolIdentity, TResource>::Value &&
                (ResourceConstraintSatisfied<TResource, TConstraints>::Value && ... && true);

        };


        /// Defines the compile-time contract for `ResourceSatisfiesRequirement`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
        template<class TResource, class TThreadIdentity, class... TConstraints>
        struct ResourceSatisfiesRequirement<
            TResource,
            DedicatedThreadRequirement<TThreadIdentity, TConstraints...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                MatchesDedicatedThreadIdentity<TThreadIdentity, TResource>::Value &&
                (ResourceConstraintSatisfied<TResource, TConstraints>::Value && ... && true);

        };


        /// Defines the compile-time contract for `ResourceSatisfiesRequirement`.
        /// @tparam TResource Topology resource Type being classified or inspected.
        /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
        template<class TResource, class TTaskIdentity, class... TConstraints>
        struct ResourceSatisfiesRequirement<
            TResource,
            DedicatedWorkerRequirement<TTaskIdentity, TConstraints...>
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                MatchesDedicatedWorkerIdentity<TTaskIdentity, TResource>::Value &&
                (ResourceConstraintSatisfied<TResource, TConstraints>::Value && ... && true);

        };


        /// Defines the compile-time contract for `ConstrainedRequirementSatisfied`.
        /// @tparam TTopology Compile-time Threading topology being realized or inspected.
        /// @tparam TRequirement Compile-time Threading requirement Type being evaluated.
        template<class TTopology, class TRequirement>
        struct ConstrainedRequirementSatisfied;


        /// Defines the compile-time contract for `ConstrainedRequirementSatisfied`.
        /// @tparam TRequirement Compile-time Threading requirement Type being evaluated.
        template<class... TResources, class TRequirement>
        struct ConstrainedRequirementSatisfied<
            ThreadingTopology<TResources...>,
            TRequirement
        > {

            /// Compile-time result produced by this trait or property specialization.
            static constexpr bool Value =
                (ResourceSatisfiesRequirement<TResources, TRequirement>::Value || ... || false);

        };

    } // ESPressio::Threading::Detail


    /// Defines the compile-time contract for `templated declaration`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TRequirement Compile-time Threading requirement Type being evaluated.
    template<class TTopology, class TRequirement>
    inline constexpr bool SatisfiesThreadingRequirement =
        Detail::ConstrainedRequirementSatisfied<
            TTopology,
            TRequirement
        >::Value;

} // ESPressio::Threading
