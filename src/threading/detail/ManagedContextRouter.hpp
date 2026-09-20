#pragma once

#include <cstddef>
#include <optional>

#include "ManagedContextWakeSet.hpp"
#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

    /// Routes targeted wakes and structural context inspection for one static Threading topology.
    ///
    /// @tparam TContextCapacity Number of managed execution contexts in the topology.
    /// @tparam TSignalProvider Concrete Platform Signal provider used by the wake set.
    template<std::size_t TContextCapacity, class TSignalProvider>
    class ManagedContextRouter;


    /// Empty-topology router specialization retaining no structural routing state.
    ///
    /// @tparam TSignalProvider Concrete Platform Signal provider Type selected by Bootstrap.
    template<class TSignalProvider>
    class ManagedContextRouter<0U, TSignalProvider> final {

        public:

            using WakeSet = ManagedContextWakeSet<
                0U,
                TSignalProvider
            >;

            using ContextIndex = typename SmallestIndex<1U>::Type;

            static constexpr std::size_t ContextCapacity = 0U;


            explicit ManagedContextRouter(
                WakeSet&
            ) noexcept {}


            template<class... TArguments>
            void BindTopology(
                TArguments&&...
            ) noexcept = delete;

            bool IsTopologyBound() const noexcept {
                return false;
            }

            std::optional<ContextIndex> CurrentContextIndex() const noexcept {
                return std::nullopt;
            }

            bool IsInterrupted(
                ContextIndex
            ) const noexcept {
                return true;
            }

            ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                ContextIndex
            ) noexcept = delete;

            ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                ContextIndex,
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept = delete;

    };


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

            explicit ManagedContextRouter(
                WakeSet& wakeSet
            ) noexcept :
                _wakeSet(&wakeSet),
                _topologyContext(nullptr),
                _currentContextIndex(nullptr),
                _isInterrupted(nullptr) {}

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


            // Structural topology binding.

            void BindTopology(
                const void* topologyContext,
                std::optional<ContextIndex> (*currentContextIndex)(const void*) noexcept,
                bool (*isInterrupted)(const void*, ContextIndex) noexcept
            ) noexcept {
                _topologyContext = topologyContext;
                _currentContextIndex = currentContextIndex;
                _isInterrupted = isInterrupted;
            }

            bool IsTopologyBound() const noexcept {
                return
                    _topologyContext != nullptr &&
                    _currentContextIndex != nullptr &&
                    _isInterrupted != nullptr;
            }


            // Context resolution.

            std::optional<ContextIndex> CurrentContextIndex() const noexcept {
                if (!IsTopologyBound()) {
                    return std::nullopt;
                }

                return _currentContextIndex(
                    _topologyContext
                );
            }

            bool IsInterrupted(
                ContextIndex contextIndex
            ) const noexcept {
                if (!IsTopologyBound()) {
                    return true;
                }

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
