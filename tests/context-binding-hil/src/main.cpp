#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <source_location>
#include <ESPressio_Clock.hpp>
#include <ESPressio_Memory.hpp>
#include <ESPressio_Platform_FreeRTOS.hpp>
#include <ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp>
#include <ESPressio_Platform_ESP_IDF_ExecutionContext.hpp>
#include <ESPressio_Platform_ESP_IDF_SpinLock.hpp>
#include <timing/EspTimerMonotonicTimebaseProvider.hpp>
#include <ESPressio_Threading_ContextBinding.hpp>

namespace Qualification {

    namespace Threading = ESPressio::Threading;
    namespace Execution = ESPressio::Platform::Execution;
    namespace Sync = ESPressio::Platform::Synchronization;
    namespace Framework = ESPressio::System::CompositionFramework;
    namespace Clock = ESPressio::Clock;

    /// Explicit fixture scenarios; no fault injection is part of the production providers.
    enum class ScenarioKind : std::uint8_t {
        Success = 0U,
        ForeignBinding = 1U,
        UnsupportedBinding = 2U,
        PartialStart = 3U,
        LateEntry = 4U,
        RetirementMismatch = 5U
    };

    /// Actual qualified native execution owner.
    using Native = ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;

    /// Existing native cross-core control lock.
    using Spin = ESPressio::Platform::ESPIDF::Synchronization::SpinLockProvider;

    /// Existing native wake primitive.
    using Signal = ESPressio::Platform::FreeRTOS::Synchronization::SignalProvider;

    /// Existing ordinary native mutex.
    using Mutex = ESPressio::Platform::FreeRTOS::Synchronization::MutexProvider;

    /// Immutable per-incarnation record.
    struct Record final {

        /// Nonzero session-unique owner-assigned identity.
        std::uint64_t Identity;

        /// Direct topology slot, independent of CPU identity.
        std::size_t Slot;
    };

    /// Real compiler-native TLS carrier, including all injected-fault scenarios.
    using Carrier = ESPressio::Platform::ESPIDF::Execution::CurrentContextBindingProvider<Record>;

    /// Written only between fully retired scenarios.
    ScenarioKind CurrentScenario = ScenarioKind::Success;

    /// Fixture-owned foreign record with firmware lifetime.
    const Record Foreign{0xFFFFFFFFFFFFFFFEULL, 99U};

    /// Current immutable record base, replaced only after verified native retirement.
    const Record* Records = nullptr;

    /// Fixture-only late-entry gate before the actual Threading entry.
    Signal LateGate;

    /// Protects cross-core fixture observations.
    Spin ObservationLock;

    /// Eligible application-call counts.
    std::array<unsigned, 4U> Calls{};

    /// Fresh task-local unbound observations.
    unsigned FreshEntries = 0U;

    /// Foreign records preserved after the actual Threading entry returns.
    unsigned PreservedForeign = 0U;

    /// Fails visibly without manufacturing successful reclamation.
    void Require(
        bool condition,
        const std::source_location location = std::source_location::current()
    ) noexcept {
        if (!condition) {
            std::printf(
                "THREADING_BINDING_HIL_FAIL line=%u function=%s\n",
                location.line(),
                location.function_name()
            );
            std::fflush(stdout);
            std::abort();
        }
    }

    /// Reads one synchronized fixture observation count.
    unsigned Count(std::size_t slot) noexcept {
        ObservationLock.Acquire();
        const auto result = Calls[slot];
        static_cast<void>(ObservationLock.Release());
        return result;
    }

    /// Validates the actual native borrowed record before recording eligible work.
    void Observe(std::size_t slot) noexcept {
        Carrier carrier;
        const auto current = carrier.ReadCurrent();
        Require(current.GetIfBound() == &Records[slot]);
        Require(current.GetIfBound()->Identity != 0U);
        Require(current.GetIfBound()->Slot == slot);
        ObservationLock.Acquire();
        ++Calls[slot];
        static_cast<void>(ObservationLock.Release());
    }

