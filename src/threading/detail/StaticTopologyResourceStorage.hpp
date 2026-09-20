#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "InfrastructureLifecycle.hpp"
#include "StaticTopologyResourceTypes.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `DeclarationThreadIdentity`.
    /// @tparam TDeclaration Static topology declaration Type being realized.
    template<class TDeclaration>
    struct DeclarationThreadIdentity;


    /// Extracts the semantic Thread identity from a Dedicated Thread declaration.
    /// @tparam TThreadIdentity Semantic identity Type carried by the Dedicated Thread declaration.
    /// @tparam TProperties Compile-time execution-property Types carried by the declaration.
    template<class TThreadIdentity, class... TProperties>
    struct DeclarationThreadIdentity<
        DedicatedThread<TThreadIdentity, TProperties...>
    > {

        /// Resolved Type produced by this compile-time helper.
        using Type = TThreadIdentity;

    };


    /// Defines the compile-time contract for `OwnedResourceAt`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TResourceIndex Compile-time topology resource index.
    /// Resolves the concrete owned runtime Type corresponding to one topology resource.
    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TResourceIndex>
    using OwnedResourceAt = typename OwnedResourceType<
        typename StaticTopologyPlan<TTopology>::template Resource<TResourceIndex>::Resource,
        TBindings,
        TManagedContextRouter,
        TExecutionContextProvider,
        TMutexProvider,
        StaticTopologyPlan<TTopology>::template Resource<TResourceIndex>::FirstContextIndex,
        TTopology::ManagedExecutionContextCount
    >::Type;


    /// Defines the compile-time contract for `ConstructOwnedResource`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TSpinLockProvider Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TResourceIndex Compile-time topology resource index.
    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider, std::size_t TResourceIndex>
    auto ConstructOwnedResource(
        TBindings& bindings,
        TManagedContextRouter& router,
        InfrastructureLifecycle<TSpinLockProvider>& lifecycle
    ) {
        /// Compile-time descriptor for the resource being constructed.
        using Descriptor =
            typename StaticTopologyPlan<TTopology>::template Resource<TResourceIndex>;

        /// Static resource declaration Type represented by this storage node.
        using Declaration = typename Descriptor::Resource;

        /// Topology resource Type described at this compile-time position.
        using Resource = OwnedResourceAt<
            TTopology,
            TBindings,
            TManagedContextRouter,
            TExecutionContextProvider,
            TMutexProvider,
            TResourceIndex
        >;

        if constexpr (
            IsDedicatedThread<Declaration>::Value
        ) {
            /// Semantic identity Type of a Dedicated Thread declaration.
            using ThreadIdentity =
                typename DeclarationThreadIdentity<Declaration>::Type;

            /// Application callable-binding Type matched to a Dedicated Thread.
            using Binding = typename DedicatedThreadBindingTypeFromTuple<
                ThreadIdentity,
                TBindings
            >::Type;

            return Resource(
                std::get<Binding>(
                    bindings
                ).TakeCallable(),
                router,
                &lifecycle,
                &InfrastructureLifecycle<TSpinLockProvider>::CanActivateThunk,
                &InfrastructureLifecycle<TSpinLockProvider>::ShouldTerminateThunk
            );
        } else {
            return Resource(
                router,
                &lifecycle,
                &InfrastructureLifecycle<TSpinLockProvider>::ShouldTerminateThunk
            );
        }
    }


    /// Defines the compile-time contract for `StaticTopologyResourceStorage`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TSpinLockProvider Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TResourceIndex Compile-time topology resource index.
    /// @tparam TComplete Whether recursive static resource storage has reached its terminal specialization.
    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider, std::size_t TResourceIndex, bool TComplete = (TResourceIndex == TTopology::ResourceCount)>
    class StaticTopologyResourceStorage;


    /// Defines the compile-time contract for `StaticTopologyResourceStorage`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TSpinLockProvider Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TResourceIndex Compile-time topology resource index.
    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider, std::size_t TResourceIndex>
    class StaticTopologyResourceStorage<
        TTopology,
        TBindings,
        TManagedContextRouter,
        TExecutionContextProvider,
        TSpinLockProvider,
        TMutexProvider,
        TResourceIndex,
        false
    > final {

        private:

            /// Topology resource Type described at this compile-time position.
            using Resource = OwnedResourceAt<
                TTopology,
                TBindings,
                TManagedContextRouter,
                TExecutionContextProvider,
                TMutexProvider,
                TResourceIndex
            >;

            /// Recursive lookup result for the remaining topology resource pack.
            using Tail = StaticTopologyResourceStorage<
                TTopology,
                TBindings,
                TManagedContextRouter,
                TExecutionContextProvider,
                TSpinLockProvider,
                TMutexProvider,
                TResourceIndex + 1U
            >;

            Resource _resource;

            Tail _tail;

        public:

            StaticTopologyResourceStorage(
                TBindings& bindings,
                TManagedContextRouter& router,
                InfrastructureLifecycle<TSpinLockProvider>& lifecycle
            ) :
                _resource(
                    ConstructOwnedResource<
                        TTopology,
                        TBindings,
                        TManagedContextRouter,
                        TExecutionContextProvider,
                        TSpinLockProvider,
                        TMutexProvider,
                        TResourceIndex
                    >(
                        bindings,
                        router,
                        lifecycle
                    )
                ),
                _tail(
                    bindings,
                    router,
                    lifecycle
                ) {}


            /// Defines the compile-time contract for `Get`.
            /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
            template<std::size_t TIndex>
            auto& Get() noexcept {
                static_assert(
                    TIndex >= TResourceIndex &&
                    TIndex < TTopology::ResourceCount,
                    "Topology resource storage index is outside the owned resource range"
                );

                if constexpr (
                    TIndex == TResourceIndex
                ) {
                    return _resource;
                } else {
                    return _tail.template Get<TIndex>();
                }
            }

            void BeginShutdown() noexcept {
                if constexpr (
                    IsTaskExecutionResource<
                        typename StaticTopologyPlan<TTopology>::template Resource<TResourceIndex>::Resource
                    >::Value
                ) {
                    _resource.BeginShutdownCancellation();
                } else if constexpr (
                    IsDedicatedThread<
                        typename StaticTopologyPlan<TTopology>::template Resource<TResourceIndex>::Resource
                    >::Value
                ) {
                    static_cast<void>(
                        _resource.RequestStop()
                    );
                }

                _tail.BeginShutdown();
            }

            bool IsExecutionQuiescent() noexcept {
                return _resource.IsExecutionQuiescent() &&
                    _tail.IsExecutionQuiescent();
            }

            ThreadingFinalizationResult FinalizeShutdown() noexcept {
                _resource.RequestInfrastructureTermination();

                const auto joinResult = _resource.JoinInfrastructure(
                    ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                );

                const auto destroyResult =
                    joinResult == ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded
                    ? _resource.DestroyInfrastructure()
                    : ESPressio::Platform::Execution::ExecutionDestroyResult::ProviderFailure;

                const auto tailResult = _tail.FinalizeShutdown();

                if (
                    joinResult != ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded ||
                    destroyResult != ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded ||
                    tailResult != ThreadingFinalizationResult::Completed
                ) {
                    return ThreadingFinalizationResult::ProviderFailure;
                }

                return ThreadingFinalizationResult::Completed;
            }


            /// Defines the compile-time contract for `Get`.
            /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
            template<std::size_t TIndex>
            const auto& Get() const noexcept {
                static_assert(
                    TIndex >= TResourceIndex &&
                    TIndex < TTopology::ResourceCount,
                    "Topology resource storage index is outside the owned resource range"
                );

                if constexpr (
                    TIndex == TResourceIndex
                ) {
                    return _resource;
                } else {
                    return _tail.template Get<TIndex>();
                }
            }

    };


    /// Defines the compile-time contract for `StaticTopologyResourceStorage`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TSpinLockProvider Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TResourceIndex Compile-time topology resource index.
    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider, std::size_t TResourceIndex>
    class StaticTopologyResourceStorage<
        TTopology,
        TBindings,
        TManagedContextRouter,
        TExecutionContextProvider,
        TSpinLockProvider,
        TMutexProvider,
        TResourceIndex,
        true
    > final {

        public:

            StaticTopologyResourceStorage(
                TBindings&,
                TManagedContextRouter&,
                InfrastructureLifecycle<TSpinLockProvider>&
            ) noexcept {}


            void BeginShutdown() noexcept {}

            bool IsExecutionQuiescent() noexcept {
                return true;
            }

            ThreadingFinalizationResult FinalizeShutdown() noexcept {
                return ThreadingFinalizationResult::Completed;
            }

    };

} // ESPressio::Threading::Detail
