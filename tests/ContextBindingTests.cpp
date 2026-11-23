#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string_view>
#include <thread>

#include <ESPressio_Threading_ContextBinding.hpp>
#include <synchronization/PortableSynchronizationProviders.hpp>

namespace Test {

    namespace Execution = ESPressio::Platform::Execution;
    namespace Sync = ESPressio::Platform::Synchronization;
    namespace Framework = ESPressio::System::CompositionFramework;
    namespace Threading = ESPressio::Threading;
    using Signal = ESPressio::Platform::Portable::Synchronization::SignalProvider;
    using Mutex = ESPressio::Platform::Portable::Synchronization::MutexProvider;
    using Spin = ESPressio::Platform::Portable::Synchronization::SpinLockProvider;

    /// Test Clock boundary: one process-wide physical coordinate, bound through canonical EDP-Clock.
    struct Clock final {
        /// Supplies the physical host coordinate exclusively through the selected test Clock.
        ESPressio::Clock::MonotonicTimestamp Now() const noexcept {
            return ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(
                std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
            );
        }
    };

    /// Minimal owner record; real applications supply their already-owned capture descriptor.
    struct Record final {
        /// Nonzero owner-assigned session identity.
        std::uint64_t Identity;
    };

    /// Host-only carrier fixture with explicit per-context fault injection.
    class Binding final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Offers<Framework::Offer<
            Execution::CurrentContextBinding,
            Framework::PropertyValue<Execution::ConstantTimeLookup, true>,
            Framework::PropertyValue<Execution::AllocationFreeLookup, true>,
            Framework::PropertyValue<Execution::CurrentContextOnly, true>,
            Framework::PropertyValue<Execution::RejectsInterruptContexts, true>
        >>
    > {
        public:
            /// Exact record consumed by the Platform contract.
            using ContextRecord = Record;
            /// Fixture task-local binding; never a production fallback.
            static inline thread_local const Record* Current = nullptr;
            /// Selected nonzero identity rejected as AlreadyBound.
            static inline std::atomic<std::uint64_t> RejectIdentity{0U};
            /// Selected nonzero identity rejected as unsupported.
            static inline std::atomic<std::uint64_t> UnsupportedIdentity{0U};
            /// Known foreign binding used to prove refusal preserves another owner record.
            static inline const Record Foreign{999999U};
            /// Test-only retirement failure override; -1 selects ordinary behavior.
            static inline std::atomic<int> RetirementFailure{-1};
            /// Counts exact owner retire calls.
            static inline std::atomic<unsigned> RetireCalls{0U};
            /// Creates an unbound fixture carrier.
            Binding() = default;
            /// Prevents copying carrier ownership.
            Binding(const Binding&) = delete;
            /// Prevents assignment of carrier ownership.
            Binding& operator =(const Binding&) = delete;
            /// Binds or injects one precise failure without replacing another record.
            Execution::CurrentContextBindResult BindCurrent(const Record& record) noexcept {
                if (record.Identity == UnsupportedIdentity) { return Execution::CurrentContextBindResult::UnsupportedContext; }
                if (record.Identity == RejectIdentity) {
                    Current = &Foreign;
                    return Execution::CurrentContextBindResult::AlreadyBound;
                }
                if (Current != nullptr) { return Execution::CurrentContextBindResult::AlreadyBound; }
                Current = &record;
                return Execution::CurrentContextBindResult::Bound;
            }
            /// Returns the exact thread-local borrowed record.
            Execution::CurrentContextReadResult<Record> ReadCurrent() const noexcept {
                return Current == nullptr ? Execution::CurrentContextReadResult<Record>::ForUnbound() : Execution::CurrentContextReadResult<Record>::ForBound(*Current);
            }
            /// Retires only the exact expected record.
            Execution::CurrentContextRetireResult RetireCurrent(const Record& record) noexcept {
                ++RetireCalls;
                if (RetirementFailure >= 0) { return static_cast<Execution::CurrentContextRetireResult>(RetirementFailure.load()); }
                if (Current == nullptr) { return Execution::CurrentContextRetireResult::NotBound; }
                if (Current != &record) { return Execution::CurrentContextRetireResult::DifferentBinding; }
                Current = nullptr;
                return Execution::CurrentContextRetireResult::Retired;
            }
    };

