#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <thread>

#include <ESPressio_Threading.hpp>

namespace Test {

    namespace Framework = ESPressio::System::CompositionFramework;

    struct MutexIdentity final {};

    /// Host SpinLock provider used to protect the independent sticky failure latch.
    class SpinLockProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Offers<
            Framework::Offer<
                ESPressio::Platform::Synchronization::SpinLock,
                Framework::PropertyValue<
                    ESPressio::Platform::Synchronization::SpinLockSupportsInterruptContext,
                    false
                >
            >
        >
    > {
        private:
            std::atomic_flag _flag = ATOMIC_FLAG_INIT;

        public:
            SpinLockProvider() noexcept = default;
            SpinLockProvider(const SpinLockProvider&) = delete;
            SpinLockProvider& operator =(const SpinLockProvider&) = delete;
            SpinLockProvider(SpinLockProvider&&) = delete;
            SpinLockProvider& operator =(SpinLockProvider&&) = delete;

            void Acquire() noexcept {
                while (_flag.test_and_set(std::memory_order_acquire)) {
                    std::this_thread::yield();
                }
            }

            ESPressio::Platform::Synchronization::SpinLockAcquireResult AcquireFromInterrupt() noexcept {
                return ESPressio::Platform::Synchronization::SpinLockAcquireResult::UnsupportedInterruptContext;
            }

            ESPressio::Platform::Synchronization::SpinLockReleaseResult Release() noexcept {
                _flag.clear(std::memory_order_release);
                return ESPressio::Platform::Synchronization::SpinLockReleaseResult::Released;
            }

            ESPressio::Platform::Synchronization::SpinLockReleaseResult ReleaseFromInterrupt() noexcept {
                return ESPressio::Platform::Synchronization::SpinLockReleaseResult::UnsupportedInterruptContext;
            }
    };

    /// Timed synthetic mutex supporting deterministic contention and injected failures.
    class TimedMutexProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Offers<
            Framework::Offer<
                ESPressio::Platform::Synchronization::Mutex,
                Framework::PropertyValue<
                    ESPressio::Platform::Synchronization::MutexWaitResolutionNanoseconds,
                    1'000'000ULL
                >
            >
        >
    > {
        private:
            std::atomic<bool> _locked{false};

        public:
            inline static std::atomic<bool> AcquireFails{false};
            inline static std::atomic<bool> ReleaseFails{false};
            inline static std::atomic<std::uint32_t> AcquireCalls{0U};

            TimedMutexProvider() noexcept = default;
            TimedMutexProvider(const TimedMutexProvider&) = delete;
            TimedMutexProvider& operator =(const TimedMutexProvider&) = delete;
            TimedMutexProvider(TimedMutexProvider&&) = delete;
            TimedMutexProvider& operator =(TimedMutexProvider&&) = delete;

            ESPressio::Platform::Synchronization::LockAcquireResult Acquire(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                ++AcquireCalls;

                if (AcquireFails.load(std::memory_order_acquire)) {
                    return ESPressio::Platform::Synchronization::LockAcquireResult::ProviderFailure;
                }

                bool expected = false;
                if (_locked.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
                    return ESPressio::Platform::Synchronization::LockAcquireResult::Acquired;
                }

                if (timeout.IsFinite()) {
                    std::this_thread::sleep_for(
                        std::chrono::nanoseconds(timeout.Nanoseconds())
                    );
                }

                return ESPressio::Platform::Synchronization::LockAcquireResult::TimedOut;
            }

            ESPressio::Platform::Synchronization::LockReleaseResult Release() noexcept {
                if (ReleaseFails.load(std::memory_order_acquire)) {
                    return ESPressio::Platform::Synchronization::LockReleaseResult::ProviderFailure;
                }

                bool expected = true;
                return _locked.compare_exchange_strong(
                    expected,
                    false,
                    std::memory_order_acq_rel
                )
                    ? ESPressio::Platform::Synchronization::LockReleaseResult::Released
                    : ESPressio::Platform::Synchronization::LockReleaseResult::InvalidOwnership;
            }

            static void ResetControls() noexcept {
                AcquireFails.store(false, std::memory_order_release);
                ReleaseFails.store(false, std::memory_order_release);
                AcquireCalls.store(0U, std::memory_order_release);
            }
    };


    using Mutex = ESPressio::Threading::FailClosedOrdinaryMutexProvider<
        MutexIdentity,
        TimedMutexProvider,
        SpinLockProvider
    >;

    static_assert(Mutex::FailureObservationIntervalNanoseconds == 1'000'000ULL);
    static_assert(
        Mutex::CompositionOffers::template Contains<
            ESPressio::Threading::FailClosedOrdinaryMutex<MutexIdentity>
        >
    );


    void TestHealthyAcquireRelease() {
        TimedMutexProvider::ResetControls();
        Mutex mutex;

        assert(!mutex.IsFailed());
        assert(mutex.Acquire() == ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult::Acquired);
        assert(mutex.Release() == ESPressio::Threading::FailClosedOrdinaryMutexReleaseResult::Released);
        assert(!mutex.IsFailed());
    }


    void TestAcquireFailureLatches() {
        TimedMutexProvider::ResetControls();
        TimedMutexProvider::AcquireFails.store(true, std::memory_order_release);
        Mutex mutex;

        assert(mutex.Acquire() == ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult::ProviderFailure);
        assert(mutex.IsFailed());

        const auto calls = TimedMutexProvider::AcquireCalls.load(std::memory_order_acquire);
        assert(mutex.Acquire() == ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult::FailedState);
        assert(TimedMutexProvider::AcquireCalls.load(std::memory_order_acquire) == calls);
    }


    void TestReleaseFailureLatches() {
        TimedMutexProvider::ResetControls();
        Mutex mutex;

        assert(mutex.Acquire() == ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult::Acquired);
        TimedMutexProvider::ReleaseFails.store(true, std::memory_order_release);
        assert(mutex.Release() == ESPressio::Threading::FailClosedOrdinaryMutexReleaseResult::ProviderFailure);
        assert(mutex.IsFailed());
        assert(mutex.Acquire() == ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult::FailedState);
    }


    void TestWaitingCallerObservesReleaseFailure() {
        TimedMutexProvider::ResetControls();
        Mutex mutex;

        assert(mutex.Acquire() == ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult::Acquired);

        std::atomic<bool> waiterStarted{false};
        std::atomic<ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult> waiterResult{
            ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult::Acquired
        };

        std::thread waiter([&]() noexcept {
            waiterStarted.store(true, std::memory_order_release);
            waiterResult.store(mutex.Acquire(), std::memory_order_release);
        });

        while (!waiterStarted.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        TimedMutexProvider::ReleaseFails.store(true, std::memory_order_release);
        assert(mutex.Release() == ESPressio::Threading::FailClosedOrdinaryMutexReleaseResult::ProviderFailure);

        waiter.join();
        assert(mutex.IsFailed());
        assert(
            waiterResult.load(std::memory_order_acquire) ==
            ESPressio::Threading::FailClosedOrdinaryMutexAcquireResult::FailedState
        );
    }

} // namespace Test


int main() {
    Test::TestHealthyAcquireRelease();
    Test::TestAcquireFailureLatches();
    Test::TestReleaseFailureLatches();
    Test::TestWaitingCallerObservesReleaseFailure();
    return 0;
}
