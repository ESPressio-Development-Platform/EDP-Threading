#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ManagedContextRouter.hpp"
#include "ManagedContextWakeSet.hpp"
#include "ShutdownWaitRuntime.hpp"
#include "StaticTopologyResourceStorage.hpp"
#include "StructuralContextResolver.hpp"
#include "ThreadingBootstrap.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `StructuralResolverForTuple`.
    /// @tparam TTuple Tuple Type whose resources are transformed or traversed.
    template<class TTuple>
    struct StructuralResolverForTuple;


    template<class... TResources>
    struct StructuralResolverForTuple<
        std::tuple<TResources...>
    > {

        /// Defines the compile-time contract for `Type`.
        /// @tparam TContextCapacity Number of managed execution contexts represented by the topology.
        template<std::size_t TContextCapacity>
        /// Resolved Type produced by this compile-time helper.
        using Type = StructuralContextResolver<
            TContextCapacity,
            TResources...
        >;

    };


    /// Defines the compile-time contract for `StaticTopologyOwner`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TSignalProvider Concrete Platform Signal provider Type used for targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TSpinLockProvider Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    template<class TTopology, class TBindings, class TSignalProvider, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider>
    class StaticTopologyOwner final {

        static_assert(
            ValidDedicatedThreadBindings<
                TTopology,
                TBindings
            >::Value,
            "Dedicated Thread callable bindings must exactly match the statically declared Dedicated Threads"
        );

        private:

            /// Number of managed execution contexts represented by this topology/runtime.
            static constexpr std::size_t ContextCapacity =
                TTopology::ManagedExecutionContextCount;

            /// Targeted-wake set Type owned by the runtime.
            using WakeSet = ManagedContextWakeSet<
                ContextCapacity,
                TSignalProvider
            >;

            /// Managed-context router Type owned by the runtime.
            using Router = ManagedContextRouter<
                ContextCapacity,
                TSignalProvider
            >;

            /// Bootstrap coordinator Type owning infrastructure lifecycle state.
            using Bootstrap = ThreadingBootstrap<
                TSpinLockProvider
            >;

            /// Bounded shutdown-completion wait runtime Type.
            using ShutdownWait = ShutdownWaitRuntime<
                typename Bootstrap::Lifecycle,
                ContextCapacity,
                TMutexProvider,
                Router
            >;

            /// Recursive in-place storage Type owning all declared topology resources.
            using Resources = StaticTopologyResourceStorage<
                TTopology,
                TBindings,
                Router,
                TExecutionContextProvider,
                TSpinLockProvider,
                TMutexProvider,
                0U
            >;

            /// Tuple Type describing the concrete resource runtime Types.
            using ResourceTypes = OwnedResourceTuple<
                TTopology,
                TBindings,
                Router,
                TExecutionContextProvider,
                TMutexProvider
            >;

            /// Structural context-resolver Type spanning all concrete resources.
            using Resolver = typename StructuralResolverForTuple<
                ResourceTypes
            >::template Type<ContextCapacity>;


            Bootstrap _bootstrap;

            WakeSet _wakeSet;

            Router _router;

            TBindings _bindings;

            Resources _resources;

            Resolver _resolver;

            ShutdownWait _shutdownWait;


            template<std::size_t... TIndices>
            static Resolver MakeResolver(
                Resources& resources,
                std::index_sequence<TIndices...>
            ) noexcept {
                return Resolver(
                    resources.template Get<TIndices>()...
                );
            }

            /// Defines the compile-time contract for `InitializeNext`.
            /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
            template<std::size_t TIndex>
            ThreadingInitializationResult InitializeNext() noexcept {
                if constexpr (
                    TIndex == TTopology::ResourceCount
                ) {
                    return ThreadingInitializationResult::Succeeded;
                } else {
                    const auto result = _resources.template Get<TIndex>().Initialize();

                    if (result != WorkerExecutionInitializationResult::Succeeded) {
                        DestroyInitializedPrefix<TIndex>();
                        return ThreadingInitializationResult::ProviderFailure;
                    }

                    return InitializeNext<TIndex + 1U>();
                }
            }

            /// Defines the compile-time contract for `DestroyInitializedPrefix`.
            /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
            template<std::size_t TIndex>
            void DestroyInitializedPrefix() noexcept {
                if constexpr (
                    TIndex > 0U
                ) {
                    static_cast<void>(
                        _resources.template Get<TIndex - 1U>().DestroyInfrastructure()
                    );

                    DestroyInitializedPrefix<TIndex - 1U>();
                }
            }

            /// Defines the compile-time contract for `InitializeResourceAt`.
            /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
            template<std::size_t TIndex>
            WorkerExecutionInitializationResult InitializeResourceAt(
                std::size_t targetIndex
            ) noexcept {
                if constexpr (
                    TIndex == TTopology::ResourceCount
                ) {
                    return WorkerExecutionInitializationResult::ProviderFailure;
                } else {
                    if (TIndex == targetIndex) {
                        return _resources.template Get<TIndex>().Initialize();
                    }

                    return InitializeResourceAt<TIndex + 1U>(
                        targetIndex
                    );
                }
            }

            /// Defines the compile-time contract for `DestroyResourceAt`.
            /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
            template<std::size_t TIndex>
            void DestroyResourceAt(
                std::size_t targetIndex
            ) noexcept {
                if constexpr (
                    TIndex < TTopology::ResourceCount
                ) {
                    if (TIndex == targetIndex) {
                        static_cast<void>(
                            _resources.template Get<TIndex>().DestroyInfrastructure()
                        );
                        return;
                    }

                    DestroyResourceAt<TIndex + 1U>(
                        targetIndex
                    );
                }
            }

            /// Defines the compile-time contract for `InitializeInRuntimeOrder`.
            /// @tparam TOrderIndex Compile-time position in an application-specified resource order.
            /// @tparam TOrderCount Number of resources in the application-specified order.
            template<std::size_t TOrderIndex, std::size_t TOrderCount>
            ThreadingInitializationResult InitializeInRuntimeOrder(
                const std::array<std::size_t, TOrderCount>& order
            ) noexcept {
                if constexpr (
                    TOrderIndex == TOrderCount
                ) {
                    return ThreadingInitializationResult::Succeeded;
                } else {
                    if (
                        InitializeResourceAt<0U>(
                            order[TOrderIndex]
                        ) != WorkerExecutionInitializationResult::Succeeded
                    ) {
                        for (std::size_t rollback = TOrderIndex; rollback > 0U; --rollback) {
                            DestroyResourceAt<0U>(
                                order[rollback - 1U]
                            );
                        }

                        return ThreadingInitializationResult::ProviderFailure;
                    }

                    return InitializeInRuntimeOrder<TOrderIndex + 1U>(
                        order
                    );
                }
            }


            /// Defines the compile-time contract for `TaskFacility`.
            /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
            template<class TPoolIdentity>
            auto& TaskFacility() noexcept {
                constexpr auto index = TaskFacilityResourceIndex<
                    TTopology,
                    TPoolIdentity
                >;

                static_assert(
                    index != TopologyResourceNotFound,
                    "Requested Task Pool identity is not present in this Threading topology"
                );

                return _resources.template Get<index>();
            }

            /// Defines the compile-time contract for `DedicatedWorker`.
            /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
            template<class TTaskIdentity>
            auto& DedicatedWorker() noexcept {
                constexpr auto index = DedicatedWorkerResourceIndex<
                    TTopology,
                    TTaskIdentity
                >;

                static_assert(
                    index != TopologyResourceNotFound,
                    "Requested Dedicated Worker Task identity is not present in this Threading topology"
                );

                return _resources.template Get<index>();
            }

            /// Defines the compile-time contract for `DedicatedThreadResource`.
            /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
            template<class TThreadIdentity>
            auto& DedicatedThreadResource() noexcept {
                constexpr auto index = DedicatedThreadResourceIndex<
                    TTopology,
                    TThreadIdentity
                >;

                static_assert(
                    index != TopologyResourceNotFound,
                    "Requested Dedicated Thread identity is not present in this Threading topology"
                );

                return _resources.template Get<index>();
            }


            template<std::size_t... TIndices>
            ThreadingStartResult StartAll(
                std::index_sequence<TIndices...>
            ) noexcept {
                return _bootstrap.Start(
                    _resources.template Get<TIndices>()...
                );
            }

        public:

            StaticTopologyOwner(
                const StaticTopologyOwner&
            ) = delete;

            StaticTopologyOwner& operator =(
                const StaticTopologyOwner&
            ) = delete;

            StaticTopologyOwner(
                StaticTopologyOwner&&
            ) = delete;

            StaticTopologyOwner& operator =(
                StaticTopologyOwner&&
            ) = delete;


            explicit StaticTopologyOwner(
                TBindings bindings
            ) :
                _bootstrap(),
                _wakeSet(),
                _router(
                    _wakeSet
                ),
                _bindings(
                    std::move(
                        bindings
                    )
                ),
                _resources(
                    _bindings,
                    _router,
                    _bootstrap.LifecycleState()
                ),
                _resolver(
                    MakeResolver(
                        _resources,
                        std::make_index_sequence<TTopology::ResourceCount>{}
                    )
                ),
                _shutdownWait(
                    _bootstrap.LifecycleState(),
                    _router
                ) {
                if constexpr (
                    ContextCapacity > 0U
                ) {
                    _router.BindTopology(
                        &_resolver,
                        &Resolver::ResolveCurrentThunk,
                        &Resolver::IsInterruptedThunk
                    );
                }
            }


            ThreadingInitializationResult Initialize() noexcept {
                if (
                    _bootstrap.LifecycleState().State() !=
                    InfrastructureState::Uninitialized
                ) {
                    return ThreadingInitializationResult::AlreadyInitialized;
                }

                if (
                    _wakeSet.Validate() !=
                    ManagedContextWakeValidationResult::Ready ||
                    !_shutdownWait.ValidateSynchronization()
                ) {
                    return ThreadingInitializationResult::ProviderFailure;
                }

                const auto resourceResult = InitializeNext<0U>();

                if (resourceResult != ThreadingInitializationResult::Succeeded) {
                    return resourceResult;
                }

                return _bootstrap.CommitInitialization();
            }

            template<std::size_t... TResourceIndices>
            ThreadingInitializationResult InitializeInOrder() noexcept {
                static_assert(
                    sizeof...(TResourceIndices) == TTopology::ResourceCount,
                    "InitializeInOrder must name every topology resource exactly once"
                );

                constexpr std::array<std::size_t, TTopology::ResourceCount> order{
                    TResourceIndices...
                };

                constexpr bool unique = []() constexpr {
                    constexpr std::array<std::size_t, TTopology::ResourceCount> values{
                        TResourceIndices...
                    };

                    for (std::size_t left = 0U; left < values.size(); ++left) {
                        for (std::size_t right = left + 1U; right < values.size(); ++right) {
                            if (values[left] == values[right]) {
                                return false;
                            }
                        }
                    }

                    return true;
                }();

                static_assert(
                    (
                        (TResourceIndices < TTopology::ResourceCount) &&
                        ... &&
                        true
                    ) &&
                    unique,
                    "InitializeInOrder must name every topology resource exactly once"
                );

                if (
                    _bootstrap.LifecycleState().State() !=
                    InfrastructureState::Uninitialized
                ) {
                    return ThreadingInitializationResult::AlreadyInitialized;
                }

                if (
                    _wakeSet.Validate() !=
                    ManagedContextWakeValidationResult::Ready ||
                    !_shutdownWait.ValidateSynchronization()
                ) {
                    return ThreadingInitializationResult::ProviderFailure;
                }

                const auto result = InitializeInRuntimeOrder<0U>(
                    order
                );

                if (result != ThreadingInitializationResult::Succeeded) {
                    return result;
                }

                return _bootstrap.CommitInitialization();
            }

            ThreadingStartResult Start() noexcept {
                return StartAll(
                    std::make_index_sequence<TTopology::ResourceCount>{}
                );
            }

            template<std::size_t... TResourceIndices>
            ThreadingStartResult StartInOrder() noexcept {
                static_assert(
                    sizeof...(TResourceIndices) == TTopology::ResourceCount,
                    "StartInOrder must name every topology resource exactly once"
                );

                static_assert(
                    (
                        (TResourceIndices < TTopology::ResourceCount) &&
                        ... &&
                        true
                    ),
                    "StartInOrder contains a topology resource index outside the declared range"
                );

                constexpr bool unique = []() constexpr {
                    constexpr std::array<std::size_t, TTopology::ResourceCount> order{
                        TResourceIndices...
                    };

                    for (std::size_t left = 0U; left < order.size(); ++left) {
                        for (std::size_t right = left + 1U; right < order.size(); ++right) {
                            if (order[left] == order[right]) {
                                return false;
                            }
                        }
                    }

                    return true;
                }();

                static_assert(
                    unique,
                    "StartInOrder must name every topology resource exactly once"
                );

                return _bootstrap.Start(
                    _resources.template Get<TResourceIndices>()...
                );
            }


            // Lifecycle-gated semantic execution surfaces.

            /// Defines the compile-time contract for `Dispatch`.
            /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
            /// @tparam TCallable Callable Type being dispatched or adapted.
            template<class TPoolIdentity, class TCallable>
            auto Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            ) {
                return _bootstrap.Dispatch(
                    TaskFacility<TPoolIdentity>(),
                    std::forward<TCallable>(
                        callable
                    ),
                    policy,
                    timeout
                );
            }

            /// Defines the compile-time contract for `DispatchDedicated`.
            /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
            /// @tparam TCallable Callable Type being dispatched or adapted.
            template<class TTaskIdentity, class TCallable>
            auto DispatchDedicated(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            ) {
                return _bootstrap.Dispatch(
                    DedicatedWorker<TTaskIdentity>(),
                    std::forward<TCallable>(
                        callable
                    ),
                    policy,
                    timeout
                );
            }

            /// Defines the compile-time contract for `ThreadHandle`.
            /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
            template<class TThreadIdentity>
            Thread<TThreadIdentity> ThreadHandle() noexcept {
                return DedicatedThreadResource<TThreadIdentity>().Handle();
            }

            /// Defines the compile-time contract for `StartThread`.
            /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
            template<class TThreadIdentity>
            ThreadStartResult StartThread() noexcept {
                return _bootstrap.StartThread(
                    DedicatedThreadResource<TThreadIdentity>()
                );
            }


            // Terminal semantic shutdown.

            ThreadingShutdownResult BeginShutdown() noexcept {
                const auto result = _bootstrap.LifecycleState().BeginShutdown();

                if (result == ThreadingShutdownResult::Accepted) {
                    _resources.BeginShutdown();
                }

                return result;
            }

            bool IsExecutionQuiescent() noexcept {
                return _resources.IsExecutionQuiescent();
            }

            ThreadingFinalizationResult FinalizeShutdown() noexcept {
                if (
                    _bootstrap.LifecycleState().State() !=
                    InfrastructureState::ShuttingDown
                ) {
                    return ThreadingFinalizationResult::NotShuttingDown;
                }

                if (!IsExecutionQuiescent()) {
                    return ThreadingFinalizationResult::ExecutionNotQuiescent;
                }

                if (!_resources.FinalizeShutdown()) {
                    return ThreadingFinalizationResult::ProviderFailure;
                }

                _bootstrap.LifecycleState().PublishShutdownComplete();
                _shutdownWait.WakeCompleted();

                return ThreadingFinalizationResult::Completed;
            }


            ShutdownWaitResult WaitForShutdown() {
                return _shutdownWait.Wait();
            }

            ShutdownWaitResult WaitForShutdownFor(
                Duration duration
            ) {
                return _shutdownWait.WaitFor(
                    duration
                );
            }

            ShutdownWaitResult WaitForShutdownUntil(
                MonotonicTimestamp deadline
            ) {
                return _shutdownWait.WaitUntil(
                    deadline
                );
            }



    };

} // ESPressio::Threading::Detail