    /// Real carrier adapter with one explicit unsupported-entry fixture injection.
    class FaultBinding final : public Framework::Provider<ESPressio::Platform::Domain, Carrier::CompositionOffers> {
        private:

            /// Actual SDK-owning carrier; no private TLS is introduced.
            Carrier _carrier;
        public:

            /// Exact Platform record contract.
            using ContextRecord = Record;

            /// Creates a fixture carrier.
            FaultBinding() noexcept = default;

            /// Prevents copying carrier ownership.
            FaultBinding(const FaultBinding&) = delete;

            /// Prevents assigning carrier ownership.
            FaultBinding& operator =(const FaultBinding&) = delete;

            /// Injects a precise refusal only at the selected real owner entry.
            Execution::CurrentContextBindResult BindCurrent(const Record& record) noexcept {
                if (CurrentScenario == ScenarioKind::UnsupportedBinding && record.Slot == 3U) {
                    return Execution::CurrentContextBindResult::UnsupportedContext;
                }
                return _carrier.BindCurrent(record);
            }

            /// Reads genuine native TLS.
            Execution::CurrentContextReadResult<Record> ReadCurrent() const noexcept { return _carrier.ReadCurrent(); }

            /// Uses genuine expected-record retirement.
            Execution::CurrentContextRetireResult RetireCurrent(const Record& record) noexcept { return _carrier.RetireCurrent(record); }
    };

    /// Actual native execution adapter with fixture-only Start/entry fault injection.
    class FaultExecution final : public Framework::Provider<ESPressio::Platform::Domain, Native::CompositionOffers> {
        private:

            /// Actual native provider owns task and Join/Destroy proof.
            Native _native;

            /// Genuine Threading entry.
            Execution::ExecutionEntry _entry = nullptr;

            /// Genuine Threading entry argument.
            void* _parameter = nullptr;

            /// Deterministic initialization ordinal.
            unsigned _ordinal = 0U;

            /// Encloses actual Threading entry with fixture-owned setup and exact foreign preservation checks.
            static void Entry(void* opaque) noexcept {
                auto& self = *static_cast<FaultExecution*>(opaque);
                Carrier carrier;
                Require(carrier.ReadCurrent().Status() == Execution::CurrentContextReadStatus::Unbound);
                ObservationLock.Acquire();
                ++FreshEntries;
                static_cast<void>(ObservationLock.Release());
                if (CurrentScenario == ScenarioKind::LateEntry && self._ordinal == 0U) {
                    Require(LateGate.Wait(Sync::WaitTimeout::Forever()) == Sync::SignalWaitResult::Signaled);
                }
                if (CurrentScenario == ScenarioKind::ForeignBinding && self._ordinal == 1U) {
                    Require(carrier.BindCurrent(Foreign) == Execution::CurrentContextBindResult::Bound);
                }
                self._entry(self._parameter);
                const auto after = carrier.ReadCurrent();
                if (after.IsBound()) {
                    Require(after.GetIfBound() == &Foreign);
                    ObservationLock.Acquire();
                    ++PreservedForeign;
                    static_cast<void>(ObservationLock.Release());
                    // Only this fixture installed Foreign; Threading must never clear that record.
                    Require(carrier.RetireCurrent(Foreign) == Execution::CurrentContextRetireResult::Retired);
                }
            }

        public:

            /// Reset only after complete previous native teardown.
            static inline unsigned NextOrdinal = 0U;

            /// Creates an empty fixture owner.
            FaultExecution() noexcept = default;

            /// Prevents copying native ownership.
            FaultExecution(const FaultExecution&) = delete;

            /// Prevents assigning native ownership.
            FaultExecution& operator =(const FaultExecution&) = delete;

            /// Delegates native task creation and caller backing to the qualified provider.
            Execution::ExecutionInitializationResult Initialize(
                const Execution::ExecutionStorage& storage,
                const Execution::ExecutionConfiguration& configuration,
                Execution::ExecutionEntry entry,
                void* parameter
            ) noexcept {
                _entry = entry;
                _parameter = parameter;
                _ordinal = NextOrdinal++;
                return _native.Initialize(
                    storage,
                    configuration,
                    &Entry,
                    this
                );
            }

