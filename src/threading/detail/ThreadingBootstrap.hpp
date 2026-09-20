#pragma once

#include <utility>

#include "../ThreadingTypes.hpp"
#include "InfrastructureLifecycle.hpp"
#include "ShutdownCoordinator.hpp"

namespace ESPressio::Threading::Detail {

    template<class TAtomicWord32Provider>
    class ThreadingBootstrap final {

        private:

            /// Authoritative application-wide Threading lifecycle.
            InfrastructureLifecycle<TAtomicWord32Provider> _lifecycle;

        public:

            using Lifecycle = InfrastructureLifecycle<TAtomicWord32Provider>;


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
            template<class... TResources>
            ThreadingStartResult Start(
                TResources&... resources
            ) noexcept {
                return _lifecycle.Start(
                    resources...
                );
            }


            // Lifecycle-gated Task dispatch.

            template<class TFacility, class TCallable>
            auto Dispatch(
                TFacility& facility,
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            ) {
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
