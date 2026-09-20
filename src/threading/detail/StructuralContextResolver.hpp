#pragma once

#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

    /// Resolves the currently executing managed context structurally from owned topology resources.
    ///
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


            static std::optional<ContextIndex> ResolveCurrentThunk(
                const void*
            ) noexcept {
                return std::nullopt;
            }

            static bool IsInterruptedThunk(
                const void*,
                ContextIndex
            ) noexcept {
                return true;
            }

    };


    /// Defines the compile-time contract for `StructuralContextResolver`.
    /// @tparam TContextCapacity Number of managed execution contexts represented by the topology.
    template<std::size_t TContextCapacity, class... TResources>
    class StructuralContextResolver final {

        public:

            /// Compact Type used to identify one managed execution context.
            using ContextIndex =
                typename ExecutionContextIndexTraits<TContextCapacity>::Type;

        private:

            std::tuple<TResources*...> _resources;


            /// Defines the compile-time contract for `ResolveCurrentNext`.
            /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
            template<std::size_t TIndex>
            std::optional<ContextIndex> ResolveCurrentNext() const noexcept {
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

            /// Defines the compile-time contract for `IsInterruptedNext`.
            /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
            template<std::size_t TIndex>
            bool IsInterruptedNext(
                ContextIndex contextIndex
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

            explicit StructuralContextResolver(
                TResources&... resources
            ) noexcept :
                _resources(
                    &resources...
                ) {}


            // Managed-context router thunks.

            static std::optional<ContextIndex> ResolveCurrentThunk(
                const void* context
            ) noexcept {
                return static_cast<const StructuralContextResolver*>(
                    context
                )->ResolveCurrentNext<0U>();
            }

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