            /// Fails the second ordinary Worker Start without touching its initialized native task.
            Execution::ExecutionStartResult Start() noexcept {
                if (CurrentScenario == ScenarioKind::PartialStart && _ordinal == 1U) {
                    return Execution::ExecutionStartResult::ProviderFailure;
                }
                return _native.Start();
            }

            /// Uses actual completion/parking proof within the exact finite owner budget.
            Execution::ExecutionJoinResult Join(Sync::WaitTimeout timeout) noexcept {
                Require(!timeout.IsForever());
                return _native.Join(timeout);
            }

            /// Never bypasses native refusal or force-deletes a running task.
            Execution::ExecutionDestroyResult Destroy() noexcept { return _native.Destroy(); }

            /// Preserves actual native identity inspection.
            bool IsCurrentContext() const noexcept { return _native.IsCurrentContext(); }

            /// Exposes genuine stack telemetry.
            Execution::ExecutionStackTelemetry GetStackTelemetry() const noexcept { return _native.GetStackTelemetry(); }

            /// Uses native scheduler yield.
            static void Yield() noexcept { Native::Yield(); }
    };

    /// Ordinary pool identity.
    struct Pool final {};

    /// Dedicated activation identity.
    struct Dedicated final {};

    /// Isolated Worker callable.
    struct Critical final {

        /// Validates the actual isolated context.
        void operator ()() const noexcept { Observe(2U); }
    };

    /// Dedicated activation callable.
    struct DedicatedBody final {

        /// Exercises real expected-record mismatch only in the final retained-owner scenario.
        void operator ()() const noexcept {
            Observe(3U);
            if (CurrentScenario == ScenarioKind::RetirementMismatch) {
                Carrier carrier;
                Require(carrier.RetireCurrent(Records[3U]) == Execution::CurrentContextRetireResult::Retired);
                Require(carrier.BindCurrent(Foreign) == Execution::CurrentContextBindResult::Bound);
            }
        }
    };

    /// Two ordinary Workers expose partial facility startup; other roles remain independently addressed.
    using Topology = Threading::ThreadingTopology<
        Threading::TaskExecutionFacility<Pool,
            Threading::TaskRecordCapacity<4U>, Threading::CallableCapacity<32U>, Threading::ResultCapacity<8U>,
            Threading::Workers<
                Threading::Worker<Threading::StackCapacity<4096U>, Threading::Affinity<0U>>,
                Threading::Worker<Threading::StackCapacity<4096U>, Threading::Affinity<1U>>>>,
        Threading::DedicatedWorkerLease<Critical,
            Threading::TaskRecordCapacity<2U>, Threading::CallableCapacity<32U>, Threading::ResultCapacity<8U>,
            Threading::StackCapacity<4096U>, Threading::Affinity<1U>>,
        Threading::DedicatedThread<Dedicated, Threading::StackCapacity<4096U>, Threading::AnyAffinity>
    >;

    /// Real selected binding policy.
    using Policy = Threading::ContextBindingPolicy<FaultBinding, 4U, Signal, Spin>;

    /// Stable dedicated callable tuple.
    using Bindings = std::tuple<Threading::DedicatedThreadBinding<Dedicated, DedicatedBody>>;

    /// Actual production owner; lower adapters only inject the stated fixture faults.
    using Runtime = Threading::StaticThreadingRuntime<Topology, Bindings, Signal, FaultExecution, Spin, Mutex, Policy>;

    /// Complete owner reconstructed only after verified prior native retirement.
    struct Scenario final {

        /// Immutable records for this incarnation.
        const std::array<Record, 4U> Contexts;

        /// Immutable direct-slot mapping.
        const Policy::Mapping Mapping;

        /// Actual production policy and retained facts.
        Policy Registration;

        /// Actual production topology and native backing.
        Runtime Workers;

