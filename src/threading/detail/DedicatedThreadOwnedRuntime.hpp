#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "../ThreadingComposition.hpp"
#include "DedicatedThreadRuntime.hpp"

namespace ESPressio::Threading::Detail {

    template<class TDeclaration, class TCallable, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedThreadOwnedRuntime;


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


            bool IsExecutionQuiescent() noexcept {
                return _runtime.IsExecutionQuiescent();
            }


            Runtime& RuntimeState() noexcept {
                return _runtime;
            }

    };

} // ESPressio::Threading::Detail
