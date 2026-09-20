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


            Thread<TThreadIdentity> Handle() noexcept {
                return _runtime.Handle();
            }

            ThreadStartResult StartActivation() noexcept {
                return _runtime.StartActivation();
            }

            ThreadStopRequestResult RequestStop() noexcept {
                return _runtime.RequestStop();
            }


            /// Returns this resource's dense context index when it owns the current Platform context.
            std::optional<typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type> CurrentContextIndex() const noexcept {
                return _runtime.CurrentContextIndex();
            }

            bool IsContextInterrupted(
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type contextIndex
            ) noexcept {
                return _runtime.IsContextInterrupted(
                    contextIndex
                );
            }


            bool IsExecutionQuiescent() noexcept {
                return _runtime.IsExecutionQuiescent();
            }


            Runtime& RuntimeState() noexcept {
                return _runtime;
            }

    };

} // ESPressio::Threading::Detail
