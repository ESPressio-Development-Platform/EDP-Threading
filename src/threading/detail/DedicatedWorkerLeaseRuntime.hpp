#pragma once

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

#include "TaskFacilityRuntime.hpp"
#include "TaskWorkerExecutionContext.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `DedicatedWorkerLeaseRuntime`.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TRecordCapacity Bounded Task-record capacity.
    /// @tparam TCallableCapacity Per-record callable-storage capacity in bytes.
    /// @tparam TResultCapacity Per-record result-storage capacity in bytes.
    /// @tparam TStackCapacity Semantic stack-capacity value requested by the topology.
    /// @tparam TExecutionContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TTaskIdentity, std::size_t TRecordCapacity, std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TStackCapacity, std::size_t TExecutionContextIndex, std::size_t TExecutionContextCapacity, class TMutexProvider, class TExecutionContextProvider, class TManagedContextRouter>
    class DedicatedWorkerLeaseRuntime final {

        private:

            /// Concrete Task-facility runtime Type owned by this resource.
            using Facility = TaskFacilityRuntime<
                TRecordCapacity,
                TCallableCapacity,
                TResultCapacity,
                1U,
                TExecutionContextIndex,
                TExecutionContextCapacity,
                TMutexProvider,
                TManagedContextRouter
            >;

            /// Persistent Worker execution-context Type owned by this resource.
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

            /// Semantic identity Type of this Dedicated Worker task.
            using TaskIdentity = TTaskIdentity;

            /// Defines the compile-time contract for `TaskForCallable`.
            /// @tparam TCallable Callable Type being invoked, stored, or adapted.
            template<class TCallable>
            /// Concrete Task handle Type produced for the supplied callable.
            using TaskForCallable = typename Facility::template TaskForCallable<TCallable>;

            /// Defines the compile-time contract for `DispatchResultFor`.
            /// @tparam TCallable Callable Type being invoked, stored, or adapted.
            template<class TCallable>
            /// Typed dispatch-result Type produced for the supplied callable.
            using DispatchResultFor = typename Facility::template DispatchResultFor<TCallable>;

            /// Number of Workers owned by this resource.
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

                const auto result = _worker.Initialize(
                    priority,
                    affinity,
                    name
                );

                if (result != WorkerExecutionInitializationResult::Succeeded) {
                    static_cast<void>(
                        _worker.Destroy()
                    );
                }

                return result;
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

            /// Defines the compile-time contract for `Dispatch`.
            /// @tparam TCallable Callable Type being invoked, stored, or adapted.
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

            /// Returns this isolated Worker's dense context index when it owns the current Platform context.
            std::optional<typename Facility::ManagedContextIndex> CurrentContextIndex() const noexcept {
                if (!_worker.IsCurrentContext()) {
                    return std::nullopt;
                }

                return static_cast<typename Facility::ManagedContextIndex>(
                    TExecutionContextIndex
                );
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

            /// Compile-time capacity reported by `RecordCapacity`.
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

            /// Deterministic byte count reported for `ProviderObjectBytes`.
            static constexpr std::size_t ProviderObjectBytes() noexcept {
                return WorkerContext::ProviderObjectBytes();
            }

            /// Deterministic byte count reported for `ControlBackingBytes`.
            static constexpr std::size_t ControlBackingBytes() noexcept {
                return WorkerContext::ControlBackingBytes();
            }

            /// Deterministic byte count reported for `StackBackingBytes`.
            static constexpr std::size_t StackBackingBytes() noexcept {
                return WorkerContext::StackBackingBytes();
            }

    };

} // ESPressio::Threading::Detail