    /// Host fixture starts real standard threads; native placement/stack qualification is tested separately.
    class ExecutionProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Offers<Framework::Offer<
            Execution::ExecutionContext,
            Framework::PropertyValue<Execution::CallerSuppliedStorage, true>,
            Framework::PropertyValue<Execution::SupportsPriority, true>,
            Framework::PropertyValue<Execution::SupportsProcessorAffinity, true>,
            Framework::PropertyValue<Execution::SupportsStackTelemetry, false>,
            Framework::PropertyValue<Execution::ControlStorageBytes, 16U>,
            Framework::PropertyValue<Execution::ControlStorageAlignment, 8U>,
            Framework::PropertyValue<Execution::StackStorageAlignment, 16U>,
            Framework::PropertyValue<Execution::StackAllocationGranularityBytes, 16U>,
            Framework::PropertyValue<Execution::JoinWaitResolutionNanoseconds, 1U>
        >>
    > {
        private:
            /// Actual host worker, used only in this fixture.
            std::thread _thread;
            /// Host completion wake.
            Signal _completion;
            /// Stored actual Threading native entry and argument.
            Execution::ExecutionEntry _entry = nullptr;
            /// Stored entry parameter.
            void* _parameter = nullptr;
            /// Stable test ordinal assigned during deterministic initialization.
            int _ordinal = -1;
            /// Indicates whether Join has already consumed host completion.
            bool _joined = false;
        public:
            /// Next fixture ordinal; reset only between fully retired runtimes.
            static inline int NextOrdinal = 0;
            /// Optional Start failure ordinal.
            static inline int FailStartAt = -1;
            /// Optional entry gate ordinal.
            static inline int HoldEntryAt = -1;
            /// Explicitly controlled late-entry gate.
            static inline std::atomic<bool> EntryAllowed{true};
            /// Counts native destroy operations for incomplete-cleanup assertions.
            static inline unsigned Destroys = 0U;
            /// Creates empty host native ownership.
            ExecutionProvider() = default;
            /// Prevents copying a native context.
            ExecutionProvider(const ExecutionProvider&) = delete;
            /// Prevents assigning a native context.
            ExecutionProvider& operator =(const ExecutionProvider&) = delete;
            /// Ensures every test retains ownership until native Join.
            ~ExecutionProvider() { assert(!_thread.joinable()); }
            /// Captures the genuine Threading entry without running it during setup.
            Execution::ExecutionInitializationResult Initialize(
                const Execution::ExecutionStorage&,
                const Execution::ExecutionConfiguration&,
                Execution::ExecutionEntry entry,
                void* parameter
            ) noexcept {
                _entry = entry;
                _parameter = parameter;
                _ordinal = NextOrdinal++;
                return Execution::ExecutionInitializationResult::Succeeded;
            }
            /// Starts actual independent execution or injects the selected failure.
            Execution::ExecutionStartResult Start() noexcept {
                if (_ordinal == FailStartAt) { return Execution::ExecutionStartResult::ProviderFailure; }
                _thread = std::thread([this]() {
                    while (_ordinal == HoldEntryAt && !EntryAllowed.load()) { std::this_thread::yield(); }
                    _entry(_parameter);
                    if (Binding::Current != nullptr) { assert(Binding::Current == &Binding::Foreign); }
                    static_cast<void>(_completion.Notify());
                });
                return Execution::ExecutionStartResult::Succeeded;
            }
            /// Joins only within the exact finite timeout supplied by the production owner.
            Execution::ExecutionJoinResult Join(Sync::WaitTimeout timeout) noexcept {
                assert(!timeout.IsForever());
                if (_joined) { return Execution::ExecutionJoinResult::Succeeded; }
                if (_completion.Wait(timeout) != Sync::SignalWaitResult::Signaled) { return Execution::ExecutionJoinResult::TimedOut; }
                _thread.join();
                _joined = true;
                return Execution::ExecutionJoinResult::Succeeded;
            }
            /// Refuses to destroy any still-running standard thread.
            Execution::ExecutionDestroyResult Destroy() noexcept {
                assert(!_thread.joinable());
                ++Destroys;
                return Execution::ExecutionDestroyResult::Succeeded;
            }
            /// Resolves current context using the real standard thread identity.
            bool IsCurrentContext() const noexcept { return _thread.get_id() == std::this_thread::get_id(); }
            /// Makes no native stack claim from a host fixture.
            Execution::ExecutionStackTelemetry GetStackTelemetry() const noexcept { return {}; }
            /// Yields actual host execution.
            static void Yield() noexcept { std::this_thread::yield(); }
    };

