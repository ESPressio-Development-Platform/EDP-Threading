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


    /// Result of acquiring a keyed ordinary-context mutex indefinitely.
    enum class OrdinaryMutexAcquireResult : std::uint8_t {
        Acquired = 0,
        ProviderFailure = 1
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

            /// Acquires the mutex indefinitely from ordinary execution context.
            [[nodiscard]] OrdinaryMutexAcquireResult Acquire() noexcept {
                return _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                ) == ESPressio::Platform::Synchronization::LockAcquireResult::Acquired
                    ? OrdinaryMutexAcquireResult::Acquired
                    : OrdinaryMutexAcquireResult::ProviderFailure;
            }

            /// Releases the mutex owned by the current ordinary execution context.
            [[nodiscard]] OrdinaryMutexReleaseResult Release() noexcept {
                return _mutex.Release() == ESPressio::Platform::Synchronization::LockReleaseResult::Released
                    ? OrdinaryMutexReleaseResult::Released
                    : OrdinaryMutexReleaseResult::ProviderFailure;
            }

    };

} // ESPressio::Threading
