#include <cassert>
#include <cstdint>
#include <type_traits>

#include <ESPressio_Threading.hpp>

#include "../src/threading/detail/DedicatedThreadControl.hpp"
#include "../src/threading/detail/DedicatedThreadRuntime.hpp"
#include "../src/threading/detail/DedicatedWorkerLeaseRuntime.hpp"
#include "../src/threading/detail/FacilityStorage.hpp"
#include "../src/threading/detail/InfrastructureLifecycle.hpp"
#include "../src/threading/detail/ShutdownWaitRuntime.hpp"
#include "../src/threading/detail/ShutdownCoordinator.hpp"
#include "../src/threading/detail/TaskFacilityCore.hpp"
#include "../src/threading/detail/TaskFacilityRuntime.hpp"
#include "../src/threading/detail/TaskPayloadAdapter.hpp"
#include "../src/threading/detail/TaskRecord.hpp"
#include "../src/threading/detail/TaskWorkerExecutionContext.hpp"
#include "../src/threading/detail/WaitRegistration.hpp"
#include "../src/threading/detail/WorkerLeaseScheduler.hpp"

namespace Test {

    class AtomicByteProvider final {

        public:

            class Word final {

                private:

                    // Test atomic state.

                    /// Test-owned byte value.
                    std::uint8_t _value = 0U;

                public:

                    // Test atomic operations.

                    /// Reads the test byte without ordering significance.
                    std::uint8_t LoadRelaxed() const noexcept {
                        return _value;
                    }

                    /// Reads the test byte with acquire-equivalent test semantics.
                    std::uint8_t LoadAcquire() const noexcept {
                        return _value;
                    }

                    /// Stores the test byte without ordering significance.
                    void StoreRelaxed(
                        std::uint8_t value
                    ) noexcept {
                        _value = value;
                    }

                    /// Stores the test byte with release-equivalent test semantics.
                    void StoreRelease(
                        std::uint8_t value
                    ) noexcept {
                        _value = value;
                    }

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


    namespace Framework = ESPressio::System::CompositionFramework;


    class ExecutionContextProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Provides<
            Framework::Offer<
                ESPressio::Platform::Execution::ExecutionContext,
                Framework::PropertyValue<ESPressio::Platform::Execution::CallerSuppliedStorage, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsPriority, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsProcessorAffinity, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsStackTelemetry, false>,
                Framework::PropertyValue<ESPressio::Platform::Execution::ControlStorageBytes, 24U>,
                Framework::PropertyValue<ESPressio::Platform::Execution::ControlStorageAlignment, 8U>,
                Framework::PropertyValue<ESPressio::Platform::Execution::StackStorageAlignment, 16U>,
                Framework::PropertyValue<ESPressio::Platform::Execution::StackAllocationGranularityBytes, 32U>,
                Framework::PropertyValue<ESPressio::Platform::Execution::JoinWaitResolutionNanoseconds, 1U>
            >
        >
    > {

        private:

            bool _initialized = false;

        public:

            ExecutionContextProvider() noexcept = default;
            ExecutionContextProvider(const ExecutionContextProvider&) = delete;
            ExecutionContextProvider& operator =(const ExecutionContextProvider&) = delete;
            ExecutionContextProvider(ExecutionContextProvider&&) = delete;
            ExecutionContextProvider& operator =(ExecutionContextProvider&&) = delete;

            ESPressio::Platform::Execution::ExecutionInitializationResult Initialize(
                const ESPressio::Platform::Execution::ExecutionStorage& storage,
                const ESPressio::Platform::Execution::ExecutionConfiguration&,
                ESPressio::Platform::Execution::ExecutionEntry,
                void*
            ) noexcept {
                if (
                    storage.ControlAddress == nullptr ||
                    storage.ControlBytes != 24U ||
                    storage.StackAddress == nullptr ||
                    storage.StackBytes != 128U
                ) {
                    return ESPressio::Platform::Execution::ExecutionInitializationResult::InvalidStorage;
                }

                _initialized = true;
                return ESPressio::Platform::Execution::ExecutionInitializationResult::Succeeded;
            }

            ESPressio::Platform::Execution::ExecutionStartResult Start() noexcept {
                return _initialized
                    ? ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
                    : ESPressio::Platform::Execution::ExecutionStartResult::InvalidState;
            }

            ESPressio::Platform::Execution::ExecutionJoinResult Join(
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                return _initialized
                    ? ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded
                    : ESPressio::Platform::Execution::ExecutionJoinResult::InvalidState;
            }

            ESPressio::Platform::Execution::ExecutionDestroyResult Destroy() noexcept {
                if (!_initialized) {
                    return ESPressio::Platform::Execution::ExecutionDestroyResult::InvalidState;
                }

                _initialized = false;
                return ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;
            }

            bool IsCurrentContext() const noexcept {
                return false;
            }

            ESPressio::Platform::Execution::ExecutionStackTelemetry GetStackTelemetry() const noexcept {
                return {};
            }

            static void Yield() noexcept {}

    };


    class MutexProvider final {

        public:

            ESPressio::Platform::Synchronization::LockAcquireResult Acquire(
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                return ESPressio::Platform::Synchronization::LockAcquireResult::Acquired;
            }

            ESPressio::Platform::Synchronization::LockReleaseResult Release() noexcept {
                return ESPressio::Platform::Synchronization::LockReleaseResult::Released;
            }

    };


