#pragma once

#include <cstddef>

#include <ESPressio_BoundedTopology.hpp>

#include "TopologyIndex.hpp"

namespace ESPressio::Threading::Detail {

    /// Shared bounded set representing structural availability of Task records.
    ///
    /// @tparam TCapacity Compile-time Task-record capacity represented by the set.
    template<std::size_t TCapacity>
    using TaskRecordAvailabilitySet = ESPressio::BoundedTopology::BoundedIndexSet<
        TaskRecordIndexSpace,
        TCapacity
    >;


    /// Shared non-owning intrusive FIFO topology over Task records.
    ///
    /// @tparam TCapacity Compile-time Task-record capacity represented by the queue.
    template<std::size_t TCapacity>
    using TaskRecordQueue = ESPressio::BoundedTopology::IntrusiveQueue<
        TaskRecordIndexSpace,
        TCapacity
    >;


    /// Shared bounded set representing availability of facility-local Worker leases.
    ///
    /// @tparam TCapacity Compile-time Worker count represented by the set.
    template<std::size_t TCapacity>
    using WorkerAvailabilitySet = ESPressio::BoundedTopology::BoundedIndexSet<
        WorkerLeaseIndexSpace,
        TCapacity
    >;

} // ESPressio::Threading::Detail
