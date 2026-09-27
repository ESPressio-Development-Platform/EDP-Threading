#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <ESPressio_Platform.hpp>
#include <ESPressio_System.hpp>

#include "ThreadingComposition.hpp"
#include "ThreadingTypes.hpp"
#include "detail/MonotonicWaitBudget.hpp"

namespace ESPressio::Threading {

/// Shared Threading capability supplying a fixed number of reusable ordinary-context
/// bounded wait/wake slots. Consumers retain only the provider reference resolved by
/// Composition; native Signal mechanics remain owned by Threading/Platform providers.
struct BoundedWaitWake final : Framework::SharedCapability<Domain> {};

/// Number of independent wait/wake slots supplied by a BoundedWaitWake provider.
struct BoundedWaitWakeCapacity final : Framework::Property<BoundedWaitWake, std::size_t> {};

enum class BoundedWaitWakeResult : std::uint8_t {
    Woken = 0U,
    TimedOut = 1U,
    ProviderFailure = 2U,
    InvalidSlot = 3U
};

template<class TSignalProvider, std::size_t TCapacity>
class BoundedWaitWakeProvider final : public Framework::Provider<
    Domain,
    Framework::Offers<
        Framework::Offer<
            BoundedWaitWake,
            Framework::PropertyValue<BoundedWaitWakeCapacity, TCapacity>
        >
    >
> {
    static_assert(TCapacity > 0U, "BoundedWaitWakeProvider capacity must be non-zero");

    std::array<TSignalProvider, TCapacity> _signals{};

    [[nodiscard]] BoundedWaitWakeResult WaitWithBudget(
        std::size_t slot,
        const Detail::MonotonicWaitBudget& budget
    ) noexcept {
        if (slot >= TCapacity) return BoundedWaitWakeResult::InvalidSlot;
        const auto remaining = budget.Remaining();
        if (remaining.IsNoWait()) return BoundedWaitWakeResult::TimedOut;

        const auto result = _signals[slot].Wait(remaining);
        if (result == ESPressio::Platform::Synchronization::SignalWaitResult::Signaled)
            return BoundedWaitWakeResult::Woken;
        if (result == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut)
            return BoundedWaitWakeResult::TimedOut;
        return BoundedWaitWakeResult::ProviderFailure;
    }

public:
    static constexpr std::size_t Capacity = TCapacity;

    BoundedWaitWakeProvider() = default;
    BoundedWaitWakeProvider(const BoundedWaitWakeProvider&) = delete;
    BoundedWaitWakeProvider& operator=(const BoundedWaitWakeProvider&) = delete;
    BoundedWaitWakeProvider(BoundedWaitWakeProvider&&) = delete;
    BoundedWaitWakeProvider& operator=(BoundedWaitWakeProvider&&) = delete;

    [[nodiscard]] BoundedWaitWakeResult WaitFor(std::size_t slot, Duration duration) noexcept {
        return WaitWithBudget(slot, Detail::MonotonicWaitBudget::For(duration));
    }

    [[nodiscard]] BoundedWaitWakeResult WaitUntil(
        std::size_t slot,
        MonotonicTimestamp deadline
    ) noexcept {
        return WaitWithBudget(slot, Detail::MonotonicWaitBudget::Until(deadline));
    }

    [[nodiscard]] BoundedWaitWakeResult Wake(std::size_t slot) noexcept {
        if (slot >= TCapacity) return BoundedWaitWakeResult::InvalidSlot;
        const auto result = _signals[slot].Notify();
        if (result == ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled)
            return BoundedWaitWakeResult::Woken;
        return BoundedWaitWakeResult::ProviderFailure;
    }
};

} // ESPressio::Threading