    class ManagedContextRouter final {

        public:

            static constexpr std::size_t ContextCapacity = 3U;

        private:

            std::size_t _wakeCount = 0U;

        public:

            std::optional<std::uint8_t> CurrentContextIndex() const noexcept {
                return static_cast<std::uint8_t>(0U);
            }

            bool IsInterrupted(
                std::uint8_t
            ) const noexcept {
                return false;
            }

            ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                std::uint8_t,
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                return ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut;
            }

            ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                std::uint8_t
            ) noexcept {
                ++_wakeCount;
                return ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled;
            }

            std::size_t WakeCount() const noexcept {
                return _wakeCount;
            }

    };


    struct LifetimeResult final {

        inline static std::size_t DestructionCount = 0U;

        bool Active = true;

        LifetimeResult() noexcept = default;

        LifetimeResult(
            LifetimeResult&& other
        ) noexcept :
            Active(other.Active) {
            other.Active = false;
        }

        LifetimeResult(const LifetimeResult&) = delete;
        LifetimeResult& operator =(const LifetimeResult&) = delete;
        LifetimeResult& operator =(LifetimeResult&&) = delete;

        ~LifetimeResult() {
            if (Active) {
                ++DestructionCount;
            }
        }

    };


    struct LifetimeCallable final {

        inline static std::size_t DestructionCount = 0U;

        bool Active = true;

        LifetimeCallable() noexcept = default;

        LifetimeCallable(
            LifetimeCallable&& other
        ) noexcept :
            Active(other.Active) {
            other.Active = false;
        }

        LifetimeCallable(const LifetimeCallable&) = delete;
        LifetimeCallable& operator =(const LifetimeCallable&) = delete;
        LifetimeCallable& operator =(LifetimeCallable&&) = delete;

        ~LifetimeCallable() {
            if (Active) {
                ++DestructionCount;
            }
        }

        LifetimeResult operator ()() noexcept {
            return LifetimeResult{};
        }

    };


    class ShutdownTaskResource final {

        public:

            bool CancellationStarted = false;
            bool Quiescent = true;

            void BeginShutdownCancellation() noexcept {
                CancellationStarted = true;
            }

            bool IsExecutionQuiescent() noexcept {
                return Quiescent;
            }

    };


    class ShutdownThreadResource final {

        public:

            bool StopRequested = false;
            bool Quiescent = true;

            ESPressio::Threading::ThreadStopRequestResult RequestStop() noexcept {
                StopRequested = true;
                return ESPressio::Threading::ThreadStopRequestResult::Accepted;
            }

            bool IsExecutionQuiescent() noexcept {
                return Quiescent;
            }

    };


    class LifecycleResource final {

        private:

            bool _failStart;

        public:

            std::size_t StartCount = 0U;
            std::size_t TerminationWakeCount = 0U;
            std::size_t JoinCount = 0U;
            std::size_t DestroyCount = 0U;

            explicit LifecycleResource(
                bool failStart = false
            ) noexcept :
                _failStart(failStart) {}

            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                ++StartCount;

                return _failStart
                    ? ESPressio::Platform::Execution::ExecutionStartResult::ProviderFailure
                    : ESPressio::Platform::Execution::ExecutionStartResult::Succeeded;
            }

