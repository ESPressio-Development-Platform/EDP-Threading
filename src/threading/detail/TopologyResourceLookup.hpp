#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../ThreadingComposition.hpp"

namespace ESPressio::Threading::Detail {

    inline constexpr std::size_t TopologyResourceNotFound =
        static_cast<std::size_t>(-1);


    template<class TResource, class TPoolIdentity>
    struct IsTaskFacilityIdentity {

        static constexpr bool Value = false;

    };


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

        static constexpr bool Value = true;

    };


    template<class TResource, class TTaskIdentity>
    struct IsDedicatedWorkerIdentity {

        static constexpr bool Value = false;

    };


    template<class TTaskIdentity, class... TProperties>
    struct IsDedicatedWorkerIdentity<
        DedicatedWorkerLease<
            TTaskIdentity,
            TProperties...
        >,
        TTaskIdentity
    > {

        static constexpr bool Value = true;

    };


    template<class TResource, class TThreadIdentity>
    struct IsDedicatedThreadIdentity {

        static constexpr bool Value = false;

    };


    template<class TThreadIdentity, class... TProperties>
    struct IsDedicatedThreadIdentity<
        DedicatedThread<
            TThreadIdentity,
            TProperties...
        >,
        TThreadIdentity
    > {

        static constexpr bool Value = true;

    };


    template<template<class, class> class TMatcher, class TIdentity, class TResources, std::size_t TIndex = 0U>
    struct TopologyResourceIndex;


    template<template<class, class> class TMatcher, class TIdentity, std::size_t TIndex>
    struct TopologyResourceIndex<
        TMatcher,
        TIdentity,
        std::tuple<>,
        TIndex
    > {

        static constexpr std::size_t Value = TopologyResourceNotFound;

    };


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

        static constexpr std::size_t Tail = TopologyResourceIndex<
            TMatcher,
            TIdentity,
            std::tuple<TRestResources...>,
            TIndex + 1U
        >::Value;

        static constexpr std::size_t Value =
            TMatcher<TFirstResource, TIdentity>::Value
                ? TIndex
                : Tail;

    };


    template<class TTopology, class TPoolIdentity>
    inline constexpr std::size_t TaskFacilityResourceIndex =
        TopologyResourceIndex<
            IsTaskFacilityIdentity,
            TPoolIdentity,
            typename TTopology::Resources
        >::Value;


    template<class TTopology, class TTaskIdentity>
    inline constexpr std::size_t DedicatedWorkerResourceIndex =
        TopologyResourceIndex<
            IsDedicatedWorkerIdentity,
            TTaskIdentity,
            typename TTopology::Resources
        >::Value;


    template<class TTopology, class TThreadIdentity>
    inline constexpr std::size_t DedicatedThreadResourceIndex =
        TopologyResourceIndex<
            IsDedicatedThreadIdentity,
            TThreadIdentity,
            typename TTopology::Resources
        >::Value;

} // ESPressio::Threading::Detail
