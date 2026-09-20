#pragma once

#include <cstddef>
#include <tuple>

#include "../ThreadingTypes.hpp"

namespace ESPressio::Threading::Detail {

    class ShutdownCoordinator final {

        private:

            template<std::size_t TIndex, class TTuple>
            static void CancelTaskResources(
                TTuple& resources
            ) noexcept {
                if constexpr (
                    TIndex < std::tuple_size_v<TTuple>
                ) {
                    std::get<TIndex>(
                        resources
                    ).BeginShutdownCancellation();

                    CancelTaskResources<TIndex + 1U>(
                        resources
                    );
                }
            }

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

                CancelTaskResources<0U>(
                    taskResources
                );

                StopDedicatedThreads<0U>(
                    dedicatedThreads
                );

                return ThreadingShutdownResult::Accepted;
            }

            /// Indicates whether all semantic execution has cooperatively ceased.
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