            void RequestInfrastructureTermination() noexcept {
                ++TerminationWakeCount;
            }

            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                ++JoinCount;
                return ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded;
            }

            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                ++DestroyCount;
                return ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;
            }

    };


    struct DedicatedThreadIdentity final {};


    struct DedicatedCallable final {

        void operator ()(
            ESPressio::Threading::ThreadContext& context
        ) noexcept {
            static_cast<void>(
                context.IsStopRequested()
            );
        }

    };


    struct OrdinaryPool final {};


    struct TelemetryThread final {};


    struct QueueRecord final {

        /// Compact intrusive queue-link storage.
        ESPressio::Threading::Detail::SmallestIndex<3U>::Type QueueLink =
            ESPressio::Threading::Detail::SmallestIndex<3U>::Invalid;

        /// Stores the next queued record index.
        void SetQueueNext(
            ESPressio::Threading::Detail::SmallestIndex<3U>::Type recordIndex
        ) noexcept {
            QueueLink = recordIndex;
        }

        /// Returns the next queued record index.
        ESPressio::Threading::Detail::SmallestIndex<3U>::Type QueueNext() const noexcept {
            return QueueLink;
        }

    };


    struct ReturningCallable final {

        /// Produces one deterministic test result.
        int operator ()() const noexcept {
            return 42;
        }

    };


    struct CancellationAwareCallable final {

        /// Acknowledges cooperative cancellation when requested.
        ESPressio::Threading::TaskCompletion<int> operator ()(
            ESPressio::Threading::TaskContext& context
        ) const {
            if (context.IsCancellationRequested()) {
                return ESPressio::Threading::TaskCompletion<int>::Cancelled();
            }

            return ESPressio::Threading::TaskCompletion<int>::Completed(
                7
            );
        }

    };


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
        ESPressio::Threading::DedicatedWorkerLease<
            ReturningCallable,
            ESPressio::Threading::TaskRecordCapacity<3U>,
            ESPressio::Threading::CallableCapacity<32U>,
            ESPressio::Threading::ResultCapacity<16U>,
            ESPressio::Threading::StackCapacity<2048U>,
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::Critical>,
            ESPressio::Threading::AnyAffinity
        >,
        ESPressio::Threading::DedicatedThread<
            TelemetryThread,
            ESPressio::Threading::StackCapacity<4096U>,
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::High>,
            ESPressio::Threading::AnyAffinity
        >
    >;


    using TestTaskRecord = ESPressio::Threading::Detail::TaskRecord<
        32U,
        16U,
        8U,
        4U,
        AtomicByteProvider
    >;

    using TestFacilityCore = ESPressio::Threading::Detail::TaskFacilityCore<
        3U,
        32U,
        16U,
        4U,
        AtomicByteProvider
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
        Topology::HasDedicatedWorkerLease &&
        Topology::TaskFacilityCount == 1U &&
        Topology::DedicatedWorkerLeaseCount == 1U &&
        Topology::DedicatedThreadCount == 1U &&
        Topology::ManagedExecutionContextCount == 3U,
        "Topology must expose compile-time execution-resource counts without runtime registry state"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::TaskPoolRequirement<OrdinaryPool>
        >,
        "Topology must satisfy requirements for its configured Task Pool identity"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedWorkerRequirement<ReturningCallable>
        >,
        "Topology must satisfy requirements for its configured DedicatedWorkerLease identity"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedThreadRequirement<TelemetryThread>
        >,
        "Topology must satisfy requirements for its configured Dedicated Thread identity"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::TaskPoolRequirement<
                OrdinaryPool,
                ESPressio::Threading::MinimumStackCapacity<4096U>,
                ESPressio::Threading::MinimumWorkerConcurrency<1U>,
                ESPressio::Threading::MinimumPriority<ESPressio::Threading::ThreadPriority::Normal>,
                ESPressio::Threading::RequiredAffinity<ESPressio::Threading::AnyAffinity>
            >
        >,
        "Task Pool requirements must validate Worker stack/concurrency/priority/affinity"
    );

    static_assert(
        !ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::TaskPoolRequirement<
                OrdinaryPool,
                ESPressio::Threading::MinimumWorkerConcurrency<2U>
            >
        >,
        "Task Pool requirement must reject insufficient Worker concurrency"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedWorkerRequirement<
                ReturningCallable,
                ESPressio::Threading::MinimumStackCapacity<2048U>,
                ESPressio::Threading::MinimumWorkerConcurrency<1U>,
                ESPressio::Threading::MinimumPriority<ESPressio::Threading::ThreadPriority::Critical>,
                ESPressio::Threading::RequiredAffinity<ESPressio::Threading::AnyAffinity>
            >
        >,
        "DedicatedWorkerLease requirements must validate its fixed one-Worker entitlement"
    );

    static_assert(
        !ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedWorkerRequirement<
                ReturningCallable,
                ESPressio::Threading::MinimumWorkerConcurrency<2U>
            >
        >,
        "DedicatedWorkerLease must reject requirements above its fixed cardinality of one"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedThreadRequirement<
                TelemetryThread,
                ESPressio::Threading::MinimumStackCapacity<4096U>,
                ESPressio::Threading::MinimumPriority<ESPressio::Threading::ThreadPriority::High>,
                ESPressio::Threading::RequiredAffinity<ESPressio::Threading::AnyAffinity>
            >
        >,
        "Dedicated Thread requirements must validate stack/priority/affinity"
    );


    static_assert(
        sizeof(ESPressio::Threading::Detail::TaskControl<AtomicByteProvider>) == 1U,
        "Task intrinsic control must remain one byte"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::DedicatedThreadControl<AtomicByteProvider>) == 1U,
        "Dedicated Thread intrinsic control must remain one byte"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::AvailabilityBitmap<10U>) == 2U,
        "Ten Task availability bits must occupy exactly two bytes"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::IntrusiveTaskQueue<3U>) == 2U,
        "Three-record Task queue endpoints must occupy two one-byte indices"
    );

    static_assert(
        TestTaskRecord::PayloadCapacity == 32U,
        "Task record payload must use the larger callable/result capacity"
    );

    using ContextIndex = ESPressio::Threading::Detail::ExecutionContextIndexTraits<4U>::Type;

    using TaskRegistration = ESPressio::Threading::Detail::TaskWaitRegistration<
        ESPressio::Threading::Detail::SmallestIndex<8U>::Type,
        ContextIndex
    >;

    static_assert(
        sizeof(ContextIndex) == 1U,
        "Four managed execution contexts must use a one-byte wake-routing index"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::RegistrationSet<TaskRegistration, 4U>) ==
        sizeof(TaskRegistration) * 4U,
        "RegistrationSet must contain only target-owned registration records"
    );

    using TestFacilityRuntime = ESPressio::Threading::Detail::TaskFacilityRuntime<
        3U,
        32U,
        16U,
        1U,
        1U,
        ManagedContextRouter::ContextCapacity,
        AtomicByteProvider,
        MutexProvider,
        ManagedContextRouter
    >;


    using TestWorkerExecutionContext = ESPressio::Threading::Detail::TaskWorkerExecutionContext<
        ExecutionContextProvider,
        100U,
        TestFacilityRuntime,
        ManagedContextRouter
    >;


    static_assert(
        TestWorkerExecutionContext::ControlBackingBytes() == 24U,
        "Worker control backing must follow provider-declared physical storage"
    );

    static_assert(
        TestWorkerExecutionContext::StackBackingBytes() == 128U,
        "Worker stack backing must round semantic capacity to provider granularity"
    );


    using TestDedicatedWorkerLeaseRuntime = ESPressio::Threading::Detail::DedicatedWorkerLeaseRuntime<
        ReturningCallable,
        3U,
        32U,
        16U,
        100U,
        1U,
        ManagedContextRouter::ContextCapacity,
        AtomicByteProvider,
        MutexProvider,
        ExecutionContextProvider,
        ManagedContextRouter
    >;


    static_assert(
        TestDedicatedWorkerLeaseRuntime::WorkerCount == 1U,
        "DedicatedWorkerLease cardinality must remain exactly one Worker"
    );

    static_assert(
        TestDedicatedWorkerLeaseRuntime::RecordCapacity() == 3U,
        "DedicatedWorkerLease must retain its explicitly bounded isolated FIFO population"
    );


    using TestDedicatedThreadRuntime = ESPressio::Threading::Detail::DedicatedThreadRuntime<
        DedicatedThreadIdentity,
        DedicatedCallable,
        100U,
        ManagedContextRouter::ContextCapacity,
        AtomicByteProvider,
        MutexProvider,
        ExecutionContextProvider,
        ManagedContextRouter
    >;


    static_assert(
        TestDedicatedThreadRuntime::ControlBackingBytes() == 24U,
        "Dedicated Thread control backing must follow provider-declared physical storage"
    );

    static_assert(
        TestDedicatedThreadRuntime::StackBackingBytes() == 128U,
        "Dedicated Thread stack backing must round semantic capacity to provider granularity"
    );


    using TestWorkerScheduler = ESPressio::Threading::Detail::WorkerLeaseScheduler<
        3U,
        2U,
        8U
    >;

    static_assert(
        sizeof(TestWorkerScheduler) == 1U,
        "Three Worker availability bits must occupy one byte"
    );

} // Test


