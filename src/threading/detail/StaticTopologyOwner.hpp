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


    /// Specializes structural-resolver construction for a tuple of concrete runtime resources.
    /// @tparam TResources Concrete topology-owned runtime resource Types represented by the tuple.
    template<class... TResources>
    struct StructuralResolverForTuple<
        std::tuple<TResources...>
    > {

        /// Resolves the concrete structural context-resolver Type for this resource tuple.
        /// @tparam TContextCapacity Number of managed execution contexts represented by the topology.
        template<std::size_t TContextCapacity>
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


            // Owned runtime state.

            /// Owns the authoritative Threading infrastructure lifecycle and start/shutdown coordinator.
            Bootstrap _bootstrap;

            /// Owns one targeted wake provider for each managed execution context.
            WakeSet _wakeSet;

            /// Routes current-context identity, interruption state, and targeted wake operations.
            Router _router;

            /// Owns the application-supplied Dedicated Thread callable bindings for the runtime lifetime.
            TBindings _bindings;

            /// Owns every statically declared Task facility, Dedicated Worker, and Dedicated Thread runtime resource.
            Resources _resources;

            /// Resolves managed context identity structurally across the owned resource tree.
            Resolver _resolver;

            /// Owns bounded wait registration for terminal Threading shutdown observation.
            ShutdownWait _shutdownWait;


            // Private construction and lifecycle helpers.

            /// Builds the structural context resolver from every topology-owned runtime resource.
            /// @tparam TIndices Compile-time resource indices expanded into the resolver constructor.
            template<std::size_t... TIndices>
            static Resolver MakeResolver(
                Resources& resources,
                std::index_sequence<TIndices...>
            ) noexcept {
                return Resolver(
                    resources.template Get<TIndices>()...
                );
            }

            /// Initializes topology resources recursively in declaration order and rolls back the initialized prefix on failure.
            /// @tparam TIndex Compile-time resource index currently being initialized.
            template<std::size_t TIndex>
            ThreadingInitializationResult InitializeNext() noexcept {
                if constexpr (
                    TIndex == TTopology::ResourceCount
                ) {
                    return ThreadingInitializationResult::Succeeded;
                } else {
                    const auto result = _resources.template Get<TIndex>().Initialize();

                    if (result != WorkerExecutionInitializationResult::Succeeded) {
                        static_cast<void>(
                            DestroyInitializedPrefix<TIndex>()
                        );
                        return ThreadingInitializationResult::ProviderFailure;
                    }

                    return InitializeNext<TIndex + 1U>();
                }
            }

            /// Destroys an already-initialized prefix in reverse order during initialization rollback.
            /// @tparam TIndex Number of initialized resources remaining in the prefix.
            template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInitializedPrefix() noexcept {
                if constexpr (
                    TIndex == 0U
                ) {
                    return ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;
                } else {
                    const auto result =
                        _resources.template Get<TIndex - 1U>().DestroyInfrastructure();

                    const auto tailResult = DestroyInitializedPrefix<TIndex - 1U>();

                    return result != ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded
                        ? result
                        : tailResult;
                }
            }

            /// Locates and initializes one runtime resource selected by a runtime index.
            /// @tparam TIndex Compile-time resource index currently being inspected.
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

            /// Locates and destroys one runtime resource selected by a runtime index during ordered rollback.
            /// @tparam TIndex Compile-time resource index currently being inspected.
            template<std::size_t TIndex>
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyResourceAt(
                std::size_t targetIndex
            ) noexcept {
                if constexpr (
                    TIndex == TTopology::ResourceCount
                ) {
                    return ESPressio::Platform::Execution::ExecutionDestroyResult::ProviderFailure;
                } else {
                    if (TIndex == targetIndex) {
                        return _resources.template Get<TIndex>().DestroyInfrastructure();
                    }

                    return DestroyResourceAt<TIndex + 1U>(
                        targetIndex
                    );
                }
            }

            /// Initializes resources according to a validated application-specified permutation and rolls back that same order on failure.
            /// @tparam TOrderIndex Compile-time position currently being initialized within the supplied order.
            /// @tparam TOrderCount Total number of topology resources represented by the supplied order.
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
                            static_cast<void>(
                                DestroyResourceAt<0U>(
                                    order[rollback - 1U]
                                )
                            );
                        }

                        return ThreadingInitializationResult::ProviderFailure;
                    }

                    return InitializeInRuntimeOrder<TOrderIndex + 1U>(
                        order
                    );
                }
            }


            /// Resolves the topology-owned ordinary Task facility identified by a Pool identity.
            /// @tparam TPoolIdentity Semantic identity Type of the requested Task pool.
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

            /// Resolves the topology-owned Dedicated Worker identified by its Task identity.
            /// @tparam TTaskIdentity Semantic identity Type of the requested Dedicated Worker task.
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

            /// Resolves the topology-owned Dedicated Thread runtime identified by Thread identity.
            /// @tparam TThreadIdentity Semantic identity Type of the requested Dedicated Thread.
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


            /// Starts all topology resources in the supplied compile-time order through transactional Bootstrap coordination.
            /// @tparam TIndices Compile-time topology resource indices expanded in start order.
            template<std::size_t... TIndices>
            ThreadingStartResult StartAll(
                std::index_sequence<TIndices...>
            ) noexcept {
                return _bootstrap.Start(
                    _resources.template Get<TIndices>()...
                );
            }

        public:

            // Construction and ownership.

            /// Prevents copying because this owner contains address-stable Platform resources and internal cross-references.
            StaticTopologyOwner(
                const StaticTopologyOwner&
            ) = delete;

            /// Prevents copy assignment for the address-stable topology owner.
            StaticTopologyOwner& operator =(
                const StaticTopologyOwner&
            ) = delete;

            /// Prevents moving because internal resource/router addresses must remain stable after construction.
            StaticTopologyOwner(
                StaticTopologyOwner&&
            ) = delete;

            /// Prevents move assignment for the address-stable topology owner.
            StaticTopologyOwner& operator =(
                StaticTopologyOwner&&
            ) = delete;


            /// Constructs the complete static topology, retaining Dedicated Thread bindings without starting execution.
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


            // Infrastructure initialization and start.

            /// Initializes every Platform-backed topology resource in declaration order without starting managed execution.
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
                    _shutdownWait.ValidateSynchronization() !=
                    ShutdownWaitSynchronizationResult::Ready
                ) {
                    return ThreadingInitializationResult::ProviderFailure;
                }

                const auto resourceResult = InitializeNext<0U>();

                if (resourceResult != ThreadingInitializationResult::Succeeded) {
                    return resourceResult;
                }

                return _bootstrap.CommitInitialization();
            }

            /// Initializes every topology resource using an application-specified compile-time permutation.
            /// @tparam TResourceIndices Topology resource indices naming every declared resource exactly once.
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
                    _shutdownWait.ValidateSynchronization() !=
                    ShutdownWaitSynchronizationResult::Ready
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

            /// Starts all initialized infrastructure contexts transactionally in topology declaration order.
            ThreadingStartResult Start() noexcept {
                return StartAll(
                    std::make_index_sequence<TTopology::ResourceCount>{}
                );
            }

            /// Starts all initialized topology resources transactionally using an application-specified compile-time permutation.
            /// @tparam TResourceIndices Topology resource indices naming every declared resource exactly once in start order.
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

            /// Dispatches finite work to the ordinary Task facility identified by Pool identity.
            /// @tparam TPoolIdentity Semantic identity Type of the destination Task pool.
            /// @tparam TCallable Callable Type admitted into the bounded facility.
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

            /// Dispatches finite work to the isolated Dedicated Worker identified by Task identity.
            /// @tparam TTaskIdentity Semantic identity Type of the destination Dedicated Worker task.
            /// @tparam TCallable Callable Type admitted into the isolated bounded facility.
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

            /// Returns a move-only non-owning control handle for the requested topology-owned Dedicated Thread.
            /// @tparam TThreadIdentity Semantic identity Type of the requested Dedicated Thread.
            template<class TThreadIdentity>
            Thread<TThreadIdentity> ThreadHandle() noexcept {
                return DedicatedThreadResource<TThreadIdentity>().Handle();
            }

            /// Starts one topology-owned Dedicated Thread through the lifecycle-gated Bootstrap surface.
            /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread to activate.
            template<class TThreadIdentity>
            ThreadStartResult StartThread() noexcept {
                return _bootstrap.StartThread(
                    DedicatedThreadResource<TThreadIdentity>()
                );
            }


            // Terminal semantic shutdown.

            /// Begins terminal shutdown, publishing the global lifecycle transition before cancelling/stopping resource work.
            ThreadingShutdownResult BeginShutdown() noexcept {
                const auto result = _bootstrap.LifecycleState().BeginShutdown();

                if (result == ThreadingShutdownResult::Accepted) {
                    static_cast<void>(
                        _resources.BeginShutdown()
                    );
                }

                return result;
            }

            /// Indicates whether every topology-owned execution resource has ceased active execution work.
            bool IsExecutionQuiescent() noexcept {
                return _resources.IsExecutionQuiescent();
            }

            /// Tears down quiescent infrastructure, publishes terminal completion, and wakes shutdown waiters.
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

                if (
                    _resources.FinalizeShutdown() !=
                    ThreadingFinalizationResult::Completed
                ) {
                    return ThreadingFinalizationResult::ProviderFailure;
                }

                _bootstrap.LifecycleState().PublishShutdownComplete();

                if (
                    _shutdownWait.WakeCompleted() !=
                    ShutdownWaitWakeResult::Succeeded
                ) {
                    return ThreadingFinalizationResult::ProviderFailure;
                }

                return ThreadingFinalizationResult::Completed;
            }


            // Terminal shutdown observation.

            /// Waits indefinitely for terminal shutdown completion from a managed Threading context.
            ShutdownWaitResult WaitForShutdown() {
                return _shutdownWait.Wait();
            }

            /// Waits for terminal shutdown completion using one relative canonical monotonic-time budget.
            ShutdownWaitResult WaitForShutdownFor(
                Duration duration
            ) {
                return _shutdownWait.WaitFor(
                    duration
                );
            }

            /// Waits for terminal shutdown completion until a canonical EDP-Clock monotonic deadline.
            ShutdownWaitResult WaitForShutdownUntil(
                MonotonicTimestamp deadline
            ) {
                return _shutdownWait.WaitUntil(
                    deadline
                );
            }



    };

} // ESPressio::Threading::Detail
