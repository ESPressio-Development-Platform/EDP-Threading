#pragma once

#include <cstddef>
#include <optional>

#include "ManagedContextWakeSet.hpp"
#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

    template<std::size_t TContextCapacity, class TSignalProvider>
    class ManagedContextRouter final {

        public:

            using WakeSet = ManagedContextWakeSet<
                TContextCapacity,
                TSignalProvider
            >;

            using ContextIndex =
                typename ExecutionContextIndexTraits<TContextCapacity>::Type;

            static constexpr std::size_t ContextCapacity = TContextCapacity;

        private:

            /// Topology-owned dense targeted wake mechanisms.
            WakeSet* _wakeSet;

            /// Non-owning topology realization used for structural current-context/interruption scans.
            const void* _topologyContext;

            /// Structural resolver for the currently executing managed context.
            std::optional<ContextIndex> (*_currentContextIndex)(const void*) noexcept;

            /// Structural authoritative interruption resolver for one managed context.
            bool (*_isInterrupted)(
                const void*,
                ContextIndex
            ) noexcept;

        public:

            ManagedContextRouter(
                WakeSet& wakeSet,
                const void* topologyContext,
                std::optional<ContextIndex> (*currentContextIndex)(const void*) noexcept,
                bool (*isInterrupted)(const void*, ContextIndex) noexcept
            ) noexcept :
                _wakeSet(&wakeSet),
                _topologyContext(topologyContext),
                _currentContextIndex(currentContextIndex),
                _isInterrupted(isInterrupted) {}


            // Context resolution.

            std::optional<ContextIndex> CurrentContextIndex() const noexcept {
                return _currentContextIndex(
                    _topologyContext
                );
            }

            bool IsInterrupted(
                ContextIndex contextIndex
            ) const noexcept {
                return _isInterrupted(
                    _topologyContext,
                    contextIndex
                );
            }


            // Targeted wake routing.

            ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                ContextIndex contextIndex
            ) noexcept {
                return _wakeSet->Wake(
                    contextIndex
                );
            }

            ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                ContextIndex contextIndex,
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _wakeSet->Wait(
                    contextIndex,
                    timeout
                );
            }

    };

} // ESPressio::Threading::Detail
