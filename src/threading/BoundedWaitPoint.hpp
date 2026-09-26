#pragma once

#include <cstdint>
#include <ESPressio_Platform.hpp>

namespace ESPressio::Threading {

    enum class BoundedWaitResult : std::uint8_t {
        Woken = 0,
        TimedOut = 1,
        ProviderFailure = 2
    };

    /// Allocation-free reusable bounded wait/wake capability for ordinary non-ISR callers.
    ///
    /// Threading owns the Platform Signal provider and translates its vocabulary so consumers
    /// never depend on Platform synchronization directly. The provider's latched Signal contract
    /// prevents a wake immediately preceding WaitFor from being lost.
    template<class TSignalProvider>
    class BoundedWaitPoint final {
        private:
            TSignalProvider _signal;

        public:
            BoundedWaitPoint() = default;
            BoundedWaitPoint(const BoundedWaitPoint&) = delete;
            BoundedWaitPoint& operator=(const BoundedWaitPoint&) = delete;
            BoundedWaitPoint(BoundedWaitPoint&&) = delete;
            BoundedWaitPoint& operator=(BoundedWaitPoint&&) = delete;

            [[nodiscard]] bool IsReady() noexcept {
                const auto result = _signal.Wait(
                    ESPressio::Platform::Synchronization::WaitTimeout::NoWait());
                return result == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut ||
                    result == ESPressio::Platform::Synchronization::SignalWaitResult::Signaled;
            }

            [[nodiscard]] BoundedWaitResult WaitFor(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                const auto result = _signal.Wait(timeout);
                if (result == ESPressio::Platform::Synchronization::SignalWaitResult::Signaled) {
                    return BoundedWaitResult::Woken;
                }
                if (result == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut) {
                    return BoundedWaitResult::TimedOut;
                }
                return BoundedWaitResult::ProviderFailure;
            }

            [[nodiscard]] bool Wake() noexcept {
                return _signal.Notify() ==
                    ESPressio::Platform::Synchronization::SignalNotifyResult::Succeeded;
            }
    };

} // ESPressio::Threading
