#pragma once

#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

    template<std::size_t TContextCapacity, class... TResources>
    class StructuralContextResolver final {

        public:

            using ContextIndex =
                typename ExecutionContextIndexTraits<TContextCapacity>::Type;

        private:

            std::tuple<TResources*...> _resources;


            template<std::size_t TIndex>
            std::optional<ContextIndex> ResolveCurrentNext() const noexcept {
                if constexpr (
                    TIndex == sizeof...(TResources)
                ) {
                    return std::nullopt;
                } else {
                    ContextIndex contextIndex = 0U;

                    if (
                        std::get<TIndex>(
                            _resources
                        )->TryResolveCurrentContext(
                            contextIndex
                        )
                    ) {
                        return contextIndex;
                    }

                    return ResolveCurrentNext<TIndex + 1U>();
                }
            }

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
