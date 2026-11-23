#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <tuple>

#include <ESPressio_Clock.hpp>
#include <ESPressio_Platform_FreeRTOS.hpp>
#include <ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp>
#include <ESPressio_Platform_ESP_IDF_ExecutionContext.hpp>
#include <ESPressio_Platform_ESP_IDF_SpinLock.hpp>
#include <timing/EspTimerMonotonicTimebaseProvider.hpp>
#include <ESPressio_Threading_ContextBinding.hpp>

namespace Example {

    namespace Threading = ESPressio::Threading;
    namespace Clock = ESPressio::Clock;
    namespace Execution = ESPressio::Platform::Execution;

    /// Existing concrete native execution owner with qualified Join/Destroy lifetime behavior.
    using Native = ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;
    /// Existing targeted wake owner.
    using Signal = ESPressio::Platform::FreeRTOS::Synchronization::SignalProvider;
    /// Existing ordinary resource mutex owner.
    using Mutex = ESPressio::Platform::FreeRTOS::Synchronization::MutexProvider;
    /// Existing cross-core setup/observation serialization owner.
    using SpinLock = ESPressio::Platform::ESPIDF::Synchronization::SpinLockProvider;

    /// Application-owned domain-neutral record borrowed by native context binding.
    struct Record final {
        /// Immutable nonzero identity assigned by this application's session owner.
        std::uint64_t Identity;
        /// Direct bounded application capture/observation slot.
        std::size_t Slot;
    };

    /// Qualified compiler-native TLS carrier; no FreeRTOS application TLS slot is reserved.
    using Binding = ESPressio::Platform::ESPIDF::Execution::CurrentContextBindingProvider<Record>;

    /// Complete immutable record storage, retained for the firmware lifetime.
    inline const std::array<Record, 3U> Records{{{101U, 0U}, {102U, 1U}, {103U, 2U}}};
    /// Serializes demonstration counters through the existing Platform owner.
    inline SpinLock ObservationLock;
    /// Fixed demonstration counters; this is application state, not profiling storage.
    inline std::array<unsigned, 3U> Observations{};

    /// Stops the demonstration on an observed contract violation, without reporting a false PASS.
    inline void Require(bool condition) noexcept {
        if (!condition) {
            std::puts("THREADING_BINDING_DEMO_FAIL");
            std::fflush(stdout);
            std::abort();
        }
    }

    /// Reads the actual current-context record in O(1), then updates application-owned observation state.
    inline void Observe(std::size_t expectedSlot) noexcept {
        Binding binding;
        const auto current = binding.ReadCurrent();
        Require(current.IsBound());
        Require(current.GetIfBound() == &Records[expectedSlot]);
        Require(current.GetIfBound()->Identity == 101U + expectedSlot);
        Require(current.GetIfBound()->Slot == expectedSlot);
        ObservationLock.Acquire();
        ++Observations[expectedSlot];
        static_cast<void>(ObservationLock.Release());
    }

    /// Returns one synchronized application observation count.
    inline unsigned Count(std::size_t slot) noexcept {
        ObservationLock.Acquire();
        const auto count = Observations[slot];
        static_cast<void>(ObservationLock.Release());
        return count;
    }

    /// Semantic identity of ordinary finite work.
    struct Pool final {};
    /// Semantic identity of restartable dedicated activation.
    struct Dedicated final {};
    /// Type-restricted work serviced by one isolated persistent Worker.
    struct CriticalWork final {
        /// Demonstrates that the isolated native Worker retains its own record.
        int operator ()() const noexcept {
            Observe(1U);
            return 42;
        }
    };
    /// Application body activated repeatedly inside one persistent bound native context.
    struct DedicatedBody final {
        /// Demonstrates binding before and after a managed block; stop remains cooperative.
        void operator ()(Threading::ThreadContext& context) const noexcept {
            Observe(2U);
            while (!context.IsStopRequested()) {
                Require(context.Wait() == Threading::ThreadWaitResult::Woken);
                if (!context.IsStopRequested()) { Observe(2U); }
            }
        }
    };

    /// Three statically provisioned native contexts; capacities are explicit application choices.
    using Topology = Threading::ThreadingTopology<
        Threading::TaskExecutionFacility<Pool,
            Threading::TaskRecordCapacity<4U>, Threading::CallableCapacity<32U>, Threading::ResultCapacity<8U>,
            Threading::Workers<Threading::Worker<Threading::StackCapacity<8192U>, Threading::Affinity<0U>>>>,
        Threading::DedicatedWorkerLease<CriticalWork,
            Threading::TaskRecordCapacity<2U>, Threading::CallableCapacity<32U>, Threading::ResultCapacity<8U>,
            Threading::StackCapacity<8192U>, Threading::Affinity<1U>>,
        Threading::DedicatedThread<Dedicated, Threading::StackCapacity<8192U>, Threading::AnyAffinity>
    >;

