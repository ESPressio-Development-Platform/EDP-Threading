#include <cassert>
#include <cstdint>
#include <type_traits>

#include <ESPressio_Threading.hpp>

#include "../src/threading/detail/DedicatedThreadControl.hpp"
#include "../src/threading/detail/FacilityStorage.hpp"
#include "../src/threading/detail/TaskFacilityCore.hpp"
#include "../src/threading/detail/TaskFacilityRuntime.hpp"
#include "../src/threading/detail/TaskPayloadAdapter.hpp"
#include "../src/threading/detail/TaskRecord.hpp"
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
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::AlreadyAvailable
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

    return 0;
}
