#pragma once

#include <cstddef>
#include <tuple>

#include "../ThreadingTypes.hpp"
#include "TaskFacilityRuntime.hpp"

namespace ESPressio::Threading::Detail {

    class ShutdownCoordinator final {

        private:

            /// Defines the compile-time contract for `CancelTaskResources`.
            /// @tparam TTuple Tuple Type containing the resources traversed by this helper.
            /// @tparam TIndex Compile-time tuple/resource index used by the recursive traversal.
            template<std::size_t TIndex, class TTuple>
            static TaskFacilityShutdownCancellationResult CancelTaskResources(
                TTuple& resources
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return TaskFacilityShutdownCancellationResult::Applied;
                } else {
                    const auto result = std::get<TIndex>(
                        resources
                    ).BeginShutdownCancellation();

                    const auto tailResult = CancelTaskResources<TIndex + 1U>(
                        resources
                    );

                    return result == TaskFacilityShutdownCancellationResult::ProviderFailure ||
                        tailResult == TaskFacilityShutdownCancellationResult::ProviderFailure
                        ? TaskFacilityShutdownCancellationResult::ProviderFailure
                        : TaskFacilityShutdownCancellationResult::Applied;
                }
            }

            /// Defines the compile-time contract for `StopDedicatedThreads`.
            /// @tparam TTuple Tuple Type containing the resources traversed by this helper.
            /// @tparam TIndex Compile-time tuple/resource index used by the recursive traversal.
            template<std::size_t TIndex, class TTuple>
            static void StopDedicatedThreads(
                TTuple& resources
            ) noexcept {
                if constexpr (
                    TIndex < std::tuple_size_v<TTuple>
                ) {
                    static_cast<void>(
                        std::get<TIndex>(
                            resources
                        ).RequestStop()
                    );

                    StopDedicatedThreads<TIndex + 1U>(
                        resources
                    );
                }
            }

            /// Defines the compile-time contract for `TaskResourcesQuiescent`.
            /// @tparam TTuple Tuple Type containing the resources traversed by this helper.
            /// @tparam TIndex Compile-time tuple/resource index used by the recursive traversal.
            template<std::size_t TIndex, class TTuple>
            static bool TaskResourcesQuiescent(
                TTuple& resources
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return true;
                } else {
                    return std::get<TIndex>(
                        resources
                    ).IsExecutionQuiescent() &&
                        TaskResourcesQuiescent<TIndex + 1U>(
                            resources
                        );
                }
            }

            /// Defines the compile-time contract for `DedicatedThreadsQuiescent`.
            /// @tparam TTuple Tuple Type containing the resources traversed by this helper.
            /// @tparam TIndex Compile-time tuple/resource index used by the recursive traversal.
            template<std::size_t TIndex, class TTuple>
            static bool DedicatedThreadsQuiescent(
                TTuple& resources
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return true;
                } else {
                    return std::get<TIndex>(
                        resources
                    ).IsExecutionQuiescent() &&
                        DedicatedThreadsQuiescent<TIndex + 1U>(
                            resources
                        );
                }
            }

        public:

            /// Initiates terminal semantic shutdown without waiting for arbitrary user callables.
            /// @tparam TLifecycle Authoritative Threading lifecycle Type coordinated during shutdown.
            /// @tparam TTaskResourceTuple Tuple Type containing Task-execution resources coordinated during shutdown.
            /// @tparam TDedicatedThreadTuple Tuple Type containing Dedicated Thread resources coordinated during shutdown.
            template<class TLifecycle, class TTaskResourceTuple, class TDedicatedThreadTuple>
            static ThreadingShutdownResult Begin(
                TLifecycle& lifecycle,
                TTaskResourceTuple& taskResources,
                TDedicatedThreadTuple& dedicatedThreads
            ) noexcept {
                const auto result = lifecycle.BeginShutdown();

                if (result != ThreadingShutdownResult::Accepted) {
                    return result;
                }

                static_cast<void>(
                    CancelTaskResources<0U>(
                        taskResources
                    )
                );

                StopDedicatedThreads<0U>(
                    dedicatedThreads
                );

                return ThreadingShutdownResult::Accepted;
            }

            /// Indicates whether all semantic execution has cooperatively ceased.
            /// @tparam TTaskResourceTuple Tuple Type containing Task-execution resources coordinated during shutdown.
            /// @tparam TDedicatedThreadTuple Tuple Type containing Dedicated Thread resources coordinated during shutdown.
            template<class TTaskResourceTuple, class TDedicatedThreadTuple>
            static bool IsExecutionQuiescent(
                TTaskResourceTuple& taskResources,
                TDedicatedThreadTuple& dedicatedThreads
            ) noexcept {
                return TaskResourcesQuiescent<0U>(
                    taskResources
                ) &&
                    DedicatedThreadsQuiescent<0U>(
                        dedicatedThreads
                    );
            }

    };

} // ESPressio::Threading::Detail
