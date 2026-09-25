#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include <ESPressio_BoundedTopology.hpp>

namespace ESPressio::Threading::Detail {

    /// Semantic bounded-index space for statically provisioned Task records.
    struct TaskRecordIndexSpace final {
    };


    /// Semantic bounded-index space for managed execution contexts.
    struct ManagedContextIndexSpace final {
    };


    /// Semantic bounded-index space for facility-local Worker leases.
    struct WorkerLeaseIndexSpace final {
    };


    /// Semantic bounded-index space used only to select storage for TaskRecord's lifecycle-reused scratch scalar.
    struct TaskScratchIndexSpace final {
    };


    /// Adapts the shared bounded-index primitive to Threading's supported compact-index range.
    ///
    /// @tparam TIndexSpace Semantic compile-time index-space tag.
    /// @tparam TCapacity Positive bounded capacity represented by the index.
    template<class TIndexSpace, std::size_t TCapacity>
    struct TopologyIndexTraits final {

        static_assert(
            TCapacity > 0U,
            "Threading compact indices require positive capacity"
        );

        static_assert(
            TCapacity <= static_cast<std::size_t>(
                std::numeric_limits<std::uint32_t>::max()
            ),
            "Threading compact indices retain the existing maximum 32-bit representation"
        );

        /// Strong shared bounded-index Type for this semantic space and capacity.
        using Strong = ESPressio::BoundedTopology::BoundedIndex<
            TIndexSpace,
            TCapacity
        >;

        /// Smallest raw scalar representation selected by the shared bounded-index primitive.
        using Storage = typename Strong::Storage;

        /// Raw scalar sentinel which cannot identify a valid member of this bounded index space.
        static constexpr Storage Invalid = Strong::InvalidValue;

    };

} // ESPressio::Threading::Detail
