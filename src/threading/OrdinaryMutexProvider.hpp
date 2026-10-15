#pragma once

#include <cstdint>

#include <ESPressio_Platform.hpp>
#include <ESPressio_System.hpp>

#include "ThreadingComposition.hpp"

namespace ESPressio::Threading {

    /// Keyed ordinary-context non-recursive mutual-exclusion capability.
    /// @tparam TIdentity Semantic identity separating independent mutex instances.
    template<class TIdentity>
    struct OrdinaryMutex final : Framework::ExclusiveCapability<Domain> {
        using Identity = TIdentity;
    };


    /// Result of ordinary-context mutex acquisition, including bounded wait outcomes.
    enum class OrdinaryMutexAcquireResult : std::uint8_t {
        Acquired = 0,
        ProviderFailure = 1,
        TimedOut = 2
    };


    /// Result of releasing a keyed ordinary-context mutex.
    enum class OrdinaryMutexReleaseResult : std::uint8_t {
        Released = 0,
        ProviderFailure = 1
    };


    /// Threading-owned adapter exposing one Platform non-recursive Mutex under a semantic identity.
    ///
    /// This capability is ordinary-context only. It deliberately exposes no ISR surface and no
    /// recursive acquisition semantics. The wrapped Platform provider remains the native owner.
    ///
    /// @tparam TIdentity Semantic identity distinguishing this mutex from unrelated consumers.
    /// @tparam TMutexProvider Concrete Platform Mutex provider owned by this adapter.
    template<class TIdentity, class TMutexProvider>
    class OrdinaryMutexProvider final : public Framework::Provider<
        Domain,
        Framework::Offers<
            Framework::Offer<OrdinaryMutex<TIdentity>>
        >
    > {

        private:

            /// Validated Platform non-recursive Mutex provider contract.
            using MutexTraits = ESPressio::Platform::Synchronization::Detail::MutexProviderTraits<
                TMutexProvider
            >;

            static_assert(
                sizeof(MutexTraits) > 0U,
                "OrdinaryMutexProvider requires a valid Platform non-recursive Mutex provider"
            );

            /// Platform-owned synchronization mechanism retained for this semantic mutex identity.
            TMutexProvider _mutex;

        public:

            /// Semantic identity Type of this independent ordinary mutex.
            using Identity = TIdentity;

            OrdinaryMutexProvider() = default;
            OrdinaryMutexProvider(const OrdinaryMutexProvider&) = delete;
            OrdinaryMutexProvider& operator =(const OrdinaryMutexProvider&) = delete;
            OrdinaryMutexProvider(OrdinaryMutexProvider&&) = delete;
            OrdinaryMutexProvider& operator =(OrdinaryMutexProvider&&) = delete;

            /// Acquires a keyed mutex using one caller-supplied native relative wait budget.
            ///
            /// The owner of a canonical Monotonic deadline must compute the remaining budget
            /// immediately before this call and recheck EDP-Clock on return. This adapter
            /// does not create another timebase or silently renew a deadline.
            ///
            /// @param timeout Native wait mechanism policy, including zero-wait and finite waits.
            [[nodiscard]] OrdinaryMutexAcquireResult Acquire(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                const auto result = _mutex.Acquire(timeout);

                switch (result) {
                    case ESPressio::Platform::Synchronization::LockAcquireResult::Acquired:
                        return OrdinaryMutexAcquireResult::Acquired;

                    case ESPressio::Platform::Synchronization::LockAcquireResult::TimedOut:
                        return OrdinaryMutexAcquireResult::TimedOut;

                    case ESPressio::Platform::Synchronization::LockAcquireResult::ProviderFailure:
                        return OrdinaryMutexAcquireResult::ProviderFailure;

                    default:
                        return OrdinaryMutexAcquireResult::ProviderFailure;
                }
            }

            /// Attempts acquisition without waiting on another mutex owner.
            [[nodiscard]] OrdinaryMutexAcquireResult TryAcquire() noexcept {
                return Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
                );
            }

            /// Acquires the mutex indefinitely from ordinary execution context.
            ///
            /// Maintained for existing higher-domain callers that explicitly require Forever.
            [[nodiscard]] OrdinaryMutexAcquireResult Acquire() noexcept {
                return Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                );
            }

            /// Releases the mutex owned by the current ordinary execution context.
            [[nodiscard]] OrdinaryMutexReleaseResult Release() noexcept {
                return _mutex.Release() == ESPressio::Platform::Synchronization::LockReleaseResult::Released
                    ? OrdinaryMutexReleaseResult::Released
                    : OrdinaryMutexReleaseResult::ProviderFailure;
            }

    };

} // ESPressio::Threading
