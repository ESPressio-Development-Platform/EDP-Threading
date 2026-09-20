#pragma once

#include <cstddef>
#include <utility>

#include "../ThreadingComposition.hpp"
#include "DedicatedWorkerLeaseRuntime.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `DedicatedWorkerOwnedRuntime`.
    /// @tparam TDeclaration Static resource declaration Type being realized.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TDeclaration, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedWorkerOwnedRuntime;


    /// Defines the compile-time contract for `DedicatedWorkerOwnedRuntime`.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TTaskIdentity, class... TProperties, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedWorkerOwnedRuntime<
        DedicatedWorkerLease<TTaskIdentity, TProperties...>,
        TManagedContextRouter,
        TExecutionContextProvider,
        TMutexProvider,
        TContextIndex,
        TExecutionContextCapacity
    > final {

        private:

            /// Static topology declaration Type realized by this owned resource.
            using Declaration = DedicatedWorkerLease<
                TTaskIdentity,
                TProperties...
            >;

            /// Concrete runtime Type realized from the static declaration.
            using Runtime = DedicatedWorkerLeaseRuntime<
                TTaskIdentity,
                Declaration::RecordCapacity,
                Declaration::CallableStorageCapacity,
                Declaration::ResultStorageCapacity,
                Declaration::Properties::StackCapacity,
                TContextIndex,
                TExecutionContextCapacity,
                TMutexProvider,
                TExecutionContextProvider,
                TManagedContextRouter
            >;

            Runtime _runtime;

        public:

            /// Semantic identity Type of this Dedicated Worker task.
            using TaskIdentity = TTaskIdentity;
            /// Public alias exposing the concrete owned runtime Type.
            using RuntimeType = Runtime;

            /// Defines the compile-time contract for `TaskForCallable`.
            /// @tparam TCallable Callable Type being invoked, stored, or adapted.
            template<class TCallable>
            /// Concrete Task handle Type produced for the supplied callable.
            using TaskForCallable = typename Runtime::template TaskForCallable<TCallable>;

            /// Defines the compile-time contract for `DispatchResultFor`.
            /// @tparam TCallable Callable Type being invoked, stored, or adapted.
            template<class TCallable>
            /// Typed dispatch-result Type produced for the supplied callable.
            using DispatchResultFor = typename Runtime::template DispatchResultFor<TCallable>;


            DedicatedWorkerOwnedRuntime(
                TManagedContextRouter& router,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept :
                _runtime(
                    router,
                    shutdownContext,
                    isShutdownRequested
                ) {}


            WorkerExecutionInitializationResult Initialize() noexcept {
                return _runtime.Initialize(
                    Declaration::Properties::Priority,
                    Declaration::Properties::Affinity
                );
            }

            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                return _runtime.StartInfrastructure();
            }

            void RequestInfrastructureTermination() noexcept {
                _runtime.RequestInfrastructureTermination();
            }

            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _runtime.JoinInfrastructure(
                    timeout
                );
            }

            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                return _runtime.DestroyInfrastructure();
            }


            /// Defines the compile-time contract for `Dispatch`.
            /// @tparam TCallable Callable Type being invoked, stored, or adapted.
            template<class TCallable>
            auto Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            ) {
                return _runtime.Dispatch(
                    std::forward<TCallable>(
                        callable
                    ),
                    policy,
                    timeout
                );
            }


            bool TryResolveCurrentContext(
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type& contextIndex
            ) const noexcept {
                return _runtime.TryResolveCurrentContext(
                    contextIndex
                );
            }

            bool IsContextInterrupted(
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type contextIndex
            ) noexcept {
                return _runtime.IsContextInterrupted(
                    contextIndex
                );
            }


            void BeginShutdownCancellation() noexcept {
                _runtime.BeginShutdownCancellation();
            }

            bool IsExecutionQuiescent() noexcept {
                return _runtime.IsExecutionQuiescent();
            }


            Runtime& RuntimeState() noexcept {
                return _runtime;
            }

    };

} // ESPressio::Threading::Detail
