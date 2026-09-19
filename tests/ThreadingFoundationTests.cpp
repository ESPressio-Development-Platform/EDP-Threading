#include <cassert>
#include <cstdint>
#include <type_traits>

#include <ESPressio_Threading.hpp>

#include "../src/threading/detail/TaskRecord.hpp"

namespace Test {

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
        sizeof(ESPressio::Threading::Detail::TaskControl) == 1U,
        "Task intrinsic control must remain one byte"
    );

} // Test


int main() {
    ESPressio::Threading::Detail::TaskControl control;

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
