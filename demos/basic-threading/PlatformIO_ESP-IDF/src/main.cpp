#include <cstdint>
#include <cstdio>
#include <tuple>
#include <utility>

#include <ESPressio_Platform_FreeRTOS.hpp>
#include <ESPressio_Platform_ESP_IDF.hpp>
#include <ESPressio_Threading.hpp>

namespace Demo {

    namespace Threading = ESPressio::Threading;


    /// Outcome of running the basic Threading demonstration.
    enum class DemonstrationResult : std::uint8_t {
        Succeeded = 0,
        InitializationFailed = 1,
        InfrastructureStartFailed = 2,
        TaskDispatchFailed = 3,
        TaskResultMismatch = 4,
        ThreadActivationFailed = 5,
        ThreadStopRequestFailed = 6,
        ShutdownInitiationFailed = 7,
        FinalizationFailed = 8,
        TerminalWaitFailed = 9
    };


    // Concrete Platform providers selected by this target surface.

    /// Targeted-wake provider used by managed Threading contexts.
    using SignalProvider =
        ESPressio::Platform::FreeRTOS::Synchronization::SignalProvider;

#ifdef ARDUINO
    /// Native execution-context provider selected by the Arduino-ESP32 surface.
    using ExecutionContextProvider =
        ESPressio::Platform::FreeRTOS::Execution::ExecutionContextProvider;
#else
    /// Native execution-context provider selected by the ESP-IDF surface.
    using ExecutionContextProvider =
        ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;
#endif

    /// Topology lifecycle SpinLock provider.
    using SpinLockProvider =
        ESPressio::Platform::ESPIDF::Synchronization::SpinLockProvider;

    /// Resource-local Mutex provider.
    using MutexProvider =
        ESPressio::Platform::FreeRTOS::Synchronization::MutexProvider;


    // Demonstration topology identities and behavior.

    /// Semantic identity of the ordinary Task execution facility.
    struct WorkPool final {};

    /// Semantic identity of the persistent Dedicated Thread.
    struct HeartbeatThread final {};

    /// Stateful callable bound to the Dedicated Thread.
    struct Heartbeat final {

        /// Runs one semantic Dedicated Thread activation until cooperative stop is requested.
        void operator ()(
            Threading::ThreadContext& context
        ) noexcept {
            while (!context.IsStopRequested()) {
                ExecutionContextProvider::Yield();
            }
        }

    };


    // Static Threading topology.

    /// Complete compile-time Threading resource topology used by the demonstration.
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


    // Diagnostic output.

    /// Writes one human-readable demonstration status line using portable Standard C output.
    void Print(
        const char* message
    ) noexcept {
        std::printf(
            "%s\n",
            message
        );
    }


    // Demonstration lifecycle.

    /// Executes the basic Threading lifecycle and reports the first operational failure encountered.
    DemonstrationResult Run() {
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
            SpinLockProvider,
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
            return DemonstrationResult::InitializationFailed;
        }

        if (
            runtime.Start() !=
            Threading::ThreadingStartResult::Succeeded
        ) {
            Print("EDP-Threading demo: infrastructure start failed");
            return DemonstrationResult::InfrastructureStartFailed;
        }

        auto dispatch = runtime.Dispatch<WorkPool>(
            []() noexcept {
                return 42;
            },
            Threading::TaskDispatchPolicy::AbandonImmediately
        );

        if (!dispatch.IsSucceeded()) {
            Print("EDP-Threading demo: Task dispatch failed");
            return DemonstrationResult::TaskDispatchFailed;
        }

        auto task = dispatch.TakeTask();

        while (!task.IsFinished()) {
            ExecutionContextProvider::Yield();
        }

        auto result = task.TakeResult();

        if (
            !result.IsSucceeded() ||
            result.TakeResult() != 42
        ) {
            Print("EDP-Threading demo: Task result mismatch");
            return DemonstrationResult::TaskResultMismatch;
        }

        auto thread = runtime.ThreadHandle<HeartbeatThread>();

        if (
            thread.Start() !=
            Threading::ThreadStartResult::Started
        ) {
            Print("EDP-Threading demo: Dedicated Thread activation failed");
            return DemonstrationResult::ThreadActivationFailed;
        }

        if (
            thread.RequestStop() !=
            Threading::ThreadStopRequestResult::Accepted
        ) {
            Print("EDP-Threading demo: Dedicated Thread stop request failed");
            return DemonstrationResult::ThreadStopRequestFailed;
        }

        while (thread.State() == Threading::ThreadState::Running) {
            ExecutionContextProvider::Yield();
        }

        if (
            runtime.BeginShutdown() !=
            Threading::ThreadingShutdownResult::Accepted
        ) {
            Print("EDP-Threading demo: shutdown initiation failed");
            return DemonstrationResult::ShutdownInitiationFailed;
        }

        while (!runtime.IsExecutionQuiescent()) {
            ExecutionContextProvider::Yield();
        }

        if (
            runtime.FinalizeShutdown() !=
            Threading::ThreadingFinalizationResult::Completed
        ) {
            Print("EDP-Threading demo: finalization failed");
            return DemonstrationResult::FinalizationFailed;
        }

        if (
            runtime.WaitForShutdown() !=
            Threading::ShutdownWaitResult::Completed
        ) {
            Print("EDP-Threading demo: terminal wait failed");
            return DemonstrationResult::TerminalWaitFailed;
        }

        Print("EDP-Threading demo: PASS");
        return DemonstrationResult::Succeeded;
    }

} // Demo


#ifdef ARDUINO

/// Runs the Arduino demonstration once after framework startup.
void setup() {
    static_cast<void>(
        Demo::Run()
    );
}

/// Leaves the Arduino demonstration idle after its one-shot run.
void loop() {}

#else

/// Runs the ESP-IDF demonstration once from the framework application entry.
extern "C" void app_main() {
    static_cast<void>(
        Demo::Run()
    );
}

#endif