/// Exercises compact Threading foundation primitives.
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


    ESPressio::Threading::Detail::AvailabilityBitmap<10U> availability;
    std::size_t claimedIndex = 0U;

    for (std::size_t expectedIndex = 0U; expectedIndex < 10U; ++expectedIndex) {
        assert(
            availability.TryClaim(
                claimedIndex
            )
        );

        assert(
            claimedIndex == expectedIndex
        );
    }

    assert(
        !availability.TryClaim(
            claimedIndex
        )
    );

    availability.Release(
        4U
    );

    assert(
        availability.IsAvailable(
            4U
        )
    );

    assert(
        availability.TryClaim(
            claimedIndex
        )
    );

    assert(
        claimedIndex == 4U
    );


    Test::QueueRecord records[3U];
    ESPressio::Threading::Detail::IntrusiveTaskQueue<3U> queue;

    queue.Push(
        records,
        0U
    );

    queue.Push(
        records,
        1U
    );

    queue.Push(
        records,
        2U
    );

    assert(
        queue.Head() == 0U
    );

    assert(
        queue.Remove(
            records,
            1U
        )
    );

    assert(
        queue.Pop(
            records
        ) == 0U
    );

    assert(
        queue.Pop(
            records
        ) == 2U
    );

    assert(
        queue.Pop(
            records
        ) ==
        ESPressio::Threading::Detail::IntrusiveTaskQueue<3U>::InvalidIndex
    );


    ESPressio::Threading::Detail::DedicatedThreadControl<Test::AtomicByteProvider> threadControl;
    bool activationPhase = false;

    assert(
        threadControl.TryStart(
            activationPhase
        )
    );

    assert(
        threadControl.State() ==
        ESPressio::Threading::Detail::DedicatedThreadOperationalState::Running
    );

    assert(
        threadControl.TryRequestStop()
    );

    assert(
        threadControl.IsStopRequested()
    );

    assert(
        threadControl.TryPublishStopped(
            activationPhase
        )
    );

    assert(
        threadControl.State() ==
        ESPressio::Threading::Detail::DedicatedThreadOperationalState::Stopped
    );

    const auto firstActivationPhase = activationPhase;

    assert(
        threadControl.TryStart(
            activationPhase
        )
    );

    assert(
        activationPhase != firstActivationPhase
    );


    Test::TestTaskRecord completedRecord;
    completedRecord.Control.InitializeQueued();
    completedRecord.Control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::Running
    );

    using ReturningAdapter = ESPressio::Threading::Detail::TaskPayloadAdapter<
        Test::TestTaskRecord,
        Test::ReturningCallable,
        int
    >;

    new (completedRecord.Payload) Test::ReturningCallable();
    completedRecord.PayloadOperations = &ReturningAdapter::Operations;

    assert(
        completedRecord.PayloadOperations->Invoke(
            completedRecord
        ) == ESPressio::Threading::Detail::TaskInvocationOutcome::Completed
    );

    assert(
        completedRecord.Control.State() ==
        ESPressio::Threading::Detail::TaskOperationalState::Running
    );

    completedRecord.Control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::Completed
    );

    alignas(int) std::byte resultStorage[sizeof(int)];

    completedRecord.PayloadOperations->MoveResult(
        completedRecord,
        resultStorage
    );

    assert(
        *reinterpret_cast<int*>(resultStorage) == 42
    );


    Test::TestTaskRecord cancelledRecord;
    cancelledRecord.Control.InitializeQueued();
    cancelledRecord.Control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::RunningCancelRequested
    );

    using CancellationAdapter = ESPressio::Threading::Detail::TaskPayloadAdapter<
        Test::TestTaskRecord,
        Test::CancellationAwareCallable,
        int
    >;

    new (cancelledRecord.Payload) Test::CancellationAwareCallable();
    cancelledRecord.PayloadOperations = &CancellationAdapter::Operations;

    assert(
        cancelledRecord.PayloadOperations->Invoke(
            cancelledRecord
        ) == ESPressio::Threading::Detail::TaskInvocationOutcome::Cancelled
    );

    assert(
        cancelledRecord.Control.State() ==
        ESPressio::Threading::Detail::TaskOperationalState::RunningCancelRequested
    );

    cancelledRecord.Control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::Cancelled
    );


    Test::TestFacilityCore facility;

    auto completedAdmission = facility.Admit(
        Test::ReturningCallable{}
    );

    assert(
        completedAdmission.IsAdmitted()
    );

    assert(
        facility.RecordsInUse() == 1U
    );

    assert(
        facility.QueuedTasks() == 1U
    );

    assert(
        facility.PublicState(
            completedAdmission.Binding()->RecordIndex,
            completedAdmission.Binding()->Phase
        ) == ESPressio::Threading::TaskState::Queued
    );

    const auto completedClaim = facility.ClaimNextForWorker(
        0U
    );

    assert(
        completedClaim.IsClaimed()
    );

    assert(
        completedClaim.Binding()->RecordIndex == completedAdmission.Binding()->RecordIndex
    );

    assert(
        facility.PublicState(
            completedClaim.Binding()->RecordIndex,
            completedClaim.Binding()->Phase
        ) == ESPressio::Threading::TaskState::Running
    );

    const auto completedOutcome = facility.Invoke(
        completedClaim.Binding()->RecordIndex,
        completedClaim.Binding()->Phase
    );

    assert(
        completedOutcome == ESPressio::Threading::Detail::TaskInvocationOutcome::Completed
    );

    facility.PublishInvocationOutcome(
        completedClaim.Binding()->RecordIndex,
        completedClaim.Binding()->Phase,
        completedOutcome
    );

    assert(
        facility.PublicState(
            completedClaim.Binding()->RecordIndex,
            completedClaim.Binding()->Phase
        ) == ESPressio::Threading::TaskState::Completed
    );

    alignas(int) std::byte facilityResultStorage[sizeof(int)];

    assert(
        facility.TakeResult(
            completedClaim.Binding()->RecordIndex,
            completedClaim.Binding()->Phase,
            facilityResultStorage
        ) == ESPressio::Threading::TaskTakeStatus::Succeeded
    );

    assert(
        *reinterpret_cast<int*>(facilityResultStorage) == 42
    );

    assert(
        facility.Reclaim(
            completedClaim.Binding()->RecordIndex,
            completedClaim.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::Reclaimed
    );

    assert(
        facility.RecordsInUse() == 0U
    );


    auto queuedCancellationAdmission = facility.Admit(
        Test::ReturningCallable{}
    );

    const auto queuedCancellation = facility.Cancel(
        queuedCancellationAdmission.Binding()->RecordIndex,
        queuedCancellationAdmission.Binding()->Phase
    );

    assert(
        queuedCancellation.Result() == ESPressio::Threading::TaskCancelResult::Accepted
    );

    assert(
        queuedCancellation.IsTerminalPublished()
    );

    assert(
        facility.QueuedTasks() == 0U
    );

    assert(
        facility.Reclaim(
            queuedCancellationAdmission.Binding()->RecordIndex,
            queuedCancellationAdmission.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::NotEligible
    );

    assert(
        facility.ReleaseOwner(
            queuedCancellationAdmission.Binding()->RecordIndex,
            queuedCancellationAdmission.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReleaseEffect::OwnershipReleased
    );

    assert(
        facility.Reclaim(
            queuedCancellationAdmission.Binding()->RecordIndex,
            queuedCancellationAdmission.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::Reclaimed
    );


    auto cooperativeAdmission = facility.Admit(
        Test::CancellationAwareCallable{}
    );

    const auto cooperativeClaim = facility.ClaimNextForWorker(
        1U
    );

    assert(
        cooperativeClaim.IsClaimed()
    );

    const auto runningCancellation = facility.Cancel(
        cooperativeClaim.Binding()->RecordIndex,
        cooperativeClaim.Binding()->Phase
    );

    assert(
        runningCancellation.Result() == ESPressio::Threading::TaskCancelResult::Accepted
    );

    assert(
        !runningCancellation.IsTerminalPublished()
    );

    const auto cooperativeOutcome = facility.Invoke(
        cooperativeClaim.Binding()->RecordIndex,
        cooperativeClaim.Binding()->Phase
    );

    assert(
        cooperativeOutcome == ESPressio::Threading::Detail::TaskInvocationOutcome::Cancelled
    );

    facility.PublishInvocationOutcome(
        cooperativeClaim.Binding()->RecordIndex,
        cooperativeClaim.Binding()->Phase,
        cooperativeOutcome
    );

    assert(
        facility.PublicState(
            cooperativeClaim.Binding()->RecordIndex,
            cooperativeClaim.Binding()->Phase
        ) == ESPressio::Threading::TaskState::Cancelled
    );

    facility.ReleaseOwner(
        cooperativeClaim.Binding()->RecordIndex,
        cooperativeClaim.Binding()->Phase
    );

    assert(
        facility.Reclaim(
            cooperativeClaim.Binding()->RecordIndex,
            cooperativeClaim.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::Reclaimed
    );


    auto abandonedRunningAdmission = facility.Admit(
        Test::ReturningCallable{}
    );

    const auto abandonedRunningClaim = facility.ClaimNextForWorker(
        2U
    );

    assert(
        abandonedRunningClaim.IsClaimed()
    );

    assert(
        facility.ReleaseOwner(
            abandonedRunningClaim.Binding()->RecordIndex,
            abandonedRunningClaim.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReleaseEffect::OwnershipReleased
    );

    const auto abandonedRunningOutcome = facility.Invoke(
        abandonedRunningClaim.Binding()->RecordIndex,
        abandonedRunningClaim.Binding()->Phase
    );

    facility.PublishInvocationOutcome(
        abandonedRunningClaim.Binding()->RecordIndex,
        abandonedRunningClaim.Binding()->Phase,
        abandonedRunningOutcome
    );

    assert(
        facility.Reclaim(
            abandonedRunningClaim.Binding()->RecordIndex,
            abandonedRunningClaim.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::Reclaimed
    );


    Test::TestWorkerScheduler workerScheduler;

    assert(
        workerScheduler.AvailableCount() == 0U
    );

    assert(
        workerScheduler.MarkAvailable(
            2U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::Available
    );

    assert(
        workerScheduler.MarkAvailable(
            4U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::Available
    );

    assert(
        workerScheduler.MarkAvailable(
            1U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::OutsideFacilityRange
    );

    assert(
        workerScheduler.AvailableCount() == 2U
    );

    const auto firstWorkerLease = workerScheduler.TryClaimAvailable();

    assert(
        firstWorkerLease.has_value()
    );

    assert(
        firstWorkerLease.value() == 2U
    );

    const auto secondWorkerLease = workerScheduler.TryClaimAvailable();

    assert(
        secondWorkerLease.has_value()
    );

    assert(
        secondWorkerLease.value() == 4U
    );

    assert(
        !workerScheduler.TryClaimAvailable().has_value()
    );


    using RegistrationSet = ESPressio::Threading::Detail::RegistrationSet<
        Test::TaskRegistration,
        2U
    >;

    RegistrationSet registrations;
    std::size_t registrationIndex = 0U;

    Test::TaskRegistration firstRegistration;
    firstRegistration.RecordIndex = 3U;
    firstRegistration.Phase = true;
    firstRegistration.WaitingContextIndex = 1U;

    assert(
        registrations.Register(
            firstRegistration,
            registrationIndex
        ) == ESPressio::Threading::Detail::WaitRegistrationStatus::Registered
    );

    assert(
        registrationIndex == 0U
    );

    Test::TaskRegistration secondRegistration;
    secondRegistration.RecordIndex = 3U;
    secondRegistration.Phase = true;
    secondRegistration.WaitingContextIndex = 2U;

    assert(
        registrations.Register(
            secondRegistration,
            registrationIndex
        ) == ESPressio::Threading::Detail::WaitRegistrationStatus::Registered
    );

    assert(
        registrationIndex == 1U
    );

    assert(
        registrations.ActiveCount() == 2U
    );

    Test::TaskRegistration thirdRegistration;
    thirdRegistration.RecordIndex = 4U;
    thirdRegistration.WaitingContextIndex = 3U;

    assert(
        registrations.Register(
            thirdRegistration,
            registrationIndex
        ) == ESPressio::Threading::Detail::WaitRegistrationStatus::CapacityUnavailable
    );

    std::size_t matchingRegistrationCount = 0U;

    registrations.VisitActive(
        [&matchingRegistrationCount](
            const Test::TaskRegistration& registration
        ) {
            if (
                registration.RecordIndex == 3U &&
                registration.Phase
            ) {
                ++matchingRegistrationCount;
            }
        }
    );

    assert(
        matchingRegistrationCount == 2U
    );

    registrations.Unregister(
        0U
    );

    assert(
        registrations.ActiveCount() == 1U
    );

    assert(
        registrations.Register(
            thirdRegistration,
            registrationIndex
        ) == ESPressio::Threading::Detail::WaitRegistrationStatus::Registered
    );

    assert(
        registrationIndex == 0U
    );



    Test::ManagedContextRouter runtimeRouter;
    Test::TestFacilityRuntime runtime(
        runtimeRouter
    );

    assert(
        runtime.ValidateSynchronization() ==
        ESPressio::Threading::Detail::TaskFacilitySynchronizationResult::Ready
    );

    const auto unavailableDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        unavailableDispatch.Status() ==
        ESPressio::Threading::TaskDispatchStatus::Unavailable
    );

    assert(
        runtime.WorkerBecameAvailable(
            1U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::Available
    );

    auto immediateDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        immediateDispatch.IsSucceeded()
    );

    auto immediateTask = immediateDispatch.TakeTask();

    assert(
        immediateTask.State() == ESPressio::Threading::TaskState::Running
    );

    const auto immediateBinding = runtime.AssignedTaskForContext(
        1U
    );

    assert(
        immediateBinding.has_value()
    );

    const auto immediateOutcome = runtime.Invoke(
        immediateBinding->RecordIndex,
        immediateBinding->Phase
    );

    runtime.CompleteWorkerTask(
        1U,
        immediateBinding->RecordIndex,
        immediateBinding->Phase,
        immediateOutcome
    );

    assert(
        immediateTask.State() == ESPressio::Threading::TaskState::Completed
    );

    {
        auto result = immediateTask.TakeResult();

        assert(
            result.IsSucceeded()
        );
    }

    assert(
        runtime.WorkerBecameAvailable(
            1U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::Available
    );

    auto firstQueuedDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::Queue
    );

    auto secondQueuedDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::Queue
    );

    assert(
        firstQueuedDispatch.IsSucceeded() &&
        secondQueuedDispatch.IsSucceeded()
    );

    auto firstQueuedTask = firstQueuedDispatch.TakeTask();
    auto secondQueuedTask = secondQueuedDispatch.TakeTask();

    assert(
        firstQueuedTask.State() == ESPressio::Threading::TaskState::Running
    );

    assert(
        secondQueuedTask.State() == ESPressio::Threading::TaskState::Queued
    );

    const auto firstQueuedBinding = runtime.AssignedTaskForContext(
        1U
    );

    assert(
        firstQueuedBinding.has_value()
    );

    const auto firstQueuedOutcome = runtime.Invoke(
        firstQueuedBinding->RecordIndex,
        firstQueuedBinding->Phase
    );

    runtime.CompleteWorkerTask(
        1U,
        firstQueuedBinding->RecordIndex,
        firstQueuedBinding->Phase,
        firstQueuedOutcome
    );

    assert(
        secondQueuedTask.State() == ESPressio::Threading::TaskState::Running
    );

    const auto secondQueuedBinding = runtime.AssignedTaskForContext(
        1U
    );

    assert(
        secondQueuedBinding.has_value()
    );

    const auto secondQueuedOutcome = runtime.Invoke(
        secondQueuedBinding->RecordIndex,
        secondQueuedBinding->Phase
    );

    runtime.CompleteWorkerTask(
        1U,
        secondQueuedBinding->RecordIndex,
        secondQueuedBinding->Phase,
        secondQueuedOutcome
    );

    {
        auto result = firstQueuedTask.TakeResult();

        assert(
            result.IsSucceeded()
        );
    }

    {
        auto result = secondQueuedTask.TakeResult();

        assert(
            result.IsSucceeded()
        );
    }

    Test::LifetimeCallable::DestructionCount = 0U;
    Test::LifetimeResult::DestructionCount = 0U;

    {
        auto lifetimeDispatch = runtime.Dispatch(
            Test::LifetimeCallable{},
            ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
        );

        assert(
            lifetimeDispatch.IsSucceeded()
        );

        auto lifetimeTask = lifetimeDispatch.TakeTask();
        const auto lifetimeBinding = runtime.AssignedTaskForContext(
            1U
        );

        assert(
            lifetimeBinding.has_value()
        );

        const auto lifetimeOutcome = runtime.Invoke(
            lifetimeBinding->RecordIndex,
            lifetimeBinding->Phase
        );

        assert(
            Test::LifetimeCallable::DestructionCount == 1U
        );

        runtime.CompleteWorkerTask(
            1U,
            lifetimeBinding->RecordIndex,
            lifetimeBinding->Phase,
            lifetimeOutcome
        );

        assert(
            Test::LifetimeResult::DestructionCount == 0U
        );
    }

    assert(
        Test::LifetimeResult::DestructionCount == 1U
    );



    auto occupyingDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        occupyingDispatch.IsSucceeded()
    );

    auto occupyingTask = occupyingDispatch.TakeTask();

    Test::LifetimeCallable::DestructionCount = 0U;

    auto cancelledQueuedDispatch = runtime.Dispatch(
        Test::LifetimeCallable{},
        ESPressio::Threading::TaskDispatchPolicy::Queue
    );

    assert(
        cancelledQueuedDispatch.IsSucceeded()
    );

    auto cancelledQueuedTask = cancelledQueuedDispatch.TakeTask();

    assert(
        cancelledQueuedTask.State() == ESPressio::Threading::TaskState::Queued
    );

    assert(
        cancelledQueuedTask.Cancel() ==
        ESPressio::Threading::TaskCancelResult::Accepted
    );

    assert(
        cancelledQueuedTask.State() == ESPressio::Threading::TaskState::Cancelled
    );

    assert(
        Test::LifetimeCallable::DestructionCount == 1U
    );

    const auto occupyingBinding = runtime.AssignedTaskForContext(
        1U
    );

    assert(
        occupyingBinding.has_value()
    );

    const auto occupyingOutcome = runtime.Invoke(
        occupyingBinding->RecordIndex,
        occupyingBinding->Phase
    );

    runtime.CompleteWorkerTask(
        1U,
        occupyingBinding->RecordIndex,
        occupyingBinding->Phase,
        occupyingOutcome
    );

    {
        auto result = occupyingTask.TakeResult();

        assert(
            result.IsSucceeded()
        );
    }

    ESPressio::Threading::Detail::InfrastructureLifecycle<
        Test::AtomicByteProvider
    > lifecycle;

    assert(
        lifecycle.CommitInitialization() ==
        ESPressio::Threading::ThreadingInitializationResult::Succeeded
    );

    Test::LifecycleResource firstLifecycleResource;
    Test::LifecycleResource failingLifecycleResource(true);
    Test::LifecycleResource laterLifecycleResource;

    assert(
        lifecycle.Start(
            firstLifecycleResource,
            failingLifecycleResource,
            laterLifecycleResource
        ) == ESPressio::Threading::ThreadingStartResult::ProviderFailure
    );

    assert(
        firstLifecycleResource.StartCount == 1U &&
        firstLifecycleResource.TerminationWakeCount == 1U &&
        firstLifecycleResource.JoinCount == 1U &&
        firstLifecycleResource.DestroyCount == 1U
    );

    assert(
        failingLifecycleResource.StartCount == 1U &&
        failingLifecycleResource.DestroyCount == 1U
    );

    assert(
        laterLifecycleResource.StartCount == 0U &&
        laterLifecycleResource.DestroyCount == 1U
    );

    assert(
        lifecycle.ShouldTerminate()
    );

    ESPressio::Threading::Detail::InfrastructureLifecycle<
        Test::AtomicByteProvider
    > successfulLifecycle;

    assert(
        successfulLifecycle.CommitInitialization() ==
        ESPressio::Threading::ThreadingInitializationResult::Succeeded
    );

    Test::LifecycleResource successfulResource;

    assert(
        successfulLifecycle.Start(
            successfulResource
        ) == ESPressio::Threading::ThreadingStartResult::Succeeded
    );

    assert(
        successfulLifecycle.CanActivate()
    );

    assert(
        successfulLifecycle.BeginShutdown() ==
        ESPressio::Threading::ThreadingShutdownResult::Accepted
    );

    assert(
        successfulLifecycle.ShouldTerminate()
    );

    ESPressio::Threading::Detail::ShutdownWaitRuntime<
        decltype(successfulLifecycle),
        Test::ManagedContextRouter::ContextCapacity,
        Test::MutexProvider,
        Test::ManagedContextRouter
    > shutdownWaitRuntime(
        successfulLifecycle,
        runtimeRouter
    );

    successfulLifecycle.FinalizeShutdown(
        shutdownWaitRuntime,
        successfulResource
    );

    assert(
        successfulResource.TerminationWakeCount == 1U &&
        successfulResource.JoinCount == 1U &&
        successfulResource.DestroyCount == 1U
    );

    assert(
        shutdownWaitRuntime.WaitFor(
            ESPressio::Clock::Duration::FromNanoseconds(
                0
            )
        ) == ESPressio::Threading::ShutdownWaitResult::Completed
    );

    assert(
        successfulLifecycle.BeginShutdown() ==
        ESPressio::Threading::ThreadingShutdownResult::AlreadyCompleted
    );


    ESPressio::Threading::Detail::InfrastructureLifecycle<
        Test::AtomicByteProvider
    > coordinatedLifecycle;

    assert(
        coordinatedLifecycle.CommitInitialization() ==
        ESPressio::Threading::ThreadingInitializationResult::Succeeded
    );

    Test::LifecycleResource coordinatedInfrastructureResource;

    assert(
        coordinatedLifecycle.Start(
            coordinatedInfrastructureResource
        ) == ESPressio::Threading::ThreadingStartResult::Succeeded
    );

    Test::ShutdownTaskResource shutdownTaskResource;
    Test::ShutdownThreadResource shutdownThreadResource;

    auto shutdownTaskResources = std::tie(
        shutdownTaskResource
    );

    auto shutdownThreadResources = std::tie(
        shutdownThreadResource
    );

    assert(
        ESPressio::Threading::Detail::ShutdownCoordinator::Begin(
            coordinatedLifecycle,
            shutdownTaskResources,
            shutdownThreadResources
        ) == ESPressio::Threading::ThreadingShutdownResult::Accepted
    );

    assert(
        shutdownTaskResource.CancellationStarted &&
        shutdownThreadResource.StopRequested
    );

    assert(
        ESPressio::Threading::Detail::ShutdownCoordinator::IsExecutionQuiescent(
            shutdownTaskResources,
            shutdownThreadResources
        )
    );


    bool shutdownRequested = false;

    const auto shutdownPredicate = [](
        const void* context
    ) noexcept {
        return *static_cast<const bool*>(
            context
        );
    };

    Test::TestWorkerExecutionContext workerExecutionContext(
        runtime,
        runtimeRouter,
        1U,
        &shutdownRequested,
        shutdownPredicate
    );

    assert(
        workerExecutionContext.Initialize(
            ESPressio::Platform::Execution::ExecutionPriority::Normal,
            ESPressio::Platform::Execution::ProcessorAffinity::Any(),
            "test-worker"
        ) == ESPressio::Threading::Detail::WorkerExecutionInitializationResult::Succeeded
    );

    assert(
        workerExecutionContext.Start() ==
        ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
    );

    assert(
        workerExecutionContext.Join(
            ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
        ) == ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded
    );

    assert(
        workerExecutionContext.Destroy() ==
        ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded
    );

    return 0;
}
