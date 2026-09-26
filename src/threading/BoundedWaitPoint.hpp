#pragma once

#include <cstdint>
#include <ESPressio_Platform.hpp>
#include "ThreadingTypes.hpp"
#include "detail/MonotonicWaitBudget.hpp"

namespace ESPressio::Threading {

enum class BoundedWaitResult : std::uint8_t { Woken = 0, TimedOut = 1, ProviderFailure = 2 };

/// Allocation-free reusable bounded wait/wake capability for ordinary non-ISR callers.
template<class TSignalProvider>
class BoundedWaitPoint final {
    TSignalProvider _signal;

    [[nodiscard]] BoundedWaitResult WaitWithBudget(const Detail::MonotonicWaitBudget& budget) noexcept {
        const auto remaining = budget.Remaining();
        if (remaining.IsNoWait()) return BoundedWaitResult::TimedOut;
        const auto result = _signal.Wait(remaining);
        if (result == ESPressio::Platform::Synchronization::SignalWaitResult::Signaled) return BoundedWaitResult::Woken;
        if (result == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut) return BoundedWaitResult::TimedOut;
        return BoundedWaitResult::ProviderFailure;
    }
public:
    BoundedWaitPoint() = default;
    BoundedWaitPoint(const BoundedWaitPoint&) = delete;
    BoundedWaitPoint& operator=(const BoundedWaitPoint&) = delete;
    BoundedWaitPoint(BoundedWaitPoint&&) = delete;
    BoundedWaitPoint& operator=(BoundedWaitPoint&&) = delete;

    [[nodiscard]] bool IsReady() noexcept {
        const auto result = _signal.Wait(ESPressio::Platform::Synchronization::WaitTimeout::NoWait());
        return result == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut ||
            result == ESPressio::Platform::Synchronization::SignalWaitResult::Signaled;
    }
    [[nodiscard]] BoundedWaitResult WaitFor(Duration duration) noexcept {
        return WaitWithBudget(Detail::MonotonicWaitBudget::For(duration));
    }
    [[nodiscard]] BoundedWaitResult WaitUntil(MonotonicTimestamp deadline) noexcept {
        return WaitWithBudget(Detail::MonotonicWaitBudget::Until(deadline));
    }
    [[nodiscard]] bool Wake() noexcept {
        return _signal.Notify() == ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled;
    }
};

} // ESPressio::Threading
