#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

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

            // Provider lifetime and exclusive-identity contract.

            /// Constructs a new unlocked synthetic native Mutex provider.
            PlatformMutex() = default;

            /// Prevents accidental duplication of the native Mutex provider.
            PlatformMutex(const PlatformMutex&) = delete;

            /// Prevents copying ownership into an existing native Mutex provider.
            PlatformMutex& operator=(const PlatformMutex&) = delete;

            /// Prevents transfer of the native Mutex provider's identity.
            PlatformMutex(PlatformMutex&&) = delete;

            /// Prevents transfer into an existing native Mutex provider.
            PlatformMutex& operator=(PlatformMutex&&) = delete;

            // Deterministic test observations and fault injection.

            /// Last requested native wait mode.
            Sync::WaitMode LastMode = Sync::WaitMode::NoWait;

            /// Last requested native duration.
            std::uint64_t LastDuration = 0U;

            /// Cause the next acquisition to report a genuine provider failure.
            inline static bool FailNextAcquire = false;

            /// Last native wait mode seen by the synthetic provider.
            inline static Sync::WaitMode LastRequestedMode = Sync::WaitMode::NoWait;

            /// Last finite native duration seen by the synthetic provider.
            inline static std::uint64_t LastRequestedDuration = 0U;

            /// Acquires or rejects according to the deterministic simulated state.
            Sync::LockAcquireResult Acquire(Sync::WaitTimeout timeout) noexcept {
                LastMode = timeout.Mode();
                LastDuration = timeout.Nanoseconds();
                LastRequestedMode = timeout.Mode();
                LastRequestedDuration = timeout.Nanoseconds();

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

    static_assert(!std::is_copy_constructible_v<PlatformMutex>);
    static_assert(!std::is_move_constructible_v<PlatformMutex>);

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

        PlatformMutex::FailNextAcquire = false;
        assert(mutex.TryAcquire() == Result::Acquired);
        assert(PlatformMutex::LastRequestedMode == Sync::WaitMode::NoWait);
        assert(mutex.TryAcquire() == Result::TimedOut);
        assert(PlatformMutex::LastRequestedMode == Sync::WaitMode::NoWait);
        assert(
            mutex.Acquire(Sync::WaitTimeout::ForNanoseconds(17'000'000ULL))
            == Result::TimedOut
        );
        assert(PlatformMutex::LastRequestedMode == Sync::WaitMode::Finite);
        assert(PlatformMutex::LastRequestedDuration == 17'000'000ULL);
        assert(mutex.Release() == ESPressio::Threading::OrdinaryMutexReleaseResult::Released);
        PlatformMutex::FailNextAcquire = true;
        assert(mutex.TryAcquire() == Result::ProviderFailure);
        assert(
            mutex.Acquire(Sync::WaitTimeout::NoWait())
            == Result::Acquired
        );
        assert(mutex.Release() == ESPressio::Threading::OrdinaryMutexReleaseResult::Released);
        assert(mutex.Acquire() == Result::Acquired);
        assert(PlatformMutex::LastRequestedMode == Sync::WaitMode::Forever);
        assert(mutex.Release() == ESPressio::Threading::OrdinaryMutexReleaseResult::Released);
    }

} // Test

int main() {
    Test::TestOrdinaryAcquisition();
    return 0;
}
