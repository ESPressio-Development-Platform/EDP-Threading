#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../ThreadingComposition.hpp"
#include "StaticTopologyPlan.hpp"
#include "TaskFacilityRuntime.hpp"
#include "TaskWorkerExecutionContext.hpp"

namespace ESPressio::Threading::Detail {

    template<class TFacility, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t TFirstContextIndex, std::size_t TExecutionContextCapacity>
    class TaskFacilityOwnedRuntime;


    template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class... TWorkers, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t TFirstContextIndex, std::size_t TExecutionContextCapacity>
    class TaskFacilityOwnedRuntime<
        TaskExecutionFacility<
            TPoolIdentity,
            TRecordCapacity,
            TCallableCapacity,
            TResultCapacity,
            Workers<TWorkers...>
        >,
        TManagedContextRouter,
        TExecutionContextProvider,
        TAtomicWord8Provider,
        TMutexProvider,
        TFirstContextIndex,
        TExecutionContextCapacity
    > final {

        private:

            using Facility = TaskFacilityRuntime<
                TRecordCapacity::Value,
                TCallableCapacity::Value,
                TResultCapacity::Value,
                sizeof...(TWorkers),
                TFirstContextIndex,
                TExecutionContextCapacity,
                TAtomicWord8Provider,
                TMutexProvider,
                TManagedContextRouter
            >;

            template<std::size_t TWorkerIndex>
            using WorkerDeclaration = std::tuple_element_t<
                TWorkerIndex,
                std::tuple<TWorkers...>
            >;

            template<std::size_t TWorkerIndex>
            using WorkerContext = TaskWorkerExecutionContext<
                TExecutionContextProvider,
                WorkerDeclaration<TWorkerIndex>::Properties::StackCapacity,
                Facility,
                TManagedContextRouter
            >;

            template<std::size_t... TIndices>
            using WorkerTuple = std::tuple<
                WorkerContext<TIndices>...
            >;

            template<std::size_t... TIndices>
            static WorkerTuple<TIndices...> MakeWorkers(
                Facility& facility,
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept,
                std::index_sequence<TIndices...>
            ) noexcept {
                return WorkerTuple<TIndices...>(
                    WorkerContext<TIndices>(
                        facility,
                        router,
                        static_cast<typename Facility::ManagedContextIndex>(
                            TFirstContextIndex + TIndices
                        ),
                        shutdownContext,
                        isShutdownRequested
                    )...
                );
            }

            using WorkersTuple = decltype(
                MakeWorkers(
                    std::declval<Facility&>(),
                    std::declval<TManagedContextRouter&>(),
                    nullptr,
                    nullptr,
                    std::make_index_sequence<sizeof...(TWorkers)>{}
                )
            );

            Facility _facility;

            WorkersTuple _workers;


            // Infrastructure lifecycle helpers.

            template<std::size_t TIndex>
            void DestroyInitializedPrefix() noexcept {
                if constexpr (
                    TIndex > 0U
                ) {
                    static_cast<void>(
                        std::get<TIndex - 1U>(
                            _workers
                        ).Destroy()
                    );

                    DestroyInitializedPrefix<TIndex - 1U>();
                }
            }


            template<std::size_t TIndex>
            WorkerExecutionInitializationResult InitializeNext() noexcept {
                if constexpr (
                    TIndex == sizeof...(TWorkers)
                ) {
                    return WorkerExecutionInitializationResult::Succeeded;
                } else {
                    using Declaration = WorkerDeclaration<TIndex>;

                    const auto result = std::get<TIndex>(
                        _workers
                    ).Initialize(
                        Declaration::Properties::Priority,
                        Declaration::Properties::Affinity
                    );

                    if (result != WorkerExecutionInitializationResult::Succeeded) {
                        static_cast<void>(
                            std::get<TIndex>(
                                _workers
                            ).Destroy()
                        );

                        DestroyInitializedPrefix<TIndex>();

                        return result;
                    }

                    return InitializeNext<TIndex + 1U>();
                }
            }

            template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionStartResult StartNext() noexcept {
                if constexpr (
                    TIndex == sizeof...(TWorkers)
                ) {
                    return ESPressio::Platform::Execution::ExecutionStartResult::Succeeded;
                } else {
                    const auto result = std::get<TIndex>(
                        _workers
                    ).StartInfrastructure();

                    if (
                        result !=
                        ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
                    ) {
                        return result;
                    }

                    return StartNext<TIndex + 1U>();
                }
            }

            template<std::size_t TIndex>
            void RequestTerminationNext() noexcept {
                if constexpr (
                    TIndex < sizeof...(TWorkers)
                ) {
                    std::get<TIndex>(
                        _workers
                    ).RequestInfrastructureTermination();

                    RequestTerminationNext<TIndex + 1U>();
                }
            }

            template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionJoinResult JoinNext(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                if constexpr (
                    TIndex == sizeof...(TWorkers)
                ) {
                    return ESPressio::Platform::Execution::ExecutionJoinResult::Joined;
                } else {
                    const auto result = std::get<TIndex>(
                        _workers
                    ).Join(
                        timeout
                    );

                    if (
                        result !=
                        ESPressio::Platform::Execution::ExecutionJoinResult::Joined
                    ) {
                        return result;
                    }

                    return JoinNext<TIndex + 1U>(
                        timeout
                    );
                }
            }

            template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyNext() noexcept {
                if constexpr (
                    TIndex == sizeof...(TWorkers)
                ) {
                    return ESPressio::Platform::Execution::ExecutionDestroyResult::Destroyed;
                } else {
                    const auto result = std::get<TIndex>(
                        _workers
                    ).Destroy();

                    if (
                        result !=
                        ESPressio::Platform::Execution::ExecutionDestroyResult::Destroyed
                    ) {
                        return result;
                    }

                    return DestroyNext<TIndex + 1U>();
                }
            }

            template<std::size_t TIndex>
            bool IsCurrentContextNext(
                typename Facility::ManagedContextIndex& contextIndex
            ) const noexcept {
                if constexpr (
                    TIndex == sizeof...(TWorkers)
                ) {
                    return false;
                } else {
                    if (
                        std::get<TIndex>(
                            _workers
                        ).IsCurrentContext()
                    ) {
                        contextIndex = static_cast<typename Facility::ManagedContextIndex>(
                            TFirstContextIndex + TIndex
                        );
                        return true;
                    }

                    return IsCurrentContextNext<TIndex + 1U>(
                        contextIndex
                    );
                }
            }

        public:

            using PoolIdentity = TPoolIdentity;
            using FacilityRuntime = Facility;

            static constexpr std::size_t WorkerCount = sizeof...(TWorkers);


            // Construction.

            TaskFacilityOwnedRuntime(
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept :
                _facility(router),
                _workers(
                    MakeWorkers(
                        _facility,
                        router,
                        shutdownContext,
                        isShutdownRequested,
                        std::make_index_sequence<sizeof...(TWorkers)>{}
                    )
                ) {}


            // Infrastructure lifecycle.

            WorkerExecutionInitializationResult Initialize() noexcept {
                return InitializeNext<0U>();
            }

            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                return StartNext<0U>();
            }

            void RequestInfrastructureTermination() noexcept {
                RequestTerminationNext<0U>();
            }

            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return JoinNext<0U>(
                    timeout
                );
            }

            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                return DestroyNext<0U>();
            }


            // Facility access.

            Facility& FacilityState() noexcept {
                return _facility;
            }

            const Facility& FacilityState() const noexcept {
                return _facility;
            }


            // Structural context resolution.

            bool TryResolveCurrentContext(
                typename Facility::ManagedContextIndex& contextIndex
            ) const noexcept {
                return IsCurrentContextNext<0U>(
                    contextIndex
                );
            }

            bool IsContextInterrupted(
                typename Facility::ManagedContextIndex contextIndex
            ) noexcept {
                if (
                    contextIndex < static_cast<typename Facility::ManagedContextIndex>(
                        TFirstContextIndex
                    ) ||
                    contextIndex >= static_cast<typename Facility::ManagedContextIndex>(
                        TFirstContextIndex + WorkerCount
                    )
                ) {
                    return false;
                }

                return _facility.IsCancellationRequestedForContext(
                    contextIndex
                );
            }


            // Shutdown cooperation.

            void BeginShutdownCancellation() noexcept {
                _facility.BeginShutdownCancellation();
            }

            bool IsExecutionQuiescent() noexcept {
                return _facility.IsExecutionQuiescent();
            }

    };

} // ESPressio::Threading::Detail
