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

    /// Determines whether one binding targets a specific Dedicated Thread identity.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread being matched.
    /// @tparam TBinding Dedicated Thread binding Type being inspected.
    template<class TThreadIdentity, class TBinding>
    struct IsDedicatedThreadBindingFor {

        /// Indicates whether this compile-time trait specialization matches its requested condition.
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

        /// Indicates whether this compile-time trait specialization matches its requested condition.
        static constexpr bool Value = true;

    };


    /// Counts bindings that target one Dedicated Thread identity.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread being counted.
    /// @tparam TBindings Binding Types searched for matching Thread identities.
    template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingCount {

        /// Number of matching Dedicated Thread bindings found in the supplied binding pack.
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


    /// Resolves the binding Type associated with one Dedicated Thread identity.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread being resolved.
    /// @tparam TBindings Binding Types searched for the matching Thread identity.
    template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingType;


    /// Resolves one Dedicated Thread binding from an application binding tuple.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread being resolved.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    template<class TThreadIdentity, class TBindings>
    struct DedicatedThreadBindingTypeFromTuple;


    /// Resolves one Dedicated Thread binding from a concrete tuple specialization.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread being resolved.
    /// @tparam TBindings Binding Types contained by the application tuple.
    template<class TThreadIdentity, class... TBindings>
    struct DedicatedThreadBindingTypeFromTuple<
        TThreadIdentity,
        std::tuple<TBindings...>
    > {

        /// Resolved Type produced by this compile-time helper.
        using Type = typename DedicatedThreadBindingType<
            TThreadIdentity,
            TBindings...
        >::Type;

    };


    /// Terminates binding lookup when no matching Dedicated Thread binding exists.
    /// @tparam TThreadIdentity Semantic identity Type whose binding lookup reached the empty tail.
    template<class TThreadIdentity>
    struct DedicatedThreadBindingType<TThreadIdentity> {

        /// Resolved Type produced by this compile-time helper.
        using Type = void;

    };


    /// Resolves a Dedicated Thread binding recursively from the supplied binding pack.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread being resolved.
    /// @tparam TFirstBinding First binding Type inspected by this recursive specialization.
    /// @tparam TRestBindings Remaining binding Types searched when the first binding does not match.
    template<class TThreadIdentity, class TFirstBinding, class... TRestBindings>
    struct DedicatedThreadBindingType<
        TThreadIdentity,
        TFirstBinding,
        TRestBindings...
    > {

        /// Resolved Type produced by this compile-time helper.
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


    /// Validates application binding requirements for one topology resource.
    /// @tparam TResource Topology resource Type whose binding requirements are checked.
    /// @tparam TBindings Application Dedicated Thread binding Types available to the topology.
    template<class TResource, class... TBindings>
    struct ResourceBindingIsValid {

        /// Indicates whether this compile-time trait specialization matches its requested condition.
        static constexpr bool Value = true;

    };


    /// Validates that a Dedicated Thread declaration has exactly one matching application binding.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread being validated.
    /// @tparam TProperties Compile-time execution properties declared for the Dedicated Thread.
    /// @tparam TBindings Application binding Types searched for the matching Thread identity.
    template<class TThreadIdentity, class... TProperties, class... TBindings>
    struct ResourceBindingIsValid<
        DedicatedThread<TThreadIdentity, TProperties...>,
        TBindings...
    > {

        /// Indicates whether this compile-time trait specialization matches its requested condition.
        static constexpr bool Value =
            DedicatedThreadBindingCount<
                TThreadIdentity,
                TBindings...
            >::Value == 1U;

    };


    /// Determines whether one supplied binding targets a Dedicated Thread declared by the topology.
    /// @tparam TBinding Dedicated Thread binding Type being inspected.
    /// @tparam TResources Topology resource Types searched for the binding's Thread identity.
    template<class TBinding, class... TResources>
    struct BindingMatchesDeclaredThread {

        /// Indicates whether this compile-time trait specialization matches its requested condition.
        static constexpr bool Value = false;

    };


    /// Matches one concrete Dedicated Thread binding against the topology resource pack.
    /// @tparam TThreadIdentity Semantic identity Type carried by the binding.
    /// @tparam TCallable Callable Type carried by the binding.
    /// @tparam TResources Topology resource Types searched for the bound Thread identity.
    template<class TThreadIdentity, class TCallable, class... TResources>
    struct BindingMatchesDeclaredThread<
        DedicatedThreadBinding<TThreadIdentity, TCallable>,
        TResources...
    > {

        /// Indicates whether this compile-time trait specialization matches its requested condition.
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

        /// Indicates whether this compile-time trait specialization matches its requested condition.
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

        /// Indicates whether this compile-time trait specialization matches its requested condition.
        static constexpr bool Value = true;

    };


    /// Identifies a Dedicated Worker lease as a Task-execution resource.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    /// @tparam TProperties Compile-time capacities and execution properties declared by the lease.
    template<class TTaskIdentity, class... TProperties>
    struct IsTaskExecutionResource<
        DedicatedWorkerLease<
            TTaskIdentity,
            TProperties...
        >
    > {

        /// Indicates whether this compile-time trait specialization matches its requested condition.
        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `IsDedicatedThreadBinding`.
    /// @tparam TBinding Dedicated Thread binding Type being inspected.
    template<class TBinding>
    struct IsDedicatedThreadBinding {

        /// Indicates whether this compile-time trait specialization matches its requested condition.
        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsDedicatedThreadBinding`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TCallable Callable Type being dispatched or adapted.
    template<class TThreadIdentity, class TCallable>
    struct IsDedicatedThreadBinding<
        DedicatedThreadBinding<TThreadIdentity, TCallable>
    > {

        /// Indicates whether this compile-time trait specialization matches its requested condition.
        static constexpr bool Value = true;

    };


    /// Defines the compile-time contract for `ValidDedicatedThreadBindings`.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    template<class TTopology, class TBindings>
    struct ValidDedicatedThreadBindings;


    /// Validates all Dedicated Thread declarations and supplied bindings as a complete topology relationship.
    /// @tparam TResources Topology resource Types whose Dedicated Threads require bindings.
    /// @tparam TBindings Application binding Types that must each target a declared Dedicated Thread.
    template<class... TResources, class... TBindings>
    struct ValidDedicatedThreadBindings<
        ThreadingTopology<TResources...>,
        std::tuple<TBindings...>
    > {

        /// Whether every declared Dedicated Thread has exactly one matching binding.
        static constexpr bool EveryThreadBound =
            (
                ResourceBindingIsValid<
                    TResources,
                    TBindings...
                >::Value &&
                ... &&
                true
            );

        /// Whether every supplied Dedicated Thread binding targets a declared thread.
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

        /// Indicates whether this compile-time trait specialization matches its requested condition.
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


    /// Maps one ordinary Task facility declaration to its concrete statically owned runtime Type.
    /// @tparam TPoolIdentity Semantic identity Type of the Task pool.
    /// @tparam TRecordCapacity Task-record capacity declaration Type or bounded capacity.
    /// @tparam TCallableCapacity Callable-storage capacity declaration Type or byte capacity.
    /// @tparam TResultCapacity Result-storage capacity declaration Type or byte capacity.
    /// @tparam TWorkers Declared Worker set Type.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TContextIndex Dense topology execution-context index assigned to the resource.
    /// @tparam TBindings Application Dedicated Thread binding Types carried through the uniform resource-mapping interface.
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

        /// Resolved Type produced by this compile-time helper.
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


    /// Maps one Dedicated Worker lease declaration to its concrete statically owned runtime Type.
    /// @tparam TTaskIdentity Semantic identity Type of the dedicated Task.
    /// @tparam TProperties Compile-time capacities and execution properties declared by the Dedicated Worker lease.
    /// @tparam TBindings Application Dedicated Thread binding Types carried through the uniform resource-mapping interface.
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

        /// Resolved Type produced by this compile-time helper.
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


    /// Maps one Dedicated Thread declaration and its binding to the concrete statically owned runtime Type.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TProperties Compile-time execution properties declared by the Dedicated Thread.
    /// @tparam TBindings Application binding Types used to resolve the Thread callable.
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

        /// Application callable-binding Type matched to a Dedicated Thread.
        using Binding = typename DedicatedThreadBindingType<
            TThreadIdentity,
            TBindings...
        >::Type;

        /// Resolved Type produced by this compile-time helper.
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


    /// Constructs the tuple Type containing every concrete topology-owned runtime resource.
    /// @tparam TTopology Compile-time Threading topology being realized.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TIndices Compile-time topology resource indices expanded into the resulting tuple Type.
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


    /// Resolves the concrete owned-resource tuple Type for a complete Threading topology.
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