    /// Ordinary pool identity.
    struct Pool final {};
    /// Dedicated activation identity.
    struct Dedicated final {};
    /// Dedicated worker callable checks its persistent native record.
    struct CriticalTask final {
        /// Publishes a truthful user-work observation from inside the binding.
        void operator ()() const noexcept { assert(Binding::Current != nullptr); }
    };
    /// One ordinary facility, one isolated worker and one dedicated native context.
    using Topology = Threading::ThreadingTopology<
        Threading::TaskExecutionFacility<Pool, Threading::TaskRecordCapacity<4U>, Threading::CallableCapacity<64U>, Threading::ResultCapacity<16U>,
            Threading::Workers<Threading::Worker<Threading::StackCapacity<256U>>, Threading::Worker<Threading::StackCapacity<256U>>>>,
        Threading::DedicatedWorkerLease<CriticalTask, Threading::TaskRecordCapacity<2U>, Threading::CallableCapacity<64U>, Threading::ResultCapacity<16U>, Threading::StackCapacity<256U>>,
        Threading::DedicatedThread<Dedicated, Threading::StackCapacity<256U>>
    >;
    /// Exact policy consumes the existing direct topology index count.
    using Policy = Threading::ContextBindingPolicy<Binding, Topology::ManagedExecutionContextCount, Signal, Spin>;
    /// Exception injection selected only for the separate host subprocess.
    std::atomic<bool> ThrowFromDedicated{false};

    /// Typed dedicated activation body used across repeated activations.
    struct DedicatedBody final {
        /// Number of actual completed activation bodies.
        std::atomic<unsigned>* Calls;
        /// Runs inside the same record throughout persistent native lifetime.
        void operator ()() const {
            assert(Binding::Current != nullptr && Binding::Current->Identity % 10U == 4U);
            if (ThrowFromDedicated) { throw std::runtime_error("test owner exit"); }
            ++*Calls;
        }
    };
    /// Fixed callable binding tuple.
    using Bindings = std::tuple<Threading::DedicatedThreadBinding<Dedicated, DedicatedBody>>;
    /// Actual production topology owner with the explicit optional policy.
    using Runtime = Threading::StaticThreadingRuntime<Topology, Bindings, Signal, ExecutionProvider, Spin, Mutex, Policy>;

    /// Resets fixture faults only after all previous host threads were joined.
    void Reset() {
        ExecutionProvider::NextOrdinal = 0;
        ExecutionProvider::FailStartAt = -1;
        ExecutionProvider::HoldEntryAt = -1;
        ExecutionProvider::EntryAllowed = true;
        ExecutionProvider::Destroys = 0U;
        Binding::RejectIdentity = 0U;
        Binding::UnsupportedIdentity = 0U;
        Binding::RetireCalls = 0U;
        Binding::RetirementFailure = -1;
    }

    /// Session-wide test identity sequence; no identity is recycled across native owner incarnations.
    std::uint64_t NextIdentityBase = 10U;

