#pragma once

#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

    /// Resolves current-context identity and interruption state structurally across topology-owned resources.
    /// @tparam TContextCapacity Number of managed execution contexts in the topology.
    /// @tparam TResources Concrete statically owned runtime resource Types participating in resolution.
    template<std::size_t TContextCapacity, class... TResources>
    class StructuralContextResolver;


    /// Empty-topology resolver specialization retaining no resource references.
    template<>
    class StructuralContextResolver<0U> final {

        public:

            /// Compact Type used to identify one managed execution context.
            using ContextIndex = typename SmallestIndex<1U>::Type;


            /// Reports that an empty topology has no currently executing managed context.
            static std::optional<ContextIndex> ResolveCurrentThunk(
                const void*
            ) noexcept {
                return std::nullopt;
            }

            /// Conservatively reports interruption for any impossible context query against an empty topology.
            static bool IsInterruptedThunk(
                const void*,
                ContextIndex
            ) noexcept {
                return true;
            }

    };


    /// Resolves managed-context identity and interruption structurally across topology-owned runtime resources.
    /// @tparam TContextCapacity Number of managed execution contexts represented by the topology.
    /// @tparam TResources Concrete topology-owned runtime resource Types searched by the resolver.
    template<std::size_t TContextCapacity, class... TResources>
    class StructuralContextResolver final {

        private:

            // Internal context vocabulary.

            /// Internal compact Type used while resolving managed execution contexts.
            using ContextIndexStorage =
                typename ExecutionContextIndexTraits<TContextCapacity>::Type;


            // Structural resource bindings.

            /// Non-owning pointers to the topology-owned runtime resources searched for context identity/interruption.
            std::tuple<TResources*...> _resources;


            // Recursive structural resolution.

            /// Resolves the current managed execution context by scanning topology resources in declaration order.
            /// @tparam TIndex Compile-time resource index currently being inspected.
            template<std::size_t TIndex>
            std::optional<ContextIndexStorage> ResolveCurrentNext() const noexcept {
                if constexpr (
                    TIndex == sizeof...(TResources)
                ) {
                    return std::nullopt;
                } else {
                    const auto contextIndex = std::get<TIndex>(
                        _resources
                    )->CurrentContextIndex();

                    if (contextIndex.has_value()) {
                        return contextIndex;
                    }

                    return ResolveCurrentNext<TIndex + 1U>();
                }
            }

            /// Resolves whether one managed execution context carries an authoritative interruption request.
            /// @tparam TIndex Compile-time resource index currently being inspected.
            template<std::size_t TIndex>
            bool IsInterruptedNext(
                ContextIndexStorage contextIndex
            ) const noexcept {
                if constexpr (
                    TIndex == sizeof...(TResources)
                ) {
                    return false;
                } else {
                    auto* resource = std::get<TIndex>(
                        _resources
                    );

                    if (
                        resource->IsContextInterrupted(
                            contextIndex
                        )
                    ) {
                        return true;
                    }

                    return IsInterruptedNext<TIndex + 1U>(
                        contextIndex
                    );
                }
            }

        public:

            // Public context vocabulary.

            /// Compact Type used to identify one managed execution context.
            using ContextIndex = ContextIndexStorage;


            // Construction.

            /// Binds the resolver to the address-stable topology-owned runtime resources.
            explicit StructuralContextResolver(
                TResources&... resources
            ) noexcept :
                _resources(
                    &resources...
                ) {}


            // Managed-context router thunks.

            /// Type-erased router bridge that resolves the currently executing managed context.
            static std::optional<ContextIndex> ResolveCurrentThunk(
                const void* context
            ) noexcept {
                return static_cast<const StructuralContextResolver*>(
                    context
                )->ResolveCurrentNext<0U>();
            }

            /// Type-erased router bridge that resolves authoritative interruption for one managed context.
            static bool IsInterruptedThunk(
                const void* context,
                ContextIndex contextIndex
            ) noexcept {
                return static_cast<const StructuralContextResolver*>(
                    context
                )->IsInterruptedNext<0U>(
                    contextIndex
                );
            }

    };

} // ESPressio::Threading::Detail
