#pragma once

#include <cstddef>
#include <cstdint>

#include <ESPressio_Platform.hpp>

#include "TaskRecord.hpp"

namespace ESPressio::Threading::Detail {

    enum class ManagedContextWakeValidationResult : std::uint8_t {
        Ready = 0,
        ProviderFailure = 1
    };


    /// Owns one reusable targeted wake primitive per managed execution context.
    ///
    /// @tparam TContextCapacity Number of managed execution contexts in the static topology.
    /// @tparam TSignalProvider Concrete Platform Signal provider used for targeted wake delivery.
    template<std::size_t TContextCapacity, class TSignalProvider>
    class ManagedContextWakeSet;


    /// Empty-topology wake-set specialization retaining no Signal provider storage.
    ///
    /// @tparam TSignalProvider Concrete Platform Signal provider Type that would otherwise back wakes.
    template<class TSignalProvider>
    class ManagedContextWakeSet<0U, TSignalProvider> final {

        public:

            using ContextIndex = typename SmallestIndex<1U>::Type;

            static constexpr std::size_t ContextCapacity = 0U;


            ManagedContextWakeValidationResult Validate() noexcept {
                return ManagedContextWakeValidationResult::Ready;
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
    class ManagedContextWakeSet final {

        static_assert(
            TContextCapacity > 0U,
            "ManagedContextWakeSet requires positive execution-context capacity"
        );

        private:

            // Topology-owned targeted wake mechanisms.

            /// Exactly one reusable latched wake primitive per managed sequential execution context.
            TSignalProvider _signals[TContextCapacity];

        public:

            // Context index vocabulary.

            /// Smallest dense index Type able to address every managed execution context.
            using ContextIndex = typename SmallestIndex<TContextCapacity>::Type;

            /// Number of managed execution contexts represented by this wake set.
            static constexpr std::size_t ContextCapacity = TContextCapacity;


            // Bootstrap validation.

            /// Validates every statically constructed wake provider before managed execution starts.
            ManagedContextWakeValidationResult Validate() noexcept {
                for (std::size_t index = 0U; index < TContextCapacity; ++index) {
                    const auto result = _signals[index].Wait(
                        ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
                    );

                    if (
                        result != ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut &&
                        result != ESPressio::Platform::Synchronization::SignalWaitResult::Signaled
                    ) {
                        return ManagedContextWakeValidationResult::ProviderFailure;
                    }
                }

                return ManagedContextWakeValidationResult::Ready;
            }


            // Targeted wake.

            /// Latches the reusable wake primitive belonging to one managed execution context.
            ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                ContextIndex contextIndex
            ) noexcept {
                return _signals[contextIndex].Notify();
            }

            /// Blocks one managed execution context until its targeted signal fires or the wait expires.
            ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                ContextIndex contextIndex,
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _signals[contextIndex].Wait(
                    timeout
                );
            }

    };

} // ESPressio::Threading::Detail
