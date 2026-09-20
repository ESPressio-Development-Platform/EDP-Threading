#include <cstdint>
#include <cstdio>
#include <tuple>
#include <utility>

#include <FreeRTOS.h>
#include <task.h>

#include <ESPressio_Platform_FreeRTOS.hpp>
#include <ESPressio_Platform_Portable.hpp>
#include <ESPressio_Threading.hpp>

namespace Demo {

    namespace Threading = ESPressio::Threading;

    struct WorkPool final {};
    struct HeartbeatThread final {};

    struct Heartbeat final {

        void operator ()(
            Threading::ThreadContext& context
        ) noexcept {
            while (!context.IsStopRequested()) {
                vTaskDelay(
                    pdMS_TO_TICKS(10U)
                );
            }
        }

    };

    using Topology = Threading::ThreadingTopology<
        Threading::TaskExecutionFacility<
            WorkPool,
            Threading::TaskRecordCapacity<4U>,
            Threading::CallableCapacity<32U>,
            Threading::ResultCapacity<8U>,
            Threading::Workers<
                Threading::Worker<
                    Threading::StackCapacity<4096U>,
                    Threading::Priority<Threading::ThreadPriority::Normal>,
                    Threading::AnyAffinity
                >
            >
        >,
        Threading::DedicatedThread<
            HeartbeatThread,
            Threading::StackCapacity<4096U>,
            Threading::Priority<Threading::ThreadPriority::Low>,
            Threading::AnyAffinity
        >
    >;

    using SignalProvider =
        ESPressio::Platform::FreeRTOS::Synchronization::SignalProvider;

    using ExecutionContextProvider =
        ESPressio::Platform::FreeRTOS::Execution::ExecutionContextProvider;

    using AtomicWord8Provider =
        ESPressio::Platform::Portable::Concurrency::AtomicWord8Provider;

    using MutexProvider =
        ESPressio::Platform::FreeRTOS::Synchronization::MutexProvider;


    bool Run() {
        auto bindings = std::make_tuple(
            Threading::BindDedicatedThread<HeartbeatThread>(
                Heartbeat{}
            )
        );

        using Bindings = decltype(bindings);
        using Runtime = Threading::StaticThreadingRuntime<
            Topology,
            Bindings,
            SignalProvider,
            ExecutionContextProvider,
            AtomicWord8Provider,
            MutexProvider
        >;

        static Runtime runtime(
            std::move(
                bindings
            )
        );

        if (
            runtime.Initialize() !=
            Threading::ThreadingInitializationResult::Succeeded
        ) {
            std::printf("EDP-Threading demo: initialization failed\n");
            return false;
        }

        if (
            runtime.Start() !=
            Threading::ThreadingStartResult::Succeeded
        ) {
            std::printf("EDP-Threading demo: infrastructure start failed\n");
            return false;
        }

        auto dispatch = runtime.Dispatch<WorkPool>(
            []() noexcept {
                return 42;
            },
            Threading::TaskDispatchPolicy::AbandonImmediately
        );

        if (!dispatch.IsSucceeded()) {
            std::printf("EDP-Threading demo: Task dispatch failed\n");
            return false;
        }

        auto task = dispatch.TakeTask();

        while (!task.IsFinished()) {
            vTaskDelay(
                pdMS_TO_TICKS(1U)
            );
        }

        auto result = task.TakeResult();

        if (
            !result.IsSucceeded() ||
            result.TakeResult() != 42
        ) {
            std::printf("EDP-Threading demo: Task result mismatch\n");
            return false;
        }

        auto thread = runtime.ThreadHandle<HeartbeatThread>();

        if (
            thread.Start() !=
            Threading::ThreadStartResult::Started
        ) {
            std::printf("EDP-Threading demo: Dedicated Thread activation failed\n");
            return false;
        }

        vTaskDelay(
            pdMS_TO_TICKS(25U)
        );

        if (
            thread.RequestStop() !=
            Threading::ThreadStopRequestResult::Accepted
        ) {
            std::printf("EDP-Threading demo: Dedicated Thread stop request failed\n");
            return false;
        }

        while (thread.State() == Threading::ThreadState::Running) {
            vTaskDelay(
                pdMS_TO_TICKS(1U)
            );
        }

        if (
            runtime.BeginShutdown() !=
            Threading::ThreadingShutdownResult::Accepted
        ) {
            std::printf("EDP-Threading demo: shutdown initiation failed\n");
            return false;
        }

        while (!runtime.IsExecutionQuiescent()) {
            vTaskDelay(
                pdMS_TO_TICKS(1U)
            );
        }

        if (
            runtime.FinalizeShutdown() !=
            Threading::ThreadingFinalizationResult::Completed
        ) {
            std::printf("EDP-Threading demo: finalization failed\n");
            return false;
        }

        if (
            runtime.WaitForShutdown() !=
            Threading::ShutdownWaitResult::Completed
        ) {
            std::printf("EDP-Threading demo: terminal wait failed\n");
            return false;
        }

        std::printf("EDP-Threading demo: PASS\n");
        return true;
    }

} // Demo

void setup() {
    static_cast<void>(
        Demo::Run()
    );
}

void loop() {}
