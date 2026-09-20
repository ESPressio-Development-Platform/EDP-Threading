#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "FacilityStorage.hpp"
#include "TaskRecord.hpp"

namespace ESPressio::Threading::Detail {

    enum class WorkerAvailabilityResult : std::uint8_t {
        Available = 0,
        OutsideFacilityRange = 1,
        ProviderFailure = 2
    };


    /// Defines the compile-time contract for `WorkerLeaseScheduler`.
    /// @tparam TWorkerCount Number of Workers represented by the lease scheduler.
    /// @tparam TFirstContextIndex First dense topology context index assigned to the Worker set.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<std::size_t TWorkerCount, std::size_t TFirstContextIndex, std::size_t TExecutionContextCapacity>
    class WorkerLeaseScheduler final {

        static_assert(
            TWorkerCount > 0U,
            "WorkerLeaseScheduler requires at least one Worker"
        );

        static_assert(
            TFirstContextIndex < TExecutionContextCapacity,
            "Worker range must begin inside the managed execution-context topology"
        );

        static_assert(
            TWorkerCount <= TExecutionContextCapacity - TFirstContextIndex,
            "Worker range must fit entirely inside the managed execution-context topology"
        );

        private:

            // Worker availability.

            /// One bit per Worker; one means available for a new Task grant.
            AvailabilityBitmap<TWorkerCount> _availableWorkers{false};


            // Worker-index conversion.

            /// Returns the zero-based facility Worker ordinal for one topology context index.
            std::size_t WorkerOrdinal(
                typename SmallestIndex<TExecutionContextCapacity>::Type contextIndex
            ) const noexcept {
                return static_cast<std::size_t>(
                    contextIndex
                ) - TFirstContextIndex;
            }

        public:

            // Context-index vocabulary.

            /// Dense topology-wide managed execution-context index Type.
            using ContextIndex = typename SmallestIndex<TExecutionContextCapacity>::Type;

            /// Number of Workers represented by this scheduler.
            static constexpr std::size_t WorkerCount = TWorkerCount;

            /// First dense managed execution-context index owned by this facility.
            static constexpr std::size_t FirstContextIndex = TFirstContextIndex;


            // Worker-range inspection.

            /// Indicates whether one managed execution-context index belongs to this facility's Workers.
            bool IsWorkerContext(
                ContextIndex contextIndex
            ) const noexcept {
                const auto normalized = static_cast<std::size_t>(
                    contextIndex
                );

                return normalized >= TFirstContextIndex &&
                    normalized < TFirstContextIndex + TWorkerCount;
            }

            /// Converts one zero-based Worker ordinal to its topology-wide dense context index.
            ContextIndex ContextIndexForWorker(
                std::size_t workerOrdinal
            ) const noexcept {
                return static_cast<ContextIndex>(
                    TFirstContextIndex + workerOrdinal
                );
            }


            // Availability mutation.

            /// Publishes one facility Worker as available for a new Task grant.
            WorkerAvailabilityResult MarkAvailable(
                ContextIndex contextIndex
            ) noexcept {
                if (!IsWorkerContext(
                    contextIndex
                )) {
                    return WorkerAvailabilityResult::OutsideFacilityRange;
                }

                _availableWorkers.Release(
                    WorkerOrdinal(
                        contextIndex
                    )
                );

                return WorkerAvailabilityResult::Available;
            }

            /// Attempts to reserve one specific Worker when it belongs to this facility and is available.
            std::optional<ContextIndex> TryClaimSpecific(
                ContextIndex contextIndex
            ) noexcept {
                if (!IsWorkerContext(
                    contextIndex
                )) {
                    return std::nullopt;
                }

                if (
                    _availableWorkers.TryClaimSpecific(
                        WorkerOrdinal(
                            contextIndex
                        )
                    ) != AvailabilityClaimResult::Claimed
                ) {
                    return std::nullopt;
                }

                return contextIndex;
            }

            /// Attempts to reserve the lowest-index currently available Worker.
            std::optional<ContextIndex> TryClaimAvailable() noexcept {
                std::size_t workerOrdinal = 0U;

                if (
                    _availableWorkers.TryClaim(
                        workerOrdinal
                    ) != AvailabilityClaimResult::Claimed
                ) {
                    return std::nullopt;
                }

                return ContextIndexForWorker(
                    workerOrdinal
                );
            }


            // Availability inspection.

            /// Indicates whether at least one facility Worker can accept a Task grant now.
            bool IsAnyAvailable() const noexcept {
                return _availableWorkers.IsAnyAvailable();
            }

            /// Returns the number of currently available Workers.
            std::size_t AvailableCount() const noexcept {
                return _availableWorkers.AvailableCount();
            }

            /// Returns the number of Workers currently granted to Tasks.
            std::size_t InUseCount() const noexcept {
                return TWorkerCount - AvailableCount();
            }

    };

} // ESPressio::Threading::Detail
