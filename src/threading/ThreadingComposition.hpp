#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <ESPressio_System.hpp>

#include "ThreadingTypes.hpp"

namespace ESPressio::Threading {

    namespace Framework = ESPressio::System::CompositionFramework;

    struct Domain final : Framework::Domain {};


    struct TaskExecution final : Framework::SharedCapability<Domain> {};


    struct DedicatedThreadExecution final : Framework::SharedCapability<Domain> {};


    struct StackCapacityBytes final : Framework::Property<TaskExecution, std::size_t> {};


    struct WorkerConcurrency final : Framework::Property<TaskExecution, std::size_t> {};


    template<class TPoolIdentity>
    struct TaskPool final {};


    template<std::size_t TCapacity>
    struct TaskRecordCapacity final {

        static constexpr std::size_t Value = TCapacity;

    };


    template<std::size_t TCapacity>
    struct CallableCapacity final {

        static constexpr std::size_t Value = TCapacity;

    };


    template<std::size_t TCapacity>
    struct ResultCapacity final {

        static constexpr std::size_t Value = TCapacity;

    };


    template<std::size_t TCapacity>
    struct StackCapacity final {

        static constexpr std::size_t Value = TCapacity;

    };


    template<ThreadPriority TPriority>
    struct Priority final {

        static constexpr ThreadPriority Value = TPriority;

    };


    template<std::uint32_t TProcessorIndex>
    struct Affinity final {

        static constexpr ProcessorAffinity Value = ProcessorAffinity::Specific(TProcessorIndex);

    };


    struct AnyAffinity final {

        static constexpr ProcessorAffinity Value = ProcessorAffinity::Any();

    };


    template<class... TProperties>
    struct ResourceProperties final {

        static constexpr std::size_t Count = sizeof...(TProperties);

    };


    template<class... TWorkerProperties>
    struct Worker final {

        using Properties = ResourceProperties<TWorkerProperties...>;

    };


    template<class... TWorkers>
    struct Workers final {

        static constexpr std::size_t Count = sizeof...(TWorkers);

    };


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