        /// Binds stable references without allocating or starting work.
        explicit Scenario(std::uint64_t base) noexcept :
            Contexts{{{base + 1U, 0U}, {base + 2U, 1U}, {base + 3U, 2U}, {base + 4U, 3U}}},
            Mapping{{{base + 1U, Contexts[0]}, {base + 2U, Contexts[1]}, {base + 3U, Contexts[2]}, {base + 4U, Contexts[3]}}},
            Registration(Mapping),
            Workers(
                Bindings{Threading::BindDedicatedThread<Dedicated>(DedicatedBody{})},
                Registration
            ) {}
    };

    /// One fixed caller-owned block; EDP-Memory owns explicit construction/destruction operations.
    alignas(Scenario) std::byte ScenarioStorage[sizeof(Scenario)];

    /// Enforces the outer fixture bound using the one canonical Clock.
    void CheckBudget(Clock::MonotonicTimestamp start) noexcept {
        Require(Clock::Delta(
            start,
            Clock::MonotonicNow()
        ).Nanoseconds() < 3000000000LL);
        Native::Yield();
    }

    /// Runs one actual native scenario; a retirement anomaly deliberately retains the final owner.
    void RunScenario(
        ScenarioKind kind,
        std::uint64_t identityBase
    ) noexcept {
        CurrentScenario = kind;
        FaultExecution::NextOrdinal = 0U;
        ObservationLock.Acquire();
        for (auto& count : Calls) { count = 0U; }
        FreshEntries = 0U;
        PreservedForeign = 0U;
        static_cast<void>(ObservationLock.Release());
        auto* scenario = ESPressio::Memory::ObjectLifetime::Construct<Scenario>(
            ScenarioStorage,
            identityBase
        );
        Records = scenario->Contexts.data();
        Require(scenario->Workers.Initialize() == Threading::ThreadingInitializationResult::Succeeded);
        const auto startup = scenario->Workers.StartFor(Clock::Duration::FromNanoseconds(
            kind == ScenarioKind::LateEntry ? 20000000LL : 2000000000LL
        ));
        if (kind == ScenarioKind::Success || kind == ScenarioKind::RetirementMismatch) {
            Require(startup == Threading::ThreadingStartResult::Succeeded);
            for (unsigned dispatch = 0U; dispatch < 4U; ++dispatch) {
                auto result = scenario->Workers.Dispatch<Pool>(
                    []() noexcept {
                        Carrier carrier;
                        const auto current = carrier.ReadCurrent();
                        Require(current.IsBound());
                        Require(current.GetIfBound()->Slot < 2U);
                        Observe(current.GetIfBound()->Slot);
                    },
                    Threading::TaskDispatchPolicy::Queue
                );
                Require(result.IsSucceeded());
                auto task = result.TakeTask();
                const auto start = Clock::MonotonicNow();
                while (!task.IsFinished()) { CheckBudget(start); }
            }
            auto isolated = scenario->Workers.DispatchDedicated<Critical>(
                Critical{},
                Threading::TaskDispatchPolicy::Queue
            );
            Require(isolated.IsSucceeded());
            auto isolatedTask = isolated.TakeTask();
            auto start = Clock::MonotonicNow();
            while (!isolatedTask.IsFinished()) { CheckBudget(start); }
            const unsigned activations = kind == ScenarioKind::Success ? 3U : 1U;
            for (unsigned activation = 0U; activation < activations; ++activation) {
                Require(scenario->Workers.StartThread<Dedicated>() == Threading::ThreadStartResult::Started);
                auto thread = scenario->Workers.ThreadHandle<Dedicated>();
                start = Clock::MonotonicNow();
                while (thread.State() != Threading::ThreadState::Stopped) { CheckBudget(start); }
            }
            Require(scenario->Workers.BeginShutdown() == Threading::ThreadingShutdownResult::Accepted);
        } else {
            Require(startup == Threading::ThreadingStartResult::ProviderFailure);
            for (std::size_t slot = 0U; slot < 4U; ++slot) { Require(Count(slot) == 0U); }
            Require(scenario->Workers.StartThread<Dedicated>() == Threading::ThreadStartResult::ShuttingDown);
            if (kind == ScenarioKind::ForeignBinding) {
                Require(scenario->Registration.Snapshot(1U)->Phase == Threading::ContextBindingPhase::AlreadyBound);
            }
            if (kind == ScenarioKind::UnsupportedBinding) {
                Require(scenario->Registration.Snapshot(3U)->Phase == Threading::ContextBindingPhase::UnsupportedContext);
            }
            if (kind == ScenarioKind::LateEntry) {
                Require(scenario->Registration.StartupResult() == Threading::ContextBindingStartupResult::TimedOut);
                Require(scenario->Workers.FinalizeShutdownFor(Clock::Duration{}) != Threading::ThreadingFinalizationResult::Completed);
                Require(scenario->Registration.Snapshot(0U)->NativeState != Threading::ContextBindingNativeState::Destroyed);
                Require(LateGate.Notify() == Sync::SignalNotifyResult::Signaled);
            }
        }
        const auto start = Clock::MonotonicNow();
        auto cleanup = Threading::ThreadingFinalizationResult::ProviderFailure;
        do {
            cleanup = scenario->Workers.FinalizeShutdownFor(Clock::Duration::FromNanoseconds(10000000LL));
            if (kind == ScenarioKind::RetirementMismatch &&
                scenario->Registration.Snapshot(3U)->NativeState == Threading::ContextBindingNativeState::Joined) {
                Require(cleanup == Threading::ThreadingFinalizationResult::ProviderFailure);
                Require(scenario->Registration.Snapshot(3U)->Phase == Threading::ContextBindingPhase::RetirementDifferentBinding);
                std::puts("THREADING_BINDING_HIL_RETIREMENT_RETAINED_PASS");
                // This final native owner/records remain live until firmware reset, without reuse.
                return;
            }
            if (cleanup != Threading::ThreadingFinalizationResult::Completed) { CheckBudget(start); }
        } while (cleanup != Threading::ThreadingFinalizationResult::Completed);
        for (std::size_t slot = 0U; slot < 4U; ++slot) {
            Require(scenario->Registration.Snapshot(slot)->NativeState == Threading::ContextBindingNativeState::Destroyed);
        }
        if (kind == ScenarioKind::ForeignBinding) { Require(PreservedForeign == 1U); }
        if (kind != ScenarioKind::PartialStart) { Require(FreshEntries == 4U); }
        std::printf(
            "THREADING_BINDING_HIL_CASE_PASS kind=%u identity_base=%llu fresh=%u foreign=%u\n",
            static_cast<unsigned>(kind),
            static_cast<unsigned long long>(identityBase),
            FreshEntries,
            PreservedForeign
        );
        ESPressio::Memory::ObjectLifetime::Destroy(*scenario);
    }

