#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../ThreadingComposition.hpp"
#include "DedicatedThreadOwnedRuntime.hpp"
#include "DedicatedWorkerOwnedRuntime.hpp"
#include "StaticTopologyPlan.hpp"
#include "TaskFacilityOwnedRuntime.hpp"
#include "TopologyResourceLookup.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `IsDedicatedThreadBindingFor`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TBinding Dedicated Thread binding Type being inspected.
    template<class TThreadIdentity, class TBinding>
    struct IsDedicatedThreadBindingFor {

        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsDedicatedThreadBindingFor`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TCallable Callable Type being dispatched or adapted.
    template<class TThreadIdentity, class TCallable>
    struct IsDedicatedThreadBindingFor<
        TThreadIdentity,
        DedicatedThreadBinding<TThreadIdentity, TCallable>
    > {

        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `DedicatedThreadBindingCount`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
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


    /// Defines the compile-time contract for `DedicatedThreadBindingType`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingType;


    /// Defines the compile-time contract for `DedicatedThreadBindingTypeFromTuple`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    template<class TThreadIdentity, class TBindings>
    struct DedicatedThreadBindingTypeFromTuple;


    /// Defines the compile-time contract for `DedicatedThreadBindingTypeFromTuple`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingTypeFromTuple<
        TThreadIdentity,
        std::tuple<TBindings...>
    > {

        using Type = typename DedicatedThreadBindingType<
            TThreadIdentity,
            TBindings...
        >::Type;

    };


    /// Defines the compile-time contract for `DedicatedThreadBindingType`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    template<class TThreadIdentity>
    struct DedicatedThreadBindingType<TThreadIdentity> {

        using Type = void;

    };


    /// Defines the compile-time contract for `DedicatedThreadBindingType`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TFirstBinding First binding Type in the recursive binding search.
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


    /// Defines the compile-time contract for `ResourceBindingIsValid`.
    /// @tparam TResource Topology resource Type being classified or inspected.
    template<class TResource, class... TBindings>
    struct ResourceBindingIsValid {

        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `ResourceBindingIsValid`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    template<class TThreadIdentity, class... TProperties, class... TBindings>
    struct ResourceBindingIsValid<
        DedicatedThread<TThreadIdentity, TProperties...>,
        TBindings...
    > {

        static constexpr bool Value =
            DedicatedThreadBindingCount<
                TThreadIdentity,
                TBindings...
            >::Value == 1U;

    };


    /// Defines the compile-time contract for `BindingMatchesDeclaredThread`.
    /// @tparam TBinding Dedicated Thread binding Type being inspected.
    template<class TBinding, class... TResources>
    struct BindingMatchesDeclaredThread {

        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `BindingMatchesDeclaredThread`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TCallable Callable Type being dispatched or adapted.
    template<class TThreadIdentity, class TCallable, class... TResources>
    struct BindingMatchesDeclaredThread<
        DedicatedThreadBinding<TThreadIdentity, TCallable>,
        TResources...
    > {

        static constexpr bool Value =
            (
                IsDedicatedThreadIdentity<
                    TResources,
                    TThreadIdentity
                >::Value ||
                ... ||
                false
            );

    };


    /// Defines the compile-time contract for `IsTaskExecutionResource`.
    /// @tparam TResource Topology resource Type being classified or inspected.
    template<class TResource>
    struct IsTaskExecutionResource {

        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsTaskExecutionResource`.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    /// @tparam TRecordCapacity Task-record capacity declaration Type or bounded capacity.
    /// @tparam TCallableCapacity Callable-storage capacity declaration Type or byte capacity.
    /// @tparam TResultCapacity Result-storage capacity declaration Type or byte capacity.
    /// @tparam TWorkers Declared Worker set Type.
    template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers>
    struct IsTaskExecutionResource<
        TaskExecutionFacility<
            TPoolIdentity,
            TRecordCapacity,
            TCallableCapacity,
            TResultCapacity,
            TWorkers
        >
    > {

        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `IsTaskExecutionResource`.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    template<class TTaskIdentity, class... TProperties>
    struct IsTaskExecutionResource<
        DedicatedWorkerLease<
            TTaskIdentity,
            TProperties...
        >
    > {

        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `IsDedicatedThreadBinding`.
    /// @tparam TBinding Dedicated Thread binding Type being inspected.
    template<class TBinding>
    struct IsDedicatedThreadBinding {

        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsDedicatedThreadBinding`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TCallable Callable Type being dispatched or adapted.
    template<class TThreadIdentity, class TCallable>
    struct IsDedicatedThreadBinding<
        DedicatedThreadBinding<TThreadIdentity, TCallable>
    > {

        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `ValidDedicatedThreadBindings`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    template<class TTopology, class TBindings>
    struct ValidDedicatedThreadBindings;


    template<class... TResources, class... TBindings>
    struct ValidDedicatedThreadBindings<
        ThreadingTopology<TResources...>,
        std::tuple<TBindings...>
    > {

        static constexpr bool EveryThreadBound =
            (
                ResourceBindingIsValid<
                    TResources,
                    TBindings...
                >::Value &&
                ... &&
                true
            );

        static constexpr bool EveryBindingDeclared =
            (
                (
                    IsDedicatedThreadBinding<TBindings>::Value &&
                    BindingMatchesDeclaredThread<
                        TBindings,
                        TResources...
                    >::Value
                ) &&
                ... &&
                true
            );

        static constexpr bool Value =
            EveryThreadBound &&
            EveryBindingDeclared;

    };


    /// Defines the compile-time contract for `OwnedResourceType`.
    /// @tparam TDeclaration Static topology declaration Type being realized.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TDeclaration, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType;


    /// Defines the compile-time contract for `OwnedResourceType`.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    /// @tparam TRecordCapacity Task-record capacity declaration Type or bounded capacity.
    /// @tparam TCallableCapacity Callable-storage capacity declaration Type or byte capacity.
    /// @tparam TResultCapacity Result-storage capacity declaration Type or byte capacity.
    /// @tparam TWorkers Declared Worker set Type.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TPoolIdentity, class TRecordCapacity, class TCallableCapacity, class TResultCapacity, class TWorkers, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
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
            TMutexProvider,
            TContextIndex,
            TExecutionContextCapacity
        >;

    };


    /// Defines the compile-time contract for `OwnedResourceType`.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TTaskIdentity, class... TProperties, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType<
        DedicatedWorkerLease<TTaskIdentity, TProperties...>,
        std::tuple<TBindings...>,
        TManagedContextRouter,
        TExecutionContextProvider,
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
            TMutexProvider,
            TContextIndex,
            TExecutionContextCapacity
        >;

    };


    /// Defines the compile-time contract for `OwnedResourceType`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TThreadIdentity, class... TProperties, class... TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t TContextIndex, std::size_t TExecutionContextCapacity>
    struct OwnedResourceType<
        DedicatedThread<TThreadIdentity, TProperties...>,
        std::tuple<TBindings...>,
        TManagedContextRouter,
        TExecutionContextProvider,
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
            TMutexProvider,
            TContextIndex,
            TExecutionContextCapacity
        >;

    };


    /// Defines the compile-time contract for `OwnedResourceTupleType`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider, std::size_t... TIndices>
    auto OwnedResourceTupleType(
        std::index_sequence<TIndices...>
    ) -> std::tuple<
        typename OwnedResourceType<
            typename StaticTopologyPlan<TTopology>::template Resource<TIndices>::Resource,
            TBindings,
            TManagedContextRouter,
            TExecutionContextProvider,
            TMutexProvider,
            StaticTopologyPlan<TTopology>::template Resource<TIndices>::FirstContextIndex,
            TTopology::ManagedExecutionContextCount
        >::Type...
    >;


    /// Defines the compile-time contract for `OwnedResourceTuple`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    template<class TTopology, class TBindings, class TManagedContextRouter, class TExecutionContextProvider, class TMutexProvider>
    using OwnedResourceTuple = decltype(
        OwnedResourceTupleType<
            TTopology,
            TBindings,
            TManagedContextRouter,
            TExecutionContextProvider,
            TMutexProvider
        >(
            std::make_index_sequence<TTopology::ResourceCount>{}
        )
    );

} // ESPressio::Threading::Detail
