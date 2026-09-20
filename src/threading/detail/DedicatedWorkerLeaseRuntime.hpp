#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "TaskFacilityRuntime.hpp"
#include "TaskWorkerExecutionContext.hpp"

namespace ESPressio::Threading::Detail {

    template<class TTaskIdentity, std::size_t TRecordCapacity, std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TStackCapacity, std::size_t TExecutionContextIndex, std::size_t TExecutionContextCapacity, class TAtomicWord8Provider, class TMutexProvider, class TExecutionContextProvider, class TManagedContextRouter>
    class DedicatedWorkerLeaseRuntime final {

        private:

            using Facility = TaskFacilityRuntime<
                TRecordCapacity,
                TCallableCapacity,
                TResultCapacity,
                1U,
                TExecutionContextIndex,
                TExecutionContextCapacity,
                TAtomicWord8Provider,
                TMutexProvider,
                TManagedContextRouter
            >;

            using WorkerContext = TaskWorkerExecutionContext<
                TExecutionContextProvider,
                TStackCapacity,
                Facility,
                TManagedContextRouter
            >;

            /// Isolated Task-record/FIFO/Worker-Lease facility for this critical Task Type only.
            Facility _facility;

            /// Exactly one isolated persistent Worker context.
            WorkerContext _worker;

        public:

            using TaskIdentity = TTaskIdentity;

            template<class TCallable>
            using TaskForCallable = typename Facility::template TaskForCallable<TCallable>;

            template<class TCallable>
            using DispatchResultFor = typename Facility::template DispatchResultFor<TCallable>;

            static constexpr std::size_t WorkerCount = 1U;


            // Construction.

            DedicatedWorkerLeaseRuntime(
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept :
                _facility(router),
                _worker(
                    _facility,
                    router,
                    static_cast<typename Facility::ManagedContextIndex>(
                        TExecutionContextIndex
                    ),
                    shutdownContext,
                    isShutdownRequested
                ) {}


            // Infrastructure lifecycle.

            WorkerExecutionInitializationResult Initialize(
                ESPressio::Platform::Execution::ExecutionPriority priority,
                ESPressio::Platform::Execution::ProcessorAffinity affinity,
                const char* name = nullptr
            ) noexcept {
                if (
                    _facility.ValidateSynchronization() !=
                    TaskFacilitySynchronizationResult::Ready
                ) {
                    return WorkerExecutionInitializationResult::ProviderFailure;
                }

                return _worker.Initialize(
                    priority,
                    affinity,
                    name
                );
            }

            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                return _worker.StartInfrastructure();
            }

            void RequestInfrastructureTermination() noexcept {
                _worker.RequestInfrastructureTermination();
            }

            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _worker.Join(
                    timeout
                );
            }

            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                return _worker.Destroy();
            }


            // Type-dedicated dispatch.

            template<class TCallable>
            auto Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            ) {
                static_assert(
                    std::is_same_v<
                        std::decay_t<TCallable>,
                        TTaskIdentity
                    >,
                    "DedicatedWorkerLease accepts only its configured critical Task Type"
                );

                return _facility.Dispatch(
                    std::forward<TCallable>(
                        callable
                    ),
                    policy,
                    timeout
                );
            }


            // Shutdown cooperation.

            void BeginShutdownCancellation() noexcept {
                _facility.BeginShutdownCancellation();
            }

            bool IsExecutionQuiescent() noexcept {
                return _facility.IsExecutionQuiescent();
            }


            // Context identity and interruption.

            bool TryResolveCurrentContext(
                typename Facility::ManagedContextIndex& contextIndex
            ) const noexcept {
                if (!_worker.IsCurrentContext()) {
                    return false;
                }

                contextIndex = static_cast<typename Facility::ManagedContextIndex>(
                    TExecutionContextIndex
                );
                return true;
            }

            bool IsContextInterrupted(
                typename Facility::ManagedContextIndex contextIndex
            ) noexcept {
                return
                    contextIndex == static_cast<typename Facility::ManagedContextIndex>(
                        TExecutionContextIndex
                    ) &&
                    _worker.IsInterrupted();
            }


            // Bounded observability.

            static constexpr std::size_t RecordCapacity() noexcept {
                return TRecordCapacity;
            }

            std::size_t RecordsInUse() noexcept {
                return _facility.RecordsInUse();
            }

            std::size_t QueuedTasks() noexcept {
                return _facility.QueuedTasks();
            }

            std::size_t WorkersInUse() noexcept {
                return _facility.WorkersInUse();
            }

            static constexpr std::size_t ProviderObjectBytes() noexcept {
                return WorkerContext::ProviderObjectBytes();
            }

            static constexpr std::size_t ControlBackingBytes() noexcept {
                return WorkerContext::ControlBackingBytes();
            }

            static constexpr std::size_t StackBackingBytes() noexcept {
                return WorkerContext::StackBackingBytes();
            }

    };

} // ESPressio::Threading::Detail
