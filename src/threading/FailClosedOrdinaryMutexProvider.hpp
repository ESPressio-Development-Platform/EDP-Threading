#pragma once

#include <cstdint>
#include <exception>

#include <ESPressio_Platform.hpp>
#include <ESPressio_System.hpp>

#include "ThreadingComposition.hpp"

namespace ESPressio::Threading {

    /// Keyed ordinary-context non-recursive mutual exclusion with a sticky terminal failure latch.
    /// @tparam TIdentity Semantic identity separating independent mutex instances.
    template<class TIdentity>
    struct FailClosedOrdinaryMutex final : Framework::ExclusiveCapability<Domain> {
        using Identity = TIdentity;
    };


    /// Result of acquiring a fail-closed ordinary mutex.
    enum class FailClosedOrdinaryMutexAcquireResult : std::uint8_t {
        Acquired = 0,
        FailedState = 1,
        ProviderFailure = 2
    };


    /// Result of releasing a fail-closed ordinary mutex.
    enum class FailClosedOrdinaryMutexReleaseResult : std::uint8_t {
        Released = 0,
        FailedState = 1,
        ProviderFailure = 2
    };


    /// Threading-owned ordinary mutex adapter with independently synchronized terminal failure publication.
    ///
    /// Acquisition waits indefinitely while healthy, but does so in bounded native-mutex slices so a
    /// release/provider failure can be observed by callers already waiting for the same mutex. The failure
    /// latch is sticky for the provider object's lifetime and is protected independently by a Platform SpinLock.
    ///
    /// @tparam TIdentity Semantic identity distinguishing this mutex from unrelated consumers.
    /// @tparam TMutexProvider Concrete Platform non-recursive Mutex provider owned by this adapter.
    /// @tparam TFailureSpinLockProvider Concrete Platform SpinLock provider protecting the failure latch.
    template<class TIdentity, class TMutexProvider, class TFailureSpinLockProvider>
    class FailClosedOrdinaryMutexProvider final : public Framework::Provider<
        Domain,
        Framework::Offers<Framework::Offer<FailClosedOrdinaryMutex<TIdentity>>>
    > {

        private:

            /// Validated native mutex contract.
            using MutexTraits = ESPressio::Platform::Synchronization::Detail::MutexProviderTraits<
                TMutexProvider
            >;

            /// Validated independent SpinLock contract protecting terminal-failure publication.
            using FailureSpinLockTraits = ESPressio::Platform::Synchronization::Detail::SpinLockProviderTraits<
                TFailureSpinLockProvider
            >;

            static_assert(
                sizeof(FailureSpinLockTraits) > 0U,
                "FailClosedOrdinaryMutexProvider requires a valid Platform SpinLock provider"
            );

            /// Target maximum interval before a blocked caller rechecks the failure latch.
            static constexpr std::uint64_t TargetFailureObservationNanoseconds = 1'000'000ULL;

            /// Native mutex wait resolution advertised by the concrete Platform provider.
            static constexpr std::uint64_t MutexWaitResolutionNanoseconds =
                MutexTraits::Properties::template Value<
                    ESPressio::Platform::Synchronization::MutexWaitResolutionNanoseconds
                >;

            /// Finite native wait slice, rounded up to the concrete mutex resolution.
            static constexpr std::uint64_t WaitSliceNanoseconds = []() constexpr {
                if constexpr (
                    MutexWaitResolutionNanoseconds >= TargetFailureObservationNanoseconds
                ) {
                    return MutexWaitResolutionNanoseconds;
                } else {
                    return (
                        (TargetFailureObservationNanoseconds + MutexWaitResolutionNanoseconds - 1ULL) /
                        MutexWaitResolutionNanoseconds
                    ) * MutexWaitResolutionNanoseconds;
                }
            }();

            /// Platform-owned native synchronization mechanism.
            TMutexProvider _mutex;

            /// Independent short critical-section lock protecting the sticky failure byte.
            mutable TFailureSpinLockProvider _failureLock;

            /// Sticky terminal-failure state protected only by _failureLock.
            bool _failed{false};

            /// Releases the failure SpinLock or terminates on an impossible ordinary-context release failure.
            void ReleaseFailureLock() const noexcept {
                if (
                    _failureLock.Release() !=
                    ESPressio::Platform::Synchronization::SpinLockReleaseResult::Released
                ) {
                    std::terminate();
                }
            }

            /// Publishes terminal failure for all current/future callers.
            void MarkFailed() noexcept {
                _failureLock.Acquire();
                _failed = true;
                ReleaseFailureLock();
            }

        public:

            /// Semantic identity Type of this independent fail-closed ordinary mutex.
            using Identity = TIdentity;

            /// Maximum healthy interval before a blocked caller rechecks terminal failure.
            static constexpr std::uint64_t FailureObservationIntervalNanoseconds = WaitSliceNanoseconds;

            FailClosedOrdinaryMutexProvider() = default;
            FailClosedOrdinaryMutexProvider(const FailClosedOrdinaryMutexProvider&) = delete;
            FailClosedOrdinaryMutexProvider& operator =(const FailClosedOrdinaryMutexProvider&) = delete;
            FailClosedOrdinaryMutexProvider(FailClosedOrdinaryMutexProvider&&) = delete;
            FailClosedOrdinaryMutexProvider& operator =(FailClosedOrdinaryMutexProvider&&) = delete;

            /// Indicates whether any underlying mutex provider failure has been observed.
            [[nodiscard]] bool IsFailed() const noexcept {
                _failureLock.Acquire();
                const bool failed = _failed;
                ReleaseFailureLock();
                return failed;
            }

            /// Acquires the mutex indefinitely while healthy, observing sticky failure between finite waits.
            [[nodiscard]] FailClosedOrdinaryMutexAcquireResult Acquire() noexcept {
                if (IsFailed()) {
                    return FailClosedOrdinaryMutexAcquireResult::FailedState;
                }

                for (;;) {
                    const auto result = _mutex.Acquire(
                        ESPressio::Platform::Synchronization::WaitTimeout::ForNanoseconds(
                            WaitSliceNanoseconds
                        )
                    );

                    if (result == ESPressio::Platform::Synchronization::LockAcquireResult::Acquired) {
                        if (!IsFailed()) {
                            return FailClosedOrdinaryMutexAcquireResult::Acquired;
                        }

                        const auto releaseResult = _mutex.Release();
                        if (releaseResult != ESPressio::Platform::Synchronization::LockReleaseResult::Released) {
                            MarkFailed();
                            return FailClosedOrdinaryMutexAcquireResult::ProviderFailure;
                        }

                        return FailClosedOrdinaryMutexAcquireResult::FailedState;
                    }

                    if (result == ESPressio::Platform::Synchronization::LockAcquireResult::TimedOut) {
                        if (IsFailed()) {
                            return FailClosedOrdinaryMutexAcquireResult::FailedState;
                        }
                        continue;
                    }

                    MarkFailed();
                    return FailClosedOrdinaryMutexAcquireResult::ProviderFailure;
                }
            }

            /// Releases the mutex and preserves terminal failure if provider integrity is uncertain.
            [[nodiscard]] FailClosedOrdinaryMutexReleaseResult Release() noexcept {
                const bool failedBeforeRelease = IsFailed();
                const auto result = _mutex.Release();

                if (result != ESPressio::Platform::Synchronization::LockReleaseResult::Released) {
                    MarkFailed();
                    return FailClosedOrdinaryMutexReleaseResult::ProviderFailure;
                }

                return failedBeforeRelease || IsFailed()
                    ? FailClosedOrdinaryMutexReleaseResult::FailedState
                    : FailClosedOrdinaryMutexReleaseResult::Released;
            }

    };

} // ESPressio::Threading
