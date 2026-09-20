#pragma once

#include <cstddef>
#include <utility>

#include "../ThreadingComposition.hpp"
#include "DedicatedWorkerLeaseRuntime.hpp"

namespace ESPressio::Threading::Detail {

    template<class TDeclaration, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedWorkerOwnedRuntime;


    template<class TTaskIdentity, class... TProperties, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    class DedicatedWorkerOwnedRuntime<
        DedicatedWorkerLease<TTaskIdentity, TProperties...>,
        TManagedContextRouter,
        TExecutionContextProvider,
        TAtomicWord8Provider,
        TMutexProvider,
        TContextIndex,
        TExecutionContextCapacity
    > final {

        private:

            using Declaration = DedicatedWorkerLease<
                TTaskIdentity,
                TProperties...
            >;

            using Runtime = DedicatedWorkerLeaseRuntime<
                TTaskIdentity,
                Declaration::RecordCapacity,
                Declaration::CallableStorageCapacity,
                Declaration::ResultStorageCapacity,
                Declaration::Properties::StackCapacity,
                TContextIndex,
                TExecutionContextCapacity,
                TAtomicWord8Provider,
                TMutexProvider,
                TExecutionContextProvider,
                TManagedContextRouter
            >;

            Runtime _runtime;

        public:

            using TaskIdentity = TTaskIdentity;
            using RuntimeType = Runtime;

            template<class TCallable>
            using TaskForCallable = typename Runtime::template TaskForCallable<TCallable>;

            template<class TCallable>
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
