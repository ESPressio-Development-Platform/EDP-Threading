#include <cstdint>
#include <cstdio>
#include <tuple>
#include <utility>

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ESPressio_Platform_FreeRTOS.hpp>
#include <ESPressio_Platform_Portable.hpp>
#ifndef ARDUINO
#include <ESPressio_Platform_ESP_IDF.hpp>
#endif
#include <ESPressio_Threading.hpp>

namespace Demo {

    void Print(
        const char* message
    ) noexcept {
#ifdef ARDUINO
        Serial.println(
            message
        );
#else
        std::printf(
            "%s\n",
            message
        );
#endif
    }


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

#ifdef ARDUINO
    using ExecutionContextProvider =
        ESPressio::Platform::FreeRTOS::Execution::ExecutionContextProvider;
#else
    using ExecutionContextProvider =
        ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;
#endif

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
            Print("EDP-Threading demo: initialization failed");
            return false;
        }

        if (
            runtime.Start() !=
            Threading::ThreadingStartResult::Succeeded
        ) {
            Print("EDP-Threading demo: infrastructure start failed");
            return false;
        }

        vTaskDelay(
            pdMS_TO_TICKS(25U)
        );

        auto dispatch = runtime.Dispatch<WorkPool>(
            []() noexcept {
                return 42;
            },
            Threading::TaskDispatchPolicy::AbandonImmediately
        );

        if (!dispatch.IsSucceeded()) {
            Print("EDP-Threading demo: Task dispatch failed");
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
            Print("EDP-Threading demo: Task result mismatch");
            return false;
        }

        auto thread = runtime.ThreadHandle<HeartbeatThread>();

        if (
            thread.Start() !=
            Threading::ThreadStartResult::Started
        ) {
            Print("EDP-Threading demo: Dedicated Thread activation failed");
            return false;
        }

        vTaskDelay(
            pdMS_TO_TICKS(25U)
        );

        if (
            thread.RequestStop() !=
            Threading::ThreadStopRequestResult::Accepted
        ) {
            Print("EDP-Threading demo: Dedicated Thread stop request failed");
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
            Print("EDP-Threading demo: shutdown initiation failed");
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
            Print("EDP-Threading demo: finalization failed");
            return false;
        }

        if (
            runtime.WaitForShutdown() !=
            Threading::ShutdownWaitResult::Completed
        ) {
            Print("EDP-Threading demo: terminal wait failed");
            return false;
        }

        Print("EDP-Threading demo: PASS");
        return true;
    }

} // Demo

extern "C" void app_main() {
    static_cast<void>(
        Demo::Run()
    );
}