    /// Runs failure handling and twenty additional fixed-backing native owner incarnations.
    void Run() noexcept {
        using Timebase = ESPressio::Platform::ESPIDF::Timing::EspTimerMonotonicTimebaseProvider;
        static Timebase source;
        static Clock::MonotonicClockProvider<Timebase> clock(source);
        Require(Clock::BindMonotonicClock(clock));
        std::uint64_t identity = 1000U;
        for (unsigned kind = 0U; kind < 5U; ++kind) {
            RunScenario(
                static_cast<ScenarioKind>(kind),
                identity
            );
            identity += 10U;
        }
        for (unsigned incarnation = 0U; incarnation < 20U; ++incarnation) {
            RunScenario(
                ScenarioKind::Success,
                identity
            );
            identity += 10U;
        }
        RunScenario(
            ScenarioKind::RetirementMismatch,
            identity
        );
        std::puts("THREADING_BINDING_HIL_PASS incarnations=26 native_contexts=104\n");
        std::fflush(stdout);
    }

} // Qualification

#ifdef ARDUINO
#include <Arduino.h>
/// Runs after framework startup.
void setup() { Qualification::Run(); }
/// Retains the terminal failed owner after successful refusal qualification.
void loop() { delay(1000); }
#else
/// Runs from the SDK application entry.
extern "C" void app_main() { Qualification::Run(); }
#endif
