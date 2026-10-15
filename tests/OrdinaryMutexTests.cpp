#include <cassert>
#include <cstdint>
#include <type_traits>

#include <ESPressio_Threading.hpp>

namespace Test {

    namespace Framework = ESPressio::System::CompositionFramework;
    namespace Sync = ESPressio::Platform::Synchronization;

    struct TestMutexIdentity final {};

    /// Deterministic non-recursive native Mutex for bounded Threading contract tests.
    class PlatformMutex final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Offers<
            Framework::Offer<
                Sync::Mutex,
                Framework::PropertyValue<
                    Sync::MutexWaitResolutionNanoseconds,
                    1'000'000ULL
                >
            >
        >
    > {

        private:

            /// Native test mutex occupancy.
            bool _locked = false;

        public:

            /// Last requested native wait mode.
            Sync::WaitMode LastMode = Sync::WaitMode::NoWait;

            /// Last requested native duration.
            std::uint64_t LastDuration = 0U;

            /// Cause the next acquisition to report a genuine provider failure.
            bool FailNextAcquire = false;

            /// Acquires or rejects according to the deterministic simulated state.
            Sync::LockAcquireResult Acquire(Sync::WaitTimeout timeout) noexcept {
                LastMode = timeout.Mode();
                LastDuration = timeout.Nanoseconds();

                if (FailNextAcquire) {
                    FailNextAcquire = false;
                    return Sync::LockAcquireResult::ProviderFailure;
                }

                if (_locked) {
                    return Sync::LockAcquireResult::TimedOut;
                }

                _locked = true;
                return Sync::LockAcquireResult::Acquired;
            }

            /// Releases the deterministic test mutex.
            Sync::LockReleaseResult Release() noexcept {
                if (!_locked) {
                    return Sync::LockReleaseResult::InvalidOwnership;
                }

                _locked = false;
                return Sync::LockReleaseResult::Released;
            }

    };

    using Mutex = ESPressio::Threading::OrdinaryMutexProvider<TestMutexIdentity, PlatformMutex>;

    static_assert(
        Mutex::CompositionOffers::template Contains<
            ESPressio::Threading::OrdinaryMutex<TestMutexIdentity>
        >
    );

    static_assert(std::is_same_v<
        decltype(std::declval<Mutex&>().TryAcquire()),
        ESPressio::Threading::OrdinaryMutexAcquireResult
    >);

    /// Checks legacy and new native wait boundaries without real-time sleeps.
    void TestOrdinaryAcquisition() {
        using Result = ESPressio::Threading::OrdinaryMutexAcquireResult;
        Mutex mutex;

        assert(mutex.TryAcquire() == Result::Acquired);
        assert(mutex.TryAcquire() == Result::TimedOut);
        assert(
            mutex.Acquire(Sync::WaitTimeout::ForNanoseconds(17'000'000ULL))
            == Result::TimedOut
        );
        assert(mutex.Release() == ESPressio::Threading::OrdinaryMutexReleaseResult::Released);
        assert(
            mutex.Acquire(Sync::WaitTimeout::NoWait())
            == Result::Acquired
        );
        assert(mutex.Release() == ESPressio::Threading::OrdinaryMutexReleaseResult::Released);
        assert(mutex.Acquire() == Result::Acquired);
        assert(mutex.Release() == ESPressio::Threading::OrdinaryMutexReleaseResult::Released);
    }

} // Test

int main() {
    Test::TestOrdinaryAcquisition();
    return 0;
}