        using PoolIdentity = TPoolIdentity;
        using RecordCapacity = TRecordCapacity;
        using CallableStorageCapacity = TCallableCapacity;
        using ResultStorageCapacity = TResultCapacity;
        using WorkerSet = TWorkers;

    };


    template<class TTaskIdentity, class... TWorkerProperties>
    struct DedicatedWorkerLease final {

        using TaskIdentity = TTaskIdentity;
        using Properties = ResourceProperties<TWorkerProperties...>;

        static constexpr std::size_t WorkerCount = 1U;

    };


    template<class TThreadIdentity, class... TThreadProperties>
    struct DedicatedThread final {

        using ThreadIdentity = TThreadIdentity;
        using Properties = ResourceProperties<TThreadProperties...>;

    };


    namespace Detail {

        template<class TResource>
        struct IsTaskFacility {

            static constexpr bool Value = false;

        };


        template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
        struct IsTaskFacility<TaskExecutionFacility<TPoolIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>> {

            static constexpr bool Value = true;

        };


        template<class TResource>
        struct IsDedicatedWorkerLease {

            static constexpr bool Value = false;

        };


        template<class TTaskIdentity, class... TProperties>
        struct IsDedicatedWorkerLease<DedicatedWorkerLease<TTaskIdentity, TProperties...>> {

            static constexpr bool Value = true;

        };


        template<class TResource>
        struct IsDedicatedThread {

            static constexpr bool Value = false;

        };


        template<class TThreadIdentity, class... TThreadProperties>
        struct IsDedicatedThread<DedicatedThread<TThreadIdentity, TThreadProperties...>> {

            static constexpr bool Value = true;

        };


        template<class TLeftResource, class TRightResource>
        struct ResourceIdentityConflict {

            static constexpr bool Value = false;

        };


        template<class TLeftPoolIdentity, class TLeftRecordCapacity, class TLeftCallableCapacity, class TLeftResultCapacity, class TLeftWorkers, class TRightPoolIdentity, class TRightRecordCapacity, class TRightCallableCapacity, class TRightResultCapacity, class TRightWorkers>
        struct ResourceIdentityConflict<
            TaskExecutionFacility<TLeftPoolIdentity, TLeftRecordCapacity, TLeftCallableCapacity, TLeftResultCapacity, TLeftWorkers>,
            TaskExecutionFacility<TRightPoolIdentity, TRightRecordCapacity, TRightCallableCapacity, TRightResultCapacity, TRightWorkers>
        > {

            static constexpr bool Value = std::is_same_v<TLeftPoolIdentity, TRightPoolIdentity>;

        };


        template<class TLeftThreadIdentity, class... TLeftProperties, class TRightThreadIdentity, class... TRightProperties>
        struct ResourceIdentityConflict<
            DedicatedThread<TLeftThreadIdentity, TLeftProperties...>,
            DedicatedThread<TRightThreadIdentity, TRightProperties...>
        > {

            static constexpr bool Value = std::is_same_v<TLeftThreadIdentity, TRightThreadIdentity>;

        };


        template<class TLeftTaskIdentity, class... TLeftProperties, class TRightTaskIdentity, class... TRightProperties>
        struct ResourceIdentityConflict<
            DedicatedWorkerLease<TLeftTaskIdentity, TLeftProperties...>,
            DedicatedWorkerLease<TRightTaskIdentity, TRightProperties...>
        > {

            static constexpr bool Value = std::is_same_v<TLeftTaskIdentity, TRightTaskIdentity>;

        };


        template<class... TResources>
        struct UniqueResourceIdentities;


        template<>
        struct UniqueResourceIdentities<> {

            static constexpr bool Value = true;

        };


        template<class TFirstResource, class... TRestResources>
        struct UniqueResourceIdentities<TFirstResource, TRestResources...> {

            static constexpr bool Value =
                ((!ResourceIdentityConflict<TFirstResource, TRestResources>::Value) && ...) &&
                UniqueResourceIdentities<TRestResources...>::Value;

        };


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

        static constexpr std::size_t ResourceCount = sizeof...(TResources);

        static constexpr bool HasTaskExecution =
            (Detail::IsTaskFacility<TResources>::Value || ... || false);

        static constexpr bool HasDedicatedThreadExecution =
            (Detail::IsDedicatedThread<TResources>::Value || ... || false);

        static constexpr bool HasDedicatedWorkerLease =
            (Detail::IsDedicatedWorkerLease<TResources>::Value || ... || false);

        static constexpr std::size_t TaskFacilityCount =
            (static_cast<std::size_t>(Detail::IsTaskFacility<TResources>::Value) + ... + 0U);

        static constexpr std::size_t DedicatedWorkerLeaseCount =
            (static_cast<std::size_t>(Detail::IsDedicatedWorkerLease<TResources>::Value) + ... + 0U);

        static constexpr std::size_t DedicatedThreadCount =
            (static_cast<std::size_t>(Detail::IsDedicatedThread<TResources>::Value) + ... + 0U);

    };


    template<class TPoolIdentity, class... TConstraints>
    struct TaskPoolRequirement final {

        using PoolIdentity = TPoolIdentity;

    };


    template<class TThreadIdentity, class... TConstraints>
    struct DedicatedThreadRequirement final {

        using ThreadIdentity = TThreadIdentity;

    };


    template<class TTaskIdentity, class... TConstraints>
    struct DedicatedWorkerRequirement final {

        using TaskIdentity = TTaskIdentity;

    };


    namespace Detail {

        template<class TIdentity, class TResource>
        struct MatchesTaskPoolIdentity {

            static constexpr bool Value = false;

        };


        template<class TIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
        struct MatchesTaskPoolIdentity<
            TIdentity,
            TaskExecutionFacility<TIdentity, TRecordCapacity, TCallableCapacity, TResultCapacity, TWorkers>
        > {

            static constexpr bool Value = true;

        };


        template<class TIdentity, class TResource>
        struct MatchesDedicatedThreadIdentity {

            static constexpr bool Value = false;

        };


        template<class TIdentity, class... TProperties>
        struct MatchesDedicatedThreadIdentity<
            TIdentity,
            DedicatedThread<TIdentity, TProperties...>
        > {

            static constexpr bool Value = true;

        };


        template<class TIdentity, class TResource>
        struct MatchesDedicatedWorkerIdentity {

            static constexpr bool Value = false;

        };


        template<class TIdentity, class... TProperties>
        struct MatchesDedicatedWorkerIdentity<
            TIdentity,
            DedicatedWorkerLease<TIdentity, TProperties...>
        > {

            static constexpr bool Value = true;

        };


        template<class TTopology, class TRequirement>
        struct RequirementSatisfied;


        template<class... TResources, class TPoolIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
            TaskPoolRequirement<TPoolIdentity, TConstraints...>
        > {

            static constexpr bool Value =
                (MatchesTaskPoolIdentity<TPoolIdentity, TResources>::Value || ... || false);

        };


        template<class... TResources, class TThreadIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
            DedicatedThreadRequirement<TThreadIdentity, TConstraints...>
        > {

            static constexpr bool Value =
                (MatchesDedicatedThreadIdentity<TThreadIdentity, TResources>::Value || ... || false);

        };


        template<class... TResources, class TTaskIdentity, class... TConstraints>
        struct RequirementSatisfied<
            ThreadingTopology<TResources...>,
            DedicatedWorkerRequirement<TTaskIdentity, TConstraints...>
        > {

            static constexpr bool Value =
                (MatchesDedicatedWorkerIdentity<TTaskIdentity, TResources>::Value || ... || false);

        };

    } // ESPressio::Threading::Detail


    template<class TTopology, class TRequirement>
    inline constexpr bool SatisfiesThreadingRequirement =
        Detail::RequirementSatisfied<
            TTopology,
            TRequirement
        >::Value;


    template<std::size_t TBytes>
    struct MinimumStackCapacity final {

        static constexpr std::size_t Value = TBytes;

    };


    template<std::size_t TCount>
    struct MinimumWorkerConcurrency final {

        static constexpr std::size_t Value = TCount;

    };


    template<ThreadPriority TPriority>
    struct MinimumPriority final {

        static constexpr ThreadPriority Value = TPriority;

    };


    template<class TAffinity>
    struct RequiredAffinity final {

        using AffinityType = TAffinity;

    };

} // ESPressio::Threading
