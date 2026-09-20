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
            static auto MakeWorkers(
                Facility& facility,
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept,
                std::index_sequence<TIndices...>
            ) noexcept {
                return std::tuple<
                    WorkerContext<TIndices>...
                >(
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

            Facility _facility;

            std::tuple<
                WorkerContext<
                    0U
                >
            > _placeholder;

        public:

            using PoolIdentity = TPoolIdentity;
            using FacilityRuntime = Facility;

            static constexpr std::size_t WorkerCount = sizeof...(TWorkers);

    };

} // ESPressio::Threading::Detail
