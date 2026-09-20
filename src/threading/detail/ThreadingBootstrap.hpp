#pragma once

#include <utility>

#include "../ThreadingTypes.hpp"
#include "InfrastructureLifecycle.hpp"
#include "ShutdownCoordinator.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `ThreadingBootstrap`.
    /// @tparam TSpinLockProvider Concrete Platform SpinLock provider Type protecting lifecycle publication.
    template<class TSpinLockProvider>
    class ThreadingBootstrap final {

        private:

            /// Authoritative application-wide Threading lifecycle.
            InfrastructureLifecycle<TSpinLockProvider> _lifecycle;

        public:

            /// Authoritative lifecycle Type used by Threading Bootstrap.
            using Lifecycle = InfrastructureLifecycle<TSpinLockProvider>;


            // Lifecycle access for statically realized resources.

            Lifecycle& LifecycleState() noexcept {
                return _lifecycle;
            }

            const Lifecycle& LifecycleState() const noexcept {
                return _lifecycle;
            }


            // Initialization barrier.

            /// Commits Threading initialization after the owning application has initialized and
            /// validated every statically realized facility/context/provider. This method starts no
            /// execution context.
            ThreadingInitializationResult CommitInitialization() noexcept {
                return _lifecycle.CommitInitialization();
            }


            // Infrastructure Start.

            /// Starts statically realized execution contexts in the exact argument order supplied
            /// by the application composition. Failure is transactional and prevents operational
            /// commit.
            /// Coordinates initialization/start/shutdown across a compile-time pack of topology resources.
    /// @tparam TResources Concrete topology-owned runtime resource Types coordinated by Bootstrap.
    template<class... TResources>
            ThreadingStartResult Start(
                TResources&... resources
            ) noexcept {
                return _lifecycle.Start(
                    resources...
                );
            }


            // Lifecycle-gated Task dispatch.

            /// Defines the compile-time contract for `Dispatch`.
            /// @tparam TFacility Task facility Type receiving lifecycle-gated dispatch.
            /// @tparam TCallable Callable Type being dispatched.
            template<class TFacility, class TCallable>
            auto Dispatch(
                TFacility& facility,
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            ) {
                /// Typed dispatch result returned for the callable being dispatched.
                using DispatchResult = typename TFacility::template DispatchResultFor<TCallable>;

                if (!_lifecycle.CanActivate()) {
                    return DispatchResult(
                        TaskDispatchStatus::ShuttingDown
                    );
                }

                return facility.Dispatch(
                    std::forward<TCallable>(
                        callable
                    ),
                    policy,
                    timeout
                );
            }


            // Lifecycle-gated Dedicated Thread activation.

            /// Defines the compile-time contract for `StartThread`.
            /// @tparam TDedicatedThreadRuntime Dedicated Thread runtime Type being lifecycle-gated.
            template<class TDedicatedThreadRuntime>
            ThreadStartResult StartThread(
                TDedicatedThreadRuntime& thread
            ) noexcept {
                if (!_lifecycle.CanActivate()) {
                    return ThreadStartResult::ShuttingDown;
                }

                return thread.StartActivation();
            }


            // Terminal shutdown.

            /// Defines the compile-time contract for `BeginShutdown`.
            /// @tparam TTaskResourceTuple Tuple Type containing Task-execution resources.
            /// @tparam TDedicatedThreadTuple Tuple Type containing Dedicated Thread resources.
            template<class TTaskResourceTuple, class TDedicatedThreadTuple>
            ThreadingShutdownResult BeginShutdown(
                TTaskResourceTuple& taskResources,
                TDedicatedThreadTuple& dedicatedThreads
            ) noexcept {
                return ShutdownCoordinator::Begin(
                    _lifecycle,
                    taskResources,
                    dedicatedThreads
                );
            }

            /// Defines the compile-time contract for `IsExecutionQuiescent`.
            /// @tparam TTaskResourceTuple Tuple Type containing Task-execution resources.
            /// @tparam TDedicatedThreadTuple Tuple Type containing Dedicated Thread resources.
            template<class TTaskResourceTuple, class TDedicatedThreadTuple>
            bool IsExecutionQuiescent(
                TTaskResourceTuple& taskResources,
                TDedicatedThreadTuple& dedicatedThreads
            ) noexcept {
                return ShutdownCoordinator::IsExecutionQuiescent(
                    taskResources,
                    dedicatedThreads
                );
            }

            /// Finalizes terminal shutdown across all supplied infrastructure resources and shutdown wait state.
            /// @tparam TShutdownWaitRuntime Shutdown-wait runtime Type used for terminal completion observation.
            /// @tparam TInfrastructureResources Concrete infrastructure resource Types finalized during shutdown.
            template<class TShutdownWaitRuntime, class... TInfrastructureResources>
            void FinalizeShutdown(
                TShutdownWaitRuntime& shutdownWaitRuntime,
                TInfrastructureResources&... resources
            ) noexcept {
                _lifecycle.FinalizeShutdown(
                    shutdownWaitRuntime,
                    resources...
                );
            }

    };

} // ESPressio::Threading::Detail
