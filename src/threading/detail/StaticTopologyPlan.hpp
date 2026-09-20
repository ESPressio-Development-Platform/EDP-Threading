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
