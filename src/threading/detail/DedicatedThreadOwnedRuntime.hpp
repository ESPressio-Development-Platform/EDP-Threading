#pragma once

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

#include "../ThreadingComposition.hpp"
#include "DedicatedThreadRuntime.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `DedicatedThreadOwnedRuntime`.
    /// @tparam TDeclaration Static topology declaration Type being realized.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed contexts.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TDeclaration, class TCallable, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedThreadOwnedRuntime;


    /// Defines the compile-time contract for `DedicatedThreadOwnedRuntime`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed contexts.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    /// @tparam TProperties Compile-time execution-property Types declared for the Dedicated Thread.
    template<class TThreadIdentity, class... TProperties, class TCallable, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedThreadOwnedRuntime<
        DedicatedThread<TThreadIdentity, TProperties...>,
        TCallable,
        TManagedContextRouter,
        TExecutionContextProvider,
        TMutexProvider,
        TContextIndex,
        TExecutionContextCapacity
    > final {

        private:

            using Declaration = DedicatedThread<
                TThreadIdentity,
                TProperties...
            >;

            using Runtime = DedicatedThreadRuntime<
                TThreadIdentity,
                TCallable,
                Declaration::Properties::StackCapacity,
                TExecutionContextCapacity,
                TMutexProvider,
                TExecutionContextProvider,
                TManagedContextRouter
            >;

            Runtime _runtime;

        public:

            using ThreadIdentity = TThreadIdentity;
            using Callable = TCallable;
            using RuntimeType = Runtime;


            /// Constructs the owned Dedicated Thread runtime from its bound callable and topology services.
            DedicatedThreadOwnedRuntime(
                TCallable callable,
                TManagedContextRouter& router,
                const void* lifecycleContext,
                bool (*canActivate)(const void*) noexcept,
                bool (*shouldTerminate)(const void*) noexcept
            ) noexcept(
                std::is_nothrow_move_constructible_v<TCallable>
            ) :
                _runtime(
                    std::move(
                        callable
                    ),
                    router,
                    static_cast<
                        typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type
                    >(
                        TContextIndex
                    ),
                    lifecycleContext,
                    canActivate,
                    shouldTerminate
                ) {}


            /// Initializes the persistent Platform execution context without starting it.
            WorkerExecutionInitializationResult Initialize() noexcept {
                return _runtime.Initialize(
                    Declaration::Properties::Priority,
                    Declaration::Properties::Affinity
                );
            }

            /// Starts the persistent Platform execution context after global initialization commits.
            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                return _runtime.StartInfrastructure();
            }

            /// Wakes the persistent context so rollback or shutdown termination can be observed.
            void RequestInfrastructureTermination() noexcept {
                _runtime.RequestInfrastructureTermination();
            }

            /// Joins the persistent Platform execution context using the supplied Platform wait budget.
            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _runtime.JoinInfrastructure(
                    timeout
                );
            }

            /// Destroys the initialized Platform execution context and its provider-owned native state.
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                return _runtime.DestroyInfrastructure();
            }


            /// Creates a non-owning control handle for this topology-owned Dedicated Thread.
            Thread<TThreadIdentity> Handle() noexcept {
                return _runtime.Handle();
            }

            /// Starts one semantic Dedicated Thread activation when lifecycle state permits it.
            ThreadStartResult StartActivation() noexcept {
                return _runtime.StartActivation();
            }

            /// Requests cooperative stop of the currently active Dedicated Thread activation.
            ThreadStopRequestResult RequestStop() noexcept {
                return _runtime.RequestStop();
            }


            /// Returns this resource's dense context index when it owns the current Platform context.
            std::optional<typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type> CurrentContextIndex() const noexcept {
                return _runtime.CurrentContextIndex();
            }

            /// Indicates whether the addressed managed context currently carries a stop or shutdown interruption.
            bool IsContextInterrupted(
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type contextIndex
            ) noexcept {
                return _runtime.IsContextInterrupted(
                    contextIndex
                );
            }


            /// Indicates whether this Dedicated Thread has no active semantic execution remaining.
            bool IsExecutionQuiescent() noexcept {
                return _runtime.IsExecutionQuiescent();
            }


            /// Exposes the owned concrete runtime to internal topology coordination.
            Runtime& RuntimeState() noexcept {
                return _runtime;
            }

    };

} // ESPressio::Threading::Detail
