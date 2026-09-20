#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../ThreadingComposition.hpp"

namespace ESPressio::Threading::Detail {

    template<class TTopology, std::size_t TResourceIndex>
    struct TopologyContextOffset {

        private:

            template<std::size_t... TIndices>
            static constexpr std::size_t Sum(
                std::index_sequence<TIndices...>
            ) noexcept {
                return (
                    ManagedContextCount<
                        std::tuple_element_t<TIndices, typename TTopology::Resources>
                    >::Value +
                    ... +
                    0U
                );
            }

        public:

            static_assert(
                TResourceIndex <= TTopology::ResourceCount,
                "Topology resource index exceeds the statically declared resource count"
            );

            static constexpr std::size_t Value = Sum(
                std::make_index_sequence<TResourceIndex>{}
            );

    };


    template<class TTopology, std::size_t TResourceIndex>
    struct TopologyResourceDescriptor final {

        static_assert(
            TResourceIndex < TTopology::ResourceCount,
            "Topology resource descriptor index exceeds the statically declared resource count"
        );

        using Resource = std::tuple_element_t<
            TResourceIndex,
            typename TTopology::Resources
        >;

        static constexpr std::size_t FirstContextIndex =
            TopologyContextOffset<TTopology, TResourceIndex>::Value;

        static constexpr std::size_t ContextCount =
            ManagedContextCount<Resource>::Value;

        static constexpr std::size_t EndContextIndex =
            FirstContextIndex + ContextCount;

        static_assert(
            EndContextIndex <= TTopology::ManagedExecutionContextCount,
            "Topology resource context range exceeds the topology-wide managed-context capacity"
        );

    };


    template<class TWorkers, std::size_t TWorkerIndex>
    struct WorkerDescriptor;


    template<class... TWorkers, std::size_t TWorkerIndex>
    struct WorkerDescriptor<
        Workers<TWorkers...>,
        TWorkerIndex
    > final {

        static_assert(
            TWorkerIndex < sizeof...(TWorkers),
            "Worker descriptor index exceeds the statically declared Worker count"
        );

        using WorkerType = std::tuple_element_t<
            TWorkerIndex,
            std::tuple<TWorkers...>
        >;

        using Properties = typename WorkerType::Properties;

    };


    template<class TTopology, std::size_t TResourceIndex, std::size_t TWorkerIndex>
    struct FacilityWorkerDescriptor final {

        private:

            using ResourceDescriptor = TopologyResourceDescriptor<
                TTopology,
                TResourceIndex
            >;

        public:

            using Facility = typename ResourceDescriptor::Resource;

            static_assert(
                IsTaskFacility<Facility>::Value,
                "FacilityWorkerDescriptor requires a TaskExecutionFacility resource"
            );

            using Worker = WorkerDescriptor<
                typename Facility::WorkerSet,
                TWorkerIndex
            >;

            using Properties = typename Worker::Properties;

            static constexpr std::size_t ContextIndex =
                ResourceDescriptor::FirstContextIndex +
                TWorkerIndex;

            static_assert(
                TWorkerIndex < ResourceDescriptor::ContextCount,
                "Worker index exceeds the Task facility's statically declared Worker range"
            );

    };


    template<class TTopology>
    class StaticTopologyPlan final {

        public:

            static constexpr std::size_t ResourceCount =
                TTopology::ResourceCount;

            static constexpr std::size_t ManagedExecutionContextCount =
                TTopology::ManagedExecutionContextCount;

            template<std::size_t TResourceIndex>
            using Resource = TopologyResourceDescriptor<
                TTopology,
                TResourceIndex
            >;

            static_assert(
                TopologyContextOffset<
                    TTopology,
                    TTopology::ResourceCount
                >::Value == ManagedExecutionContextCount,
                "Static topology context ranges must exactly cover the topology-wide managed-context capacity"
            );

    };

} // ESPressio::Threading::Detail