    /// Explicit opt-in policy; ordinary six-argument runtimes retain the unselected default.
    using Policy = Threading::ContextBindingPolicy<Binding, Topology::ManagedExecutionContextCount, Signal, SpinLock>;
    /// Fixed callable binding tuple, owned by the runtime.
    using Bindings = std::tuple<Threading::DedicatedThreadBinding<Dedicated, DedicatedBody>>;
    /// Complete static Threading owner with explicit context binding.
    using Runtime = Threading::StaticThreadingRuntime<Topology, Bindings, Signal, Native, SpinLock, Mutex, Policy>;

    /// Stable mapping from existing topology positions to session identities and borrowed records.
    inline const Policy::Mapping Mapping{{{101U, Records[0]}, {102U, Records[1]}, {103U, Records[2]}}};
    /// One address-stable policy; no session identity is allocated by the carrier or Threading.
    inline Policy Contexts(Mapping);
    /// One address-stable runtime; a failed bounded cleanup never destroys this storage.
    inline Runtime Workers(
        Bindings{Threading::BindDedicatedThread<Dedicated>(DedicatedBody{})},
        Contexts
    );

    /// Checks progress against the same application-wide canonical Clock used by Threading.
    inline void CheckBudget(Clock::MonotonicTimestamp start) noexcept {
        Require(Clock::Delta(
            start,
            Clock::MonotonicNow()
        ).Nanoseconds() < 2000000000LL);
        Native::Yield();
    }

    /// Waits for an unmanaged Bootstrap caller's finite Task result using a bounded application check.
    /// @tparam TTask Concrete move-only Task handle Type returned by dispatch.
    template<class TTask>
    inline void CheckTask(TTask& task) {
        const auto start = Clock::MonotonicNow();
        while (!task.IsFinished()) { CheckBudget(start); }
        auto result = task.TakeResult();
        Require(result.IsSucceeded());
        Require(result.TakeResult() == 42);
    }

    /// Runs a complete basic integration, with explicit finite startup and shutdown budgets.
    inline void Run() {
        using Timebase = ESPressio::Platform::ESPIDF::Timing::EspTimerMonotonicTimebaseProvider;
        static Timebase source;
        static Clock::MonotonicClockProvider<Timebase> monotonic(source);
        Require(Clock::BindMonotonicClock(monotonic));
        Require(Workers.Initialize() == Threading::ThreadingInitializationResult::Succeeded);
        Require(Workers.StartFor(Clock::Duration::FromNanoseconds(2000000000LL)) == Threading::ThreadingStartResult::Succeeded);
        for (unsigned iteration = 0U; iteration < 3U; ++iteration) {
            auto ordinary = Workers.Dispatch<Pool>(
                []() noexcept { Observe(0U); return 42; },
                Threading::TaskDispatchPolicy::Queue
            );
            Require(ordinary.IsSucceeded());
            auto ordinaryTask = ordinary.TakeTask();
            CheckTask(ordinaryTask);
            auto critical = Workers.DispatchDedicated<CriticalWork>(
                CriticalWork{},
                Threading::TaskDispatchPolicy::Queue
            );
            Require(critical.IsSucceeded());
            auto criticalTask = critical.TakeTask();
            CheckTask(criticalTask);
            auto dedicated = Workers.ThreadHandle<Dedicated>();
            const auto before = Count(2U);
            Require(dedicated.Start() == Threading::ThreadStartResult::Started);
            auto start = Clock::MonotonicNow();
            while (Count(2U) == before) { CheckBudget(start); }
            Require(dedicated.Wake() == Threading::ThreadWakeResult::Woken);
            start = Clock::MonotonicNow();
            while (Count(2U) == before + 1U) { CheckBudget(start); }
            Require(dedicated.RequestStop() == Threading::ThreadStopRequestResult::Accepted);
            start = Clock::MonotonicNow();
            while (dedicated.State() != Threading::ThreadState::Stopped) { CheckBudget(start); }
        }
        Require(Workers.BeginShutdown() == Threading::ThreadingShutdownResult::Accepted);
        Require(Workers.FinalizeShutdownFor(Clock::Duration::FromNanoseconds(2000000000LL)) == Threading::ThreadingFinalizationResult::Completed);
        for (std::size_t slot = 0U; slot < Records.size(); ++slot) {
            const auto facts = Contexts.Snapshot(slot);
            Require(facts.has_value());
            Require(facts->Phase == Threading::ContextBindingPhase::Retired);
            Require(facts->NativeState == Threading::ContextBindingNativeState::Destroyed);
            Require(!facts->HasAbnormalExit);
        }
        std::printf(
            "THREADING_BINDING_DEMO_PASS ordinary=%u isolated=%u dedicated=%u policy_bytes=%u runtime_bytes=%u\n",
            Count(0U),
            Count(1U),
            Count(2U),
            static_cast<unsigned>(sizeof(Policy)),
            static_cast<unsigned>(sizeof(Runtime))
        );
    }

} // Example