    /// Exercises real entry, persistent dispatch/activation, partial Start, late acknowledgement and exact failures.
    void RunCase(unsigned test) {
        Reset();
        const std::uint64_t epoch = NextIdentityBase;
        NextIdentityBase += 10U;
        std::array<Record, 4U> records{{{epoch + 1U}, {epoch + 2U}, {epoch + 3U}, {epoch + 4U}}};
        const Policy::Mapping mapping{{{records[0].Identity, records[0]}, {records[1].Identity, records[1]}, {records[2].Identity, records[2]}, {records[3].Identity, records[3]}}};
        Policy policy(mapping);
        std::atomic<unsigned> calls{0U};
        Runtime runtime(
            Bindings{Threading::BindDedicatedThread<Dedicated>(DedicatedBody{&calls})},
            policy
        );
        assert(runtime.Initialize() == Threading::ThreadingInitializationResult::Succeeded);
        assert(runtime.Dispatch<Pool>(
            []() {},
            Threading::TaskDispatchPolicy::Queue
        ).Status() == Threading::TaskDispatchStatus::ShuttingDown);
        if (test == 1U) { Binding::RejectIdentity = records[1].Identity; }
        if (test == 2U) { Binding::UnsupportedIdentity = records[3].Identity; }
        if (test == 3U) { ExecutionProvider::FailStartAt = 1; }
        if (test == 4U) { ExecutionProvider::HoldEntryAt = 0; ExecutionProvider::EntryAllowed = false; }
        const auto budget = Threading::Duration::FromNanoseconds(test == 4U ? 20000000 : test == 5U ? 0 : 2000000000);
        const auto started = test == 0U && epoch % 20U == 0U
            ? runtime.StartInOrderFor<2U, 0U, 1U>(budget)
            : runtime.StartFor(budget);
        if (test == 0U) {
            assert(started == Threading::ThreadingStartResult::Succeeded);
            for (unsigned iteration = 0U; iteration < 2U; ++iteration) {
                auto dispatched = runtime.Dispatch<Pool>(
                    [&calls]() { assert(Binding::Current != nullptr); ++calls; },
                    Threading::TaskDispatchPolicy::Queue
                );
                assert(dispatched.IsSucceeded());
                auto task = dispatched.TakeTask();
                while (task.State() != Threading::TaskState::Completed) { std::this_thread::yield(); }
                auto isolated = runtime.DispatchDedicated<CriticalTask>(
                    CriticalTask{},
                    Threading::TaskDispatchPolicy::Queue
                );
                assert(isolated.IsSucceeded());
                auto isolatedTask = isolated.TakeTask();
                while (isolatedTask.State() != Threading::TaskState::Completed) { std::this_thread::yield(); }
                assert(runtime.StartThread<Dedicated>() == Threading::ThreadStartResult::Started);
                const auto target = (iteration + 1U) * 2U;
                while (calls.load() != target) { std::this_thread::yield(); }
                auto handle = runtime.ThreadHandle<Dedicated>();
                while (handle.State() != Threading::ThreadState::Stopped) { std::this_thread::yield(); }
            }
            assert(runtime.BeginShutdown() == Threading::ThreadingShutdownResult::Accepted);
        } else {
            assert(started == Threading::ThreadingStartResult::ProviderFailure);
            assert(calls == 0U);
            assert(runtime.StartThread<Dedicated>() == Threading::ThreadStartResult::ShuttingDown);
            assert(runtime.Dispatch<Pool>(
                []() {},
                Threading::TaskDispatchPolicy::Queue
            ).Status() == Threading::TaskDispatchStatus::ShuttingDown);
            if (test == 1U) { assert(policy.Snapshot(1U)->Phase == Threading::ContextBindingPhase::AlreadyBound); }
            if (test == 2U) { assert(policy.Snapshot(3U)->Phase == Threading::ContextBindingPhase::UnsupportedContext); }
            if (test == 4U) {
                assert(policy.StartupResult() == Threading::ContextBindingStartupResult::TimedOut);
                assert(runtime.FinalizeShutdownFor(Threading::Duration{}) != Threading::ThreadingFinalizationResult::Completed);
                assert(policy.Snapshot(0U)->NativeState != Threading::ContextBindingNativeState::Destroyed);
                ExecutionProvider::EntryAllowed = true;
                assert(runtime.StartFor(Threading::Duration::FromNanoseconds(2000000000)) == Threading::ThreadingStartResult::NotInitialized);
            }
        }
        assert(runtime.FinalizeShutdownFor(Threading::Duration::FromNanoseconds(2000000000)) == Threading::ThreadingFinalizationResult::Completed);
        assert(ExecutionProvider::Destroys == 4U);
        for (unsigned index = 0U; index < 4U; ++index) {
            assert(policy.Snapshot(index)->NativeState == Threading::ContextBindingNativeState::Destroyed);
        }
        assert(!policy.Snapshot(4U).has_value());
        assert(Binding::RetireCalls == (test == 3U ? 1U : test == 1U || test == 2U ? 3U : 4U));
        std::printf(
            "CONTEXT_BINDING_CASE_PASS=%u\n",
            test
        );
    }

    /// Requires an explicit finite startup budget for a selected owner.
    template<class TOwner>
    concept HasUnboundedStart = requires(TOwner& owner) { owner.Start(); };
    static_assert(!HasUnboundedStart<Runtime>);
    static_assert(!std::is_constructible_v<Policy, Policy::Mapping&&>);

    /// Exercises invalid fixed mapping before any native initialization occurs.
    void InvalidMappings() {
        for (unsigned test = 0U; test < 3U; ++test) {
            Reset();
            std::array<Record, 4U> records{{{101U}, {102U}, {103U}, {104U}}};
            Policy::Mapping mapping{{{101U, records[0]}, {102U, records[1]}, {103U, records[2]}, {104U, records[3]}}};
            if (test == 0U) { mapping[0].Identity = 0U; }
            if (test == 1U) { mapping[1].Identity = mapping[0].Identity; }
            if (test == 2U) { mapping[1].Record = mapping[0].Record; }
            Policy policy(mapping);
            std::atomic<unsigned> calls{0U};
            Runtime runtime(
                Bindings{Threading::BindDedicatedThread<Dedicated>(DedicatedBody{&calls})},
                policy
            );
            assert(runtime.Initialize() == Threading::ThreadingInitializationResult::ProviderFailure);
            assert(ExecutionProvider::NextOrdinal == 0);
            assert(policy.StartupResult() == Threading::ContextBindingStartupResult::InvalidMapping);
        }
        std::puts("CONTEXT_BINDING_INVALID_MAPPING_PASS");
    }

