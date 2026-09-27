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

    /// Shared Threading capability supplying fixed reusable ordinary-context wait/wake slots.
    struct BoundedWaitWake final : Framework::SharedCapability<Domain> {
    };

    /// Number of independent wait/wake slots supplied by a BoundedWaitWake provider.
    struct BoundedWaitWakeCapacity final : Framework::Property<BoundedWaitWake, std::size_t> {
    };

    /// Result of a bounded wait or wake operation.
    enum class BoundedWaitWakeResult : std::uint8_t {
        Woken = 0U,
        TimedOut = 1U,
        ProviderFailure = 2U,
        InvalidSlot = 3U
    };

    /// Fixed-capacity ordinary-context wait/wake provider.
    /// @tparam TSignalProvider Platform Signal provider stored once per wait slot.
    /// @tparam TCapacity Number of independent wait/wake slots supplied by this provider.
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
        static_assert(
            TCapacity > 0U,
            "BoundedWaitWakeProvider capacity must be non-zero"
        );

    private:
        // Fixed synchronization storage.

        /// Platform Signal provider owned for every bounded wait slot.
        std::array<TSignalProvider, TCapacity> _signals{};

        // Internal wait implementation.

        /// Waits on one slot using a canonical monotonic finite wait budget.
        [[nodiscard]] BoundedWaitWakeResult WaitWithBudget(
            std::size_t slot,
            const Detail::MonotonicWaitBudget& budget
        ) noexcept {
            if (slot >= TCapacity) {
                return BoundedWaitWakeResult::InvalidSlot;
            }

            const auto remaining = budget.Remaining();
            if (remaining.IsNoWait()) {
                return BoundedWaitWakeResult::TimedOut;
            }

            const auto result = _signals[slot].Wait(remaining);
            if (result == ESPressio::Platform::Synchronization::SignalWaitResult::Signaled) {
                return BoundedWaitWakeResult::Woken;
            }
            if (result == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut) {
                return BoundedWaitWakeResult::TimedOut;
            }
            return BoundedWaitWakeResult::ProviderFailure;
        }

    public:
        // Compile-time capacity.

        /// Number of independent wait/wake slots owned by this provider.
        static constexpr std::size_t Capacity = TCapacity;

        // Construction and ownership.

        /// Constructs all bounded wait/wake slots in place.
        BoundedWaitWakeProvider() = default;

        /// Prevents copying provider-owned synchronization state.
        BoundedWaitWakeProvider(const BoundedWaitWakeProvider&) = delete;

        /// Prevents copy assignment of provider-owned synchronization state.
        BoundedWaitWakeProvider& operator=(const BoundedWaitWakeProvider&) = delete;

        /// Prevents moving provider-owned synchronization state.
        BoundedWaitWakeProvider(BoundedWaitWakeProvider&&) = delete;

        /// Prevents move assignment of provider-owned synchronization state.
        BoundedWaitWakeProvider& operator=(BoundedWaitWakeProvider&&) = delete;

        // Finite waiting.

        /// Waits for a wake notification on slot for at most duration.
        [[nodiscard]] BoundedWaitWakeResult WaitFor(
            std::size_t slot,
            Duration duration
        ) noexcept {
            return WaitWithBudget(
                slot,
                Detail::MonotonicWaitBudget::For(duration)
            );
        }

        /// Waits for a wake notification on slot until deadline.
        [[nodiscard]] BoundedWaitWakeResult WaitUntil(
            std::size_t slot,
            MonotonicTimestamp deadline
        ) noexcept {
            return WaitWithBudget(
                slot,
                Detail::MonotonicWaitBudget::Until(deadline)
            );
        }

        // Wake publication.

        /// Publishes a wake notification to one bounded slot.
        [[nodiscard]] BoundedWaitWakeResult Wake(std::size_t slot) noexcept {
            if (slot >= TCapacity) {
                return BoundedWaitWakeResult::InvalidSlot;
            }

            const auto result = _signals[slot].Notify();
            if (result == ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled) {
                return BoundedWaitWakeResult::Woken;
            }
            return BoundedWaitWakeResult::ProviderFailure;
        }
    };

} // ESPressio::Threading
