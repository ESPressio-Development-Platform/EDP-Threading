#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../ThreadingComposition.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `TopologyContextOffset`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TResourceIndex Compile-time topology resource index.
    template<class TTopology, std::size_t TResourceIndex>
    struct TopologyContextOffset {

        private:

            template<std::size_t... TIndices>
            /// Computes the managed-context prefix sum preceding one topology resource.
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

            /// Compile-time result or index produced by this trait/specialization.
            static constexpr std::size_t Value = Sum(
                std::make_index_sequence<TResourceIndex>{}
            );

    };


    /// Defines the compile-time contract for `TopologyResourceDescriptor`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TResourceIndex Compile-time topology resource index.
    template<class TTopology, std::size_t TResourceIndex>
    struct TopologyResourceDescriptor final {

        static_assert(
            TResourceIndex < TTopology::ResourceCount,
            "Topology resource descriptor index exceeds the statically declared resource count"
        );

        /// Topology resource Type described at this compile-time position.
        using Resource = std::tuple_element_t<
            TResourceIndex,
            typename TTopology::Resources
        >;

        /// First dense managed-context index assigned to this resource.
        static constexpr std::size_t FirstContextIndex =
            TopologyContextOffset<TTopology, TResourceIndex>::Value;

        /// Number of managed execution contexts owned by this resource.
        static constexpr std::size_t ContextCount =
            ManagedContextCount<Resource>::Value;

        /// One-past-last dense managed-context index owned by this resource.
        static constexpr std::size_t EndContextIndex =
            FirstContextIndex + ContextCount;

        static_assert(
            EndContextIndex <= TTopology::ManagedExecutionContextCount,
            "Topology resource context range exceeds the topology-wide managed-context capacity"
        );

    };


    /// Defines the compile-time contract for `WorkerDescriptor`.
    /// @tparam TWorkers Declared Worker set Type.
    /// @tparam TWorkerIndex Compile-time index of a Worker within its facility.
    template<class TWorkers, std::size_t TWorkerIndex>
    struct WorkerDescriptor;


    /// Defines the compile-time contract for `WorkerDescriptor`.
    /// @tparam TWorkerIndex Compile-time index of a Worker within its facility.
    template<class... TWorkers, std::size_t TWorkerIndex>
    struct WorkerDescriptor<
        Workers<TWorkers...>,
        TWorkerIndex
    > final {

        static_assert(
            TWorkerIndex < sizeof...(TWorkers),
            "Worker descriptor index exceeds the statically declared Worker count"
        );

        /// Worker declaration Type selected from the facility Worker pack.
        using WorkerType = std::tuple_element_t<
            TWorkerIndex,
            std::tuple<TWorkers...>
        >;

        /// Resolved execution properties belonging to the selected Worker/resource.
        using Properties = typename WorkerType::Properties;

    };


    /// Defines the compile-time contract for `FacilityWorkerDescriptor`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TResourceIndex Compile-time topology resource index.
    /// @tparam TWorkerIndex Compile-time index of a Worker within its facility.
    template<class TTopology, std::size_t TResourceIndex, std::size_t TWorkerIndex>
    struct FacilityWorkerDescriptor final {

        private:

            /// Compile-time descriptor for the owning topology resource.
            using ResourceDescriptor = TopologyResourceDescriptor<
                TTopology,
                TResourceIndex
            >;

        public:

            /// Concrete Task facility Type represented by this descriptor.
            using Facility = typename ResourceDescriptor::Resource;

            static_assert(
                IsTaskFacility<Facility>::Value,
                "FacilityWorkerDescriptor requires a TaskExecutionFacility resource"
            );

            /// Compile-time descriptor for the selected Worker.
            using Worker = WorkerDescriptor<
                typename Facility::WorkerSet,
                TWorkerIndex
            >;

            /// Resolved execution properties belonging to the selected Worker/resource.
            using Properties = typename Worker::Properties;

            /// Compact Type used to identify one managed execution context.
            static constexpr std::size_t ContextIndex =
                ResourceDescriptor::FirstContextIndex +
                TWorkerIndex;

            static_assert(
                TWorkerIndex < ResourceDescriptor::ContextCount,
                "Worker index exceeds the Task facility's statically declared Worker range"
            );

    };


    /// Defines the compile-time contract for `StaticTopologyPlan`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    template<class TTopology>
    class StaticTopologyPlan final {

        public:

            /// Number of resources declared by the topology.
            static constexpr std::size_t ResourceCount =
                TTopology::ResourceCount;

            /// Total number of managed execution contexts in the topology.
            static constexpr std::size_t ManagedExecutionContextCount =
                TTopology::ManagedExecutionContextCount;

            /// Defines the compile-time contract for `Resource`.
            /// @tparam TResourceIndex Compile-time topology resource index.
            template<std::size_t TResourceIndex>
            /// Topology resource Type described at this compile-time position.
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
