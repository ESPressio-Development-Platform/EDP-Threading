#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "InfrastructureLifecycle.hpp"
#include "StaticTopologyResourceTypes.hpp"

namespace ESPressio::Threading::Detail {

    template<class TDeclaration>
    struct DeclarationThreadIdentity;


    template<class TThreadIdentity, class... TProperties>
    struct DeclarationThreadIdentity<
        DedicatedThread<TThreadIdentity, TProperties...>
    > {

        using Type = TThreadIdentity;

    };


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


    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord32Provider, class TMutexProvider, std::size_t TResourceIndex>
    auto ConstructOwnedResource(
        TBindings& bindings,
        TManagedContextRouter& router,
        InfrastructureLifecycle<TAtomicWord32Provider>& lifecycle
    ) {
        using Descriptor =
            typename StaticTopologyPlan<TTopology>::template Resource<TResourceIndex>;

        using Declaration = typename Descriptor::Resource;

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
            using ThreadIdentity =
                typename DeclarationThreadIdentity<Declaration>::Type;

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
                &InfrastructureLifecycle<TAtomicWord32Provider>::CanActivateThunk,
                &InfrastructureLifecycle<TAtomicWord32Provider>::ShouldTerminateThunk
            );
        } else {
            return Resource(
                router,
                &lifecycle,
                &InfrastructureLifecycle<TAtomicWord32Provider>::ShouldTerminateThunk
            );
        }
    }


    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord32Provider, class TMutexProvider, std::size_t TResourceIndex, bool TComplete = (TResourceIndex == TTopology::ResourceCount)>
    class StaticTopologyResourceStorage;


    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord32Provider, class TMutexProvider, std::size_t TResourceIndex>
    class StaticTopologyResourceStorage<
        TTopology,
        TBindings,
        TManagedContextRouter,
        TExecutionContextProvider,
        TAtomicWord32Provider,
        TMutexProvider,
        TResourceIndex,
        false
    > final {

        private:

            using Resource = OwnedResourceAt<
                TTopology,
                TBindings,
                TManagedContextRouter,
                TExecutionContextProvider,
                TMutexProvider,
                TResourceIndex
            >;

            using Tail = StaticTopologyResourceStorage<
                TTopology,
                TBindings,
                TManagedContextRouter,
                TExecutionContextProvider,
                TAtomicWord32Provider,
                TMutexProvider,
                TResourceIndex + 1U
            >;

            Resource _resource;

            Tail _tail;

        public:

            StaticTopologyResourceStorage(
                TBindings& bindings,
                TManagedContextRouter& router,
                InfrastructureLifecycle<TAtomicWord32Provider>& lifecycle
            ) :
                _resource(
                    ConstructOwnedResource<
                        TTopology,
                        TBindings,
                        TManagedContextRouter,
                        TExecutionContextProvider,
                        TAtomicWord32Provider,
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

            bool FinalizeShutdown() noexcept {
                _resource.RequestInfrastructureTermination();

                const bool joined =
                    _resource.JoinInfrastructure(
                        ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                    ) ==
                    ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded;

                const bool destroyed =
                    joined &&
                    _resource.DestroyInfrastructure() ==
                    ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;

                const bool tailFinalized =
                    _tail.FinalizeShutdown();

                return joined &&
                    destroyed &&
                    tailFinalized;
            }


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


    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord32Provider, class TMutexProvider, std::size_t TResourceIndex>
    class StaticTopologyResourceStorage<
        TTopology,
        TBindings,
        TManagedContextRouter,
        TExecutionContextProvider,
        TAtomicWord32Provider,
        TMutexProvider,
        TResourceIndex,
        true
    > final {

        public:

            StaticTopologyResourceStorage(
                TBindings&,
                TManagedContextRouter&,
                InfrastructureLifecycle<TAtomicWord32Provider>&
            ) noexcept {}


            void BeginShutdown() noexcept {}

            bool IsExecutionQuiescent() noexcept {
                return true;
            }

            bool FinalizeShutdown() noexcept {
                return true;
            }

    };

} // ESPressio::Threading::Detail
