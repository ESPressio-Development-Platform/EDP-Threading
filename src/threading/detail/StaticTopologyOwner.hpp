#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ManagedContextRouter.hpp"
#include "ManagedContextWakeSet.hpp"
#include "StaticTopologyResourceStorage.hpp"
#include "StructuralContextResolver.hpp"
#include "ThreadingBootstrap.hpp"

namespace ESPressio::Threading::Detail {

    template<class TTuple>
    struct StructuralResolverForTuple;


    template<class... TResources>
    struct StructuralResolverForTuple<
        std::tuple<TResources...>
    > {

        template<std::size_t TContextCapacity>
        using Type = StructuralContextResolver<
            TContextCapacity,
            TResources...
        >;

    };


    template<class TTopology, class TBindings, class TSignalProvider, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider>
    class StaticTopologyOwner final {

        static_assert(
            TTopology::ManagedExecutionContextCount > 0U,
            "StaticTopologyOwner requires at least one managed execution context"
        );

        static_assert(
            ValidDedicatedThreadBindings<
                TTopology,
                TBindings
            >::Value,
            "Dedicated Thread callable bindings must exactly match the statically declared Dedicated Threads"
        );

        private:

            static constexpr std::size_t ContextCapacity =
                TTopology::ManagedExecutionContextCount;

            using WakeSet = ManagedContextWakeSet<
                ContextCapacity,
                TSignalProvider
            >;

            using Router = ManagedContextRouter<
                ContextCapacity,
                TSignalProvider
            >;

            using Bootstrap = ThreadingBootstrap<
                TAtomicWord8Provider
            >;

            using Resources = StaticTopologyResourceStorage<
                TTopology,
                TBindings,
                Router,
                TExecutionContextProvider,
                TAtomicWord8Provider,
                TMutexProvider,
                0U
            >;

            using ResourceTypes = OwnedResourceTuple<
                TTopology,
                TBindings,
                Router,
                TExecutionContextProvider,
                TAtomicWord8Provider,
                TMutexProvider
            >;

            using Resolver = typename StructuralResolverForTuple<
                ResourceTypes
            >::template Type<ContextCapacity>;


            Bootstrap _bootstrap;

            WakeSet _wakeSet;

            Router _router;

            TBindings _bindings;

            Resources _resources;

            Resolver _resolver;


            template<std::size_t... TIndices>
            static Resolver MakeResolver(
                Resources& resources,
                std::index_sequence<TIndices...>
            ) noexcept {
                return Resolver(
                    resources.template Get<TIndices>()...
                );
            }

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

            template<std::size_t... TIndices>
            ThreadingStartResult StartAll(
                std::index_sequence<TIndices...>
            ) noexcept {
                return _bootstrap.Start(
                    _resources.template Get<TIndices>()...
                );
            }

        public:

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
                ) {
                _router.BindTopology(
                    &_resolver,
                    &Resolver::ResolveCurrentThunk,
                    &Resolver::IsInterruptedThunk
                );
            }


            ThreadingInitializationResult Initialize() noexcept {
                if (
                    _wakeSet.Validate() !=
                    ManagedContextWakeValidationResult::Ready
                ) {
                    return ThreadingInitializationResult::ProviderFailure;
                }

                const auto resourceResult = InitializeNext<0U>();

                if (resourceResult != ThreadingInitializationResult::Succeeded) {
                    return resourceResult;
                }

                return _bootstrap.CommitInitialization();
            }

            ThreadingStartResult Start() noexcept {
                return StartAll(
                    std::make_index_sequence<TTopology::ResourceCount>{}
                );
            }


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


            Router& ContextRouter() noexcept {
                return _router;
            }

            Bootstrap& BootstrapState() noexcept {
                return _bootstrap;
            }

    };

} // ESPressio::Threading::Detail
