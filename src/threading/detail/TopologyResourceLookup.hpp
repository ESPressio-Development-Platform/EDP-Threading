#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../ThreadingComposition.hpp"

namespace ESPressio::Threading::Detail {

    inline constexpr std::size_t TopologyResourceNotFound =
        static_cast<std::size_t>(-1);


    /// Defines the compile-time contract for `IsTaskFacilityIdentity`.
    /// @tparam TResource Topology resource Type being classified.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    template<class TResource, class TPoolIdentity>
    struct IsTaskFacilityIdentity {

        /// Compile-time result or topology index produced by this trait/specialization.
        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsTaskFacilityIdentity`.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    /// @tparam TRecordCapacity Task-record capacity declaration Type.
    /// @tparam TCallableCapacity Callable-storage capacity declaration Type.
    /// @tparam TResultCapacity Result-storage capacity declaration Type.
    /// @tparam TWorkers Declared Worker set Type.
    template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
    struct IsTaskFacilityIdentity<
        TaskExecutionFacility<
            TPoolIdentity,
            TRecordCapacity,
            TCallableCapacity,
            TResultCapacity,
            TWorkers
        >,
        TPoolIdentity
    > {

        /// Compile-time result or topology index produced by this trait/specialization.
        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `IsDedicatedWorkerIdentity`.
    /// @tparam TResource Topology resource Type being classified.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    template<class TResource, class TTaskIdentity>
    struct IsDedicatedWorkerIdentity {

        /// Compile-time result or topology index produced by this trait/specialization.
        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsDedicatedWorkerIdentity`.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    /// Matches one Dedicated Worker lease declaration against a requested Task identity.
    /// @tparam TTaskIdentity Semantic Task identity Type carried by the Dedicated Worker lease.
    /// @tparam TProperties Compile-time capacity/execution-property Types carried by the lease.
    template<class TTaskIdentity, class... TProperties>
    struct IsDedicatedWorkerIdentity<
        DedicatedWorkerLease<
            TTaskIdentity,
            TProperties...
        >,
        TTaskIdentity
    > {

        /// Compile-time result or topology index produced by this trait/specialization.
        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `IsDedicatedThreadIdentity`.
    /// @tparam TResource Topology resource Type being classified.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    template<class TResource, class TThreadIdentity>
    struct IsDedicatedThreadIdentity {

        /// Compile-time result or topology index produced by this trait/specialization.
        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsDedicatedThreadIdentity`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// Matches one Dedicated Thread declaration against a requested Thread identity.
    /// @tparam TThreadIdentity Semantic identity Type carried by the Dedicated Thread declaration.
    /// @tparam TProperties Compile-time execution-property Types carried by the declaration.
    template<class TThreadIdentity, class... TProperties>
    struct IsDedicatedThreadIdentity<
        DedicatedThread<
            TThreadIdentity,
            TProperties...
        >,
        TThreadIdentity
    > {

        /// Compile-time result or topology index produced by this trait/specialization.
        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `TopologyResourceIndex`.
    /// @tparam TMatcher Compile-time identity matcher template.
    /// @tparam TIdentity Semantic identity Type being located.
    /// @tparam TResources Remaining topology resource pack searched recursively.
    /// @tparam TIndex Compile-time topology resource index.
    template<template<class, class> class TMatcher, class TIdentity, class TResources, std::size_t TIndex = 0U>
    struct TopologyResourceIndex;


    /// Defines the compile-time contract for `TopologyResourceIndex`.
    /// @tparam TMatcher Compile-time identity matcher template.
    /// @tparam TIdentity Semantic identity Type being located.
    /// @tparam TIndex Compile-time topology resource index.
    template<template<class, class> class TMatcher, class TIdentity, std::size_t TIndex>
    struct TopologyResourceIndex<
        TMatcher,
        TIdentity,
        std::tuple<>,
        TIndex
    > {

        /// Compile-time result or topology index produced by this trait/specialization.
        static constexpr std::size_t Value = TopologyResourceNotFound;

    };


    /// Defines the compile-time contract for `TopologyResourceIndex`.
    /// @tparam TMatcher Compile-time identity matcher template.
    /// @tparam TIdentity Semantic identity Type being located.
    /// @tparam TFirstResource First topology resource Type in the recursive lookup.
    /// @tparam TIndex Compile-time topology resource index.
    template<template<class, class> class TMatcher, class TIdentity, class TFirstResource, class... TRestResources, std::size_t TIndex>
    struct TopologyResourceIndex<
        TMatcher,
        TIdentity,
        std::tuple<
            TFirstResource,
            TRestResources...
        >,
        TIndex
    > {

        /// Recursive lookup result for the remaining topology resource pack.
        static constexpr std::size_t Tail = TopologyResourceIndex<
            TMatcher,
            TIdentity,
            std::tuple<TRestResources...>,
            TIndex + 1U
        >::Value;

        /// Compile-time result or topology index produced by this trait/specialization.
        static constexpr std::size_t Value =
            TMatcher<TFirstResource, TIdentity>::Value
                ? TIndex
                : Tail;

    };


    /// Defines the compile-time contract for `templated declaration`.
    /// @tparam TTopology Compile-time Threading topology being searched.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    template<class TTopology, class TPoolIdentity>
    inline constexpr std::size_t TaskFacilityResourceIndex =
        TopologyResourceIndex<
            IsTaskFacilityIdentity,
            TPoolIdentity,
            typename TTopology::Resources
        >::Value;


    /// Defines the compile-time contract for `templated declaration`.
    /// @tparam TTopology Compile-time Threading topology being searched.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    template<class TTopology, class TTaskIdentity>
    inline constexpr std::size_t DedicatedWorkerResourceIndex =
        TopologyResourceIndex<
            IsDedicatedWorkerIdentity,
            TTaskIdentity,
            typename TTopology::Resources
        >::Value;


    /// Defines the compile-time contract for `templated declaration`.
    /// @tparam TTopology Compile-time Threading topology being searched.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    template<class TTopology, class TThreadIdentity>
    inline constexpr std::size_t DedicatedThreadResourceIndex =
        TopologyResourceIndex<
            IsDedicatedThreadIdentity,
            TThreadIdentity,
            typename TTopology::Resources
        >::Value;

} // ESPressio::Threading::Detail
