#include <array>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <source_location>
#include <ESPressio_Threading_ContextBinding.hpp>
#include <ESPressio_Platform_FreeRTOS.hpp>
#include <ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp>
#include <ESPressio_Platform_ESP_IDF_ExecutionContext.hpp>
#include <ESPressio_Platform_ESP_IDF_SpinLock.hpp>
#include <timing/EspTimerMonotonicTimebaseProvider.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace Qualification {

    namespace Threading = ESPressio::Threading;
    namespace Clock = ESPressio::Clock;

    /// Actual native execution owner, used by the production Threading runtime.
    using Native = ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;

    /// Existing cross-core observation primitive.
    using Spin = ESPressio::Platform::ESPIDF::Synchronization::SpinLockProvider;

    /// Existing production wait primitive.
    using Signal = ESPressio::Platform::FreeRTOS::Synchronization::SignalProvider;

    /// Existing production ordinary mutex.
    using Mutex = ESPressio::Platform::FreeRTOS::Synchronization::MutexProvider;

    /// Immutable same-incarnation context record.
    struct Record final {

        /// Nonzero firmware-session identity, independent of CPU.
        std::uint64_t Identity;
    };

    /// Actual native TLS provider with no test injection.
    using Carrier = ESPressio::Platform::ESPIDF::Execution::CurrentContextBindingProvider<Record>;

    /// Ordinary facility identity.
    struct Pool final {};

    /// One persistent unpinned Worker; it is not recreated during migration.
    using Topology = Threading::ThreadingTopology<
        Threading::TaskExecutionFacility<
            Pool,
            Threading::TaskRecordCapacity<2U>,
            Threading::CallableCapacity<32U>,
            Threading::ResultCapacity<8U>,
            Threading::Workers<
                Threading::Worker<Threading::StackCapacity<8192U>, Threading::AnyAffinity>
            >
        >
    >;

    /// One native-slot selected policy.
    using Policy = Threading::ContextBindingPolicy<Carrier, 1U, Signal, Spin>;

    /// Actual production owner with direct native provider.
    using Runtime = Threading::StaticThreadingRuntime<Topology, std::tuple<>, Signal, Native, Spin, Mutex, Policy>;

    /// Stable record remains alive through successful native Destroy.
    const Record Context{9001U};

    /// Exact immutable topology mapping.
    const Policy::Mapping Mapping{{{Context.Identity, Context}}};

    /// Stable selected policy.
    Policy Registration(Mapping);

    /// Stable actual production Worker owner.
    Runtime Workers(std::tuple<>{}, Registration);

    /// Pinned load helpers and controller use only fixed fixture backing.
    StackType_t HelperStacks[3U][8192U / sizeof(StackType_t)];

    /// Native fixture control blocks, never reused before parked deletion.
    StaticTask_t HelperControls[3U];

    /// Higher-priority load helpers pinned on distinct cores.
    TaskHandle_t Loads[2U];

    /// Pinned fixture controller.
    TaskHandle_t Controller = nullptr;

    /// Original framework entry awaiting successful controller completion.
    TaskHandle_t Application = nullptr;

    /// Fails visibly, with assertions independent of NDEBUG.
    void Require(
        bool condition,
        const std::source_location location = std::source_location::current()
    ) noexcept {
        if (!condition) {
            std::printf(
                "THREADING_MIGRATION_FAIL line=%u function=%s\n",
                location.line(),
                location.function_name()
            );
            std::fflush(stdout);
            std::abort();
        }
    }

    /// Bounded controller wait, rejecting any unexpected load-expiry marker.
    void WaitBits(std::uint32_t expected) noexcept {
        std::uint32_t seen = 0U;
        while ((seen & expected) != expected) {
            std::uint32_t next = 0U;
            Require(xTaskNotifyWait(
                0U,
                UINT32_MAX,
                &next,
                pdMS_TO_TICKS(5000U)
            ) == pdTRUE);
            Require((next & 64U) == 0U);
            seen |= next;
        }
    }

    /// Deletes a fixture helper only after stable blocked-state proof and no future wake sender.
    void DeleteParked(TaskHandle_t task) noexcept {
        for (unsigned poll = 0U; poll < 1000U; ++poll) {
            if (eTaskGetState(task) == eBlocked) {
                vTaskDelete(task);
                return;
            }
            vTaskDelay(1U);
        }
        Require(false);
    }

    /// Finite higher-priority load makes the unpinned production Worker move to the other core.
    void LoadEntry(void* opaque) noexcept {
        const auto core = reinterpret_cast<std::uintptr_t>(opaque);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (core == 1U) {
            Require(xTaskNotify(
                Controller,
                16U,
                eSetBits
            ) == pdPASS);
        } else {
            xTaskNotifyGive(Loads[1U]);
        }
        bool released = false;
        for (unsigned spin = 0U; spin < 500000U; ++spin) {
            if (ulTaskNotifyTake(pdTRUE, 0U) != 0U) {
                released = true;
                break;
            }
        }
        if (!released) {
            xTaskNotify(
                Controller,
                64U,
                eSetBits
            );
        }
        for (;;) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
    }

    /// Executes inside the production persistent Worker before and after actual migration.
    void Migrate() noexcept {
        Carrier carrier;
        const auto task = xTaskGetCurrentTaskHandle();
        Require(xPortGetCoreID() == 0);
        Require(carrier.ReadCurrent().GetIfBound() == &Context);
        Require(carrier.ReadCurrent().GetIfBound()->Identity == 9001U);
        xTaskNotifyGive(Loads[0U]);
        Require(xPortGetCoreID() == 1);
        Require(xTaskGetCurrentTaskHandle() == task);
        Require(carrier.ReadCurrent().GetIfBound() == &Context);
        Require(carrier.ReadCurrent().GetIfBound()->Identity == 9001U);
        xTaskNotifyGive(Loads[0U]);
    }

    /// Owns startup/dispatch/shutdown while the same real Worker migrates.
    void ControllerEntry(void*) noexcept {
        Controller = xTaskGetCurrentTaskHandle();
        const auto priority = uxTaskPriorityGet(nullptr);
        for (unsigned core = 0U; core < 2U; ++core) {
            Loads[core] = xTaskCreateStaticPinnedToCore(
                LoadEntry,
                "threading-load",
                sizeof(HelperStacks[core]),
                reinterpret_cast<void*>(static_cast<std::uintptr_t>(core)),
                priority + 1U,
                HelperStacks[core],
                &HelperControls[core],
                static_cast<BaseType_t>(core)
            );
            Require(Loads[core] != nullptr);
        }
        xTaskNotifyGive(Loads[1U]);
        WaitBits(16U);
        Require(Workers.Initialize() == Threading::ThreadingInitializationResult::Succeeded);
        Require(Workers.StartFor(Clock::Duration::FromNanoseconds(2000000000LL)) ==
            Threading::ThreadingStartResult::Succeeded);
        auto result = Workers.Dispatch<Pool>(
            &Migrate,
            Threading::TaskDispatchPolicy::Queue
        );
        Require(result.IsSucceeded());
        auto task = result.TakeTask();
        const auto start = Clock::MonotonicNow();
        while (!task.IsFinished()) {
            Require(Clock::Delta(
                start,
                Clock::MonotonicNow()
            ).Nanoseconds() < 3000000000LL);
            vTaskDelay(1U);
        }
        // A second dispatch proves the same outer binding survived the return to the Worker loop.
        auto second = Workers.Dispatch<Pool>(
            []() noexcept {
                Carrier carrier;
                Require(carrier.ReadCurrent().GetIfBound() == &Context);
            },
            Threading::TaskDispatchPolicy::Queue
        );
        Require(second.IsSucceeded());
        auto next = second.TakeTask();
        while (!next.IsFinished()) {
            Require(Clock::Delta(
                start,
                Clock::MonotonicNow()
            ).Nanoseconds() < 3000000000LL);
            vTaskDelay(1U);
        }
        Require(Workers.BeginShutdown() == Threading::ThreadingShutdownResult::Accepted);
        Require(Workers.FinalizeShutdownFor(Clock::Duration::FromNanoseconds(2000000000LL)) ==
            Threading::ThreadingFinalizationResult::Completed);
        Require(Registration.Snapshot(0U)->Phase == Threading::ContextBindingPhase::Retired);
        Require(Registration.Snapshot(0U)->NativeState == Threading::ContextBindingNativeState::Destroyed);
        for (const auto load : Loads) {
            DeleteParked(load);
        }
        Require(xTaskNotify(
            Application,
            128U,
            eSetBits
        ) == pdPASS);
        for (;;) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
    }

    /// Runs one forced same-Worker migration, using canonical Clock and explicit fixture scheduling.
    void Run() noexcept {
        using Timebase = ESPressio::Platform::ESPIDF::Timing::EspTimerMonotonicTimebaseProvider;
        static Timebase source;
        static Clock::MonotonicClockProvider<Timebase> clock(source);
        Require(Clock::BindMonotonicClock(clock));
        Application = xTaskGetCurrentTaskHandle();
        // Match the actual native provider's Normal mapping for this SDK configuration.
        const UBaseType_t normalPriority = configMAX_PRIORITIES / 2U;
        const auto controller = xTaskCreateStaticPinnedToCore(
            ControllerEntry,
            "threading-migctl",
            sizeof(HelperStacks[2U]),
            nullptr,
            normalPriority,
            HelperStacks[2U],
            &HelperControls[2U],
            0
        );
        Require(controller != nullptr);
        WaitBits(128U);
        DeleteParked(controller);
        std::puts("THREADING_MIGRATION_PASS same_worker=1 cores=0,1 identity=9001 retired=1 destroyed=1\n");
        std::fflush(stdout);
    }

} // Qualification

#ifdef ARDUINO
#include <Arduino.h>
/// Runs from ordinary framework entry.
void setup() { Qualification::Run(); }
/// Keeps the completed firmware image observable.
void loop() { delay(1000); }
#else
/// Runs from the SDK application entry.
extern "C" void app_main() { Qualification::Run(); }
#endif
