#pragma once

#include <cstddef>
#include <cstdint>

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


    template<class... TWorkerProperties>
    struct Worker final {};


    template<class... TWorkers>
    struct Workers final {

        static constexpr std::size_t Count = sizeof...(TWorkers);

    };


    template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
    struct TaskExecutionFacility final {

        using PoolIdentity = TPoolIdentity;
        using RecordCapacity = TRecordCapacity;
        using CallableStorageCapacity = TCallableCapacity;
        using ResultStorageCapacity = TResultCapacity;
        using WorkerSet = TWorkers;

    };


    template<class TTaskIdentity, class... TWorkerProperties>
    struct DedicatedWorkerLease final {

        using TaskIdentity = TTaskIdentity;

    };


    template<class TThreadIdentity, class... TThreadProperties>
    struct DedicatedThread final {

        using ThreadIdentity = TThreadIdentity;

    };


    template<class... TResources>
    struct ThreadingTopology final {

        static constexpr std::size_t ResourceCount = sizeof...(TResources);

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
