#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../ThreadingComposition.hpp"
#include "DedicatedThreadOwnedRuntime.hpp"
#include "DedicatedWorkerOwnedRuntime.hpp"
#include "StaticTopologyPlan.hpp"
#include "TaskFacilityOwnedRuntime.hpp"

namespace ESPressio::Threading::Detail {

    template<class TThreadIdentity, class TBinding>
    struct IsDedicatedThreadBindingFor {

        static constexpr bool Value = false;

    };


    template<class TThreadIdentity, class TCallable>
    struct IsDedicatedThreadBindingFor<
        TThreadIdentity,
        DedicatedThreadBinding<TThreadIdentity, TCallable>
    > {

        static constexpr bool Value = true;

    };


    template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingCount {

        static constexpr std::size_t Value =
            (
                static_cast<std::size_t>(
                    IsDedicatedThreadBindingFor<
                        TThreadIdentity,
                        TBindings
                    >::Value
                ) +
                ... +
                0U
            );

    };


    template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingType;


    template<class TThreadIdentity>
    struct DedicatedThreadBindingType<TThreadIdentity> {

        using Type = void;

    };


    template<class TThreadIdentity, class TFirstBinding, class... TRestBindings>
    struct DedicatedThreadBindingType<
        TThreadIdentity,
        TFirstBinding,
        TRestBindings...
    > {

        using Type = std::conditional_t<
            IsDedicatedThreadBindingFor<
                TThreadIdentity,
                TFirstBinding
            >::Value,
            TFirstBinding,
            typename DedicatedThreadBindingType<
                TThreadIdentity,
                TRestBindings...
            >::Type
        >;

    };


    template<class TDeclaration, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType;


    template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType<
        TaskExecutionFacility<
            TPoolIdentity,
            TRecordCapacity,
            TCallableCapacity,
            TResultCapacity,
            TWorkers
        >,
        std::tuple<TBindings...>,
        TManagedContextRouter,
        TExecutionContextProvider,
        TAtomicWord8Provider,
        TMutexProvider,
        TContextIndex,
        TExecutionContextCapacity
    > {

        using Type = TaskFacilityOwnedRuntime<
            TaskExecutionFacility<
                TPoolIdentity,
                TRecordCapacity,
                TCallableCapacity,
                TResultCapacity,
                TWorkers
            >,
            TManagedContextRouter,
            TExecutionContextProvider,
            TAtomicWord8Provider,
            TMutexProvider,
            TContextIndex,
            TExecutionContextCapacity
        >;

    };


    template<class TTaskIdentity, class... TProperties, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType<
        DedicatedWorkerLease<TTaskIdentity, TProperties...>,
        std::tuple<TBindings...>,
        TManagedContextRouter,
        TExecutionContextProvider,
        TAtomicWord8Provider,
        TMutexProvider,
        TContextIndex,
        TExecutionContextCapacity
    > {

        using Type = DedicatedWorkerOwnedRuntime<
            DedicatedWorkerLease<
                TTaskIdentity,
                TProperties...
            >,
            TManagedContextRouter,
            TExecutionContextProvider,
            TAtomicWord8Provider,
            TMutexProvider,
            TContextIndex,
            TExecutionContextCapacity
        >;

    };


    template<class TThreadIdentity, class... TProperties, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType<
        DedicatedThread<TThreadIdentity, TProperties...>,
        std::tuple<TBindings...>,
        TManagedContextRouter,
        TExecutionContextProvider,
        TAtomicWord8Provider,
        TMutexProvider,
        TContextIndex,
        TExecutionContextCapacity
    > {

        static_assert(
            DedicatedThreadBindingCount<
                TThreadIdentity,
                TBindings...
            >::Value == 1U,
            "Every DedicatedThread declaration requires exactly one matching DedicatedThreadBinding"
        );

        using Binding = typename DedicatedThreadBindingType<
            TThreadIdentity,
            TBindings...
        >::Type;

        using Type = DedicatedThreadOwnedRuntime<
            DedicatedThread<
                TThreadIdentity,
                TProperties...
            >,
            typename Binding::Callable,
            TManagedContextRouter,
            TExecutionContextProvider,
            TAtomicWord8Provider,
            TMutexProvider,
            TContextIndex,
            TExecutionContextCapacity
        >;

    };


    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider, std::size_t... TIndices>
    auto OwnedResourceTupleType(
        std::index_sequence<TIndices...>
    ) -> std::tuple<
        typename OwnedResourceType<
            typename StaticTopologyPlan<TTopology>::template Resource<TIndices>::Resource,
            TBindings,
            TManagedContextRouter,
            TExecutionContextProvider,
            TAtomicWord8Provider,
            TMutexProvider,
            StaticTopologyPlan<TTopology>::template Resource<TIndices>::FirstContextIndex,
            TTopology::ManagedExecutionContextCount
        >::Type...
    >;


    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TAtomicWord8Provider, class TMutexProvider>
    using OwnedResourceTuple = decltype(
        OwnedResourceTupleType<
            TTopology,
            TBindings,
            TManagedContextRouter,
            TExecutionContextProvider,
            TAtomicWord8Provider,
            TMutexProvider
        >(
            std::make_index_sequence<TTopology::ResourceCount>{}
        )
    );

} // ESPressio::Threading::Detail
