#pragma once

#include <cstddef>
#include <optional>
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


            /// Constructs the topology-owned Dedicated Worker runtime against routing and shutdown services.
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


            /// Initializes synchronization and the isolated persistent Worker context without starting execution.
            WorkerExecutionInitializationResult Initialize() noexcept {
                return _runtime.Initialize(
                    Declaration::Properties::Priority,
                    Declaration::Properties::Affinity
                );
            }

            /// Starts the isolated persistent Worker after the topology initialization barrier.
            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                return _runtime.StartInfrastructure();
            }

            /// Wakes the isolated Worker so rollback or shutdown termination can be observed.
            void RequestInfrastructureTermination() noexcept {
                _runtime.RequestInfrastructureTermination();
            }

            /// Joins the isolated Worker context using the supplied Platform wait budget.
            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _runtime.JoinInfrastructure(
                    timeout
                );
            }

            /// Destroys the isolated Worker context and releases its native provider state.
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                return _runtime.DestroyInfrastructure();
            }


            /// Defines the compile-time contract for `Dispatch`.
            /// @tparam TCallable Callable Type being invoked, stored, or adapted.
            template<class TCallable>
            /// Dispatches one callable through the isolated Dedicated Worker facility.
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


            /// Returns this resource's dense context index when it owns the current Platform context.
            std::optional<typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type> CurrentContextIndex() const noexcept {
                return _runtime.CurrentContextIndex();
            }

            /// Indicates whether the addressed isolated Worker context carries cancellation or shutdown interruption.
            bool IsContextInterrupted(
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type contextIndex
            ) noexcept {
                return _runtime.IsContextInterrupted(
                    contextIndex
                );
            }


            /// Applies terminal shutdown cancellation semantics to the isolated Task facility.
            void BeginShutdownCancellation() noexcept {
                _runtime.BeginShutdownCancellation();
            }

            /// Indicates whether this Dedicated Worker resource has no executable Task work remaining.
            bool IsExecutionQuiescent() noexcept {
                return _runtime.IsExecutionQuiescent();
            }


            /// Exposes the owned concrete runtime to internal topology coordination.
            Runtime& RuntimeState() noexcept {
                return _runtime;
            }

    };

} // ESPressio::Threading::Detail
