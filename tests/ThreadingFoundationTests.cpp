#include <cassert>
#include <cstdint>
#include <type_traits>

#include <ESPressio_Threading.hpp>

#include "../src/threading/detail/TaskRecord.hpp"

namespace Test {

    class AtomicByteProvider final {

        public:

            class Word final {

                private:

                    /// Test-owned byte value.
                    std::uint8_t _value = 0U;

                public:

                    /// Reads the test byte.
                    std::uint8_t LoadRelaxed() const noexcept { return _value; }

                    /// Reads the test byte with acquire-equivalent test semantics.
                    std::uint8_t LoadAcquire() const noexcept { return _value; }

                    /// Stores the test byte.
                    void StoreRelaxed(std::uint8_t value) noexcept { _value = value; }

                    /// Stores the test byte with release-equivalent test semantics.
                    void StoreRelease(std::uint8_t value) noexcept { _value = value; }

                    /// Replaces the byte when the expected value matches.
                    bool CompareExchangeAcqRel(
                        std::uint8_t& expected,
                        std::uint8_t desired
                    ) noexcept {
                        if (_value != expected) {
                            expected = _value;
                            return false;
                        }

                        _value = desired;
                        return true;
                    }

            };

    };


    struct OrdinaryPool final {};


    struct TelemetryThread final {};


    using Topology = ESPressio::Threading::ThreadingTopology<
        ESPressio::Threading::TaskExecutionFacility<
            OrdinaryPool,
            ESPressio::Threading::TaskRecordCapacity<8U>,
            ESPressio::Threading::CallableCapacity<64U>,
            ESPressio::Threading::ResultCapacity<32U>,
            ESPressio::Threading::Workers<
                ESPressio::Threading::Worker<
                    ESPressio::Threading::StackCapacity<4096U>,
                    ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::Normal>,
                    ESPressio::Threading::AnyAffinity
                >
            >
        >,
        ESPressio::Threading::DedicatedThread<
            TelemetryThread,
            ESPressio::Threading::StackCapacity<4096U>,
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::High>,
            ESPressio::Threading::AnyAffinity
        >
    >;


    static_assert(
        Topology::HasTaskExecution,
        "Topology containing a Task facility must advertise Task execution"
    );

    static_assert(
        Topology::HasDedicatedThreadExecution,
        "Topology containing a Dedicated Thread must advertise Dedicated Thread execution"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::TaskControl<AtomicByteProvider>) == 1U,
        "Task intrinsic control must remain one byte"
    );

} // Test


int main() {
    ESPressio::Threading::Detail::TaskControl<Test::AtomicByteProvider> control;

    control.InitializeQueued();

    assert(
        control.State() ==
        ESPressio::Threading::Detail::TaskOperationalState::Queued
    );

    assert(
        control.HasOwner()
    );

    const auto firstPhase = control.Phase();

    control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::RunningCancelRequested
    );

    assert(
        control.IsCancellationRequested()
    );

    control.ReleaseOwner();

    assert(
        !control.HasOwner()
    );

    control.InitializeQueued();

    assert(
        control.Phase() != firstPhase
    );

    return 0;
}