    /// Host-only native proof fixture; no actual operating-system ownership is represented here.
    struct JoinedProvider final {
        /// Counts attempted native destruction.
        unsigned DestroyCalls = 0U;
        /// Confirms fixture native completion within the received bounded budget.
        Execution::ExecutionJoinResult Join(Sync::WaitTimeout timeout) noexcept {
            assert(!timeout.IsForever());
            return Execution::ExecutionJoinResult::Succeeded;
        }
        /// Would reveal improper reclamation following anomalous retirement.
        Execution::ExecutionDestroyResult Destroy() noexcept {
            ++DestroyCalls;
            return Execution::ExecutionDestroyResult::Succeeded;
        }
    };

    /// Verifies all retirement failures preserve ownership even after a successful native Join.
    void RetirementFailures() {
        for (int failure = 1; failure <= 3; ++failure) {
            Reset();
            std::array<Record, 4U> records{{{201U}, {202U}, {203U}, {204U}}};
            const Policy::Mapping mapping{{{201U, records[0]}, {202U, records[1]}, {203U, records[2]}, {204U, records[3]}}};
            Policy policy(mapping);
            assert(policy.Prepare() == Threading::ContextBindingStartupResult::Ready);
            policy.MarkInitialized(0U);
            policy.MarkStarted(0U);
            assert(policy.Enter(0U) == Execution::CurrentContextBindResult::Bound);
            Binding::RetirementFailure = failure;
            policy.Leave(0U);
            assert(Binding::Current == &records[0]);
            JoinedProvider native;
            auto budget = Threading::Detail::MonotonicWaitBudget::For(Threading::Duration::FromNanoseconds(2000000000));
            assert(policy.Cleanup(
                0U,
                native,
                budget
            ) == Threading::ThreadingFinalizationResult::ProviderFailure);
            assert(native.DestroyCalls == 0U);
            assert(policy.Snapshot(0U)->NativeState == Threading::ContextBindingNativeState::Joined);
            // Fixture teardown only: this synthetic carrier/native lifetime ends here, outside production APIs.
            Binding::Current = nullptr;
        }
        std::puts("CONTEXT_BINDING_RETIREMENT_FAILURE_PASS");
    }

    /// Separate process exercises actual exceptional native exit while retaining the failed owner.
    void ExceptionalExit() {
        Reset();
        std::array<Record, 4U> records{{{301U}, {302U}, {303U}, {304U}}};
        const Policy::Mapping mapping{{{301U, records[0]}, {302U, records[1]}, {303U, records[2]}, {304U, records[3]}}};
        Policy policy(mapping);
        std::atomic<unsigned> calls{0U};
        Runtime runtime(
            Bindings{Threading::BindDedicatedThread<Dedicated>(DedicatedBody{&calls})},
            policy
        );
        assert(runtime.Initialize() == Threading::ThreadingInitializationResult::Succeeded);
        assert(runtime.StartFor(Threading::Duration::FromNanoseconds(2000000000)) == Threading::ThreadingStartResult::Succeeded);
        ThrowFromDedicated = true;
        assert(runtime.StartThread<Dedicated>() == Threading::ThreadStartResult::Started);
        while (policy.Snapshot(3U)->Phase != Threading::ContextBindingPhase::Retired) { std::this_thread::yield(); }
        assert(policy.Snapshot(3U)->HasAbnormalExit);
        assert(runtime.BeginShutdown() == Threading::ThreadingShutdownResult::Accepted);
        assert(runtime.FinalizeShutdownFor(Threading::Duration::FromNanoseconds(1000000)) != Threading::ThreadingFinalizationResult::Completed);
        assert(ExecutionProvider::Destroys == 0U);
        std::puts("CONTEXT_BINDING_EXCEPTION_RETAINED_PASS");
        std::fflush(stdout);
        // The test subprocess terminates with the owner deliberately retained. This is not a production cleanup API.
        std::_Exit(0);
    }

} // Test

/// Runs actual production Threading ownership over a host-only carrier/native fixture.
int main(
    int argc,
    char** argv
) {
    Test::Clock clock;
    assert(ESPressio::Clock::BindMonotonicClock(clock));
    if (argc > 1 && std::string_view(argv[1]) == "--exception") { Test::ExceptionalExit(); }
    for (unsigned test = 0U; test < 6U; ++test) { Test::RunCase(test); }
    for (unsigned incarnation = 0U; incarnation < 20U; ++incarnation) { Test::RunCase(0U); }
    Test::InvalidMappings();
    Test::RetirementFailures();
    std::puts("CONTEXT_BINDING_HOST_PASS");
}
