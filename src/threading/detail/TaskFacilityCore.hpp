#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <optional>
#include <type_traits>
#include <utility>

#include "../ThreadingTypes.hpp"
#include "CallableTraits.hpp"
#include "FacilityStorage.hpp"
#include "TaskPayloadAdapter.hpp"
#include "TaskRecord.hpp"

namespace ESPressio::Threading::Detail {

    enum class TaskAdmissionCoreStatus : std::uint8_t {
        Admitted = 0,
        CapacityUnavailable = 1
    };


    enum class TaskWorkerClaimStatus : std::uint8_t {
        Claimed = 0,
        QueueEmpty = 1
    };


    enum class TaskReleaseEffect : std::uint8_t {
        OwnershipReleased = 0,
        TerminalPublished = 1
    };


    enum class TaskReclaimResult : std::uint8_t {
        Reclaimed = 0,
        NotEligible = 1
    };


    template<class TIndex>
    struct TaskRecordBinding final {

        /// Bound Task record index.
        TIndex RecordIndex;

        /// Bound record incarnation Phase.
        bool Phase;

    };


    template<class TIndex>
    class TaskAdmissionCoreResult final {

        private:

            // Admission outcome.

            /// Operational admission status.
            TaskAdmissionCoreStatus _status;

            /// Admitted record binding, present only after successful admission.
            std::optional<TaskRecordBinding<TIndex>> _binding;

            // Internal construction.

            /// Creates one normalized admission outcome.
            TaskAdmissionCoreResult(
                TaskAdmissionCoreStatus status,
                std::optional<TaskRecordBinding<TIndex>> binding
            ) noexcept :
                _status(status),
                _binding(std::move(binding)) {}

        public:

            // Construction.

            /// Creates a capacity-unavailable admission outcome.
            static TaskAdmissionCoreResult CapacityUnavailable() noexcept {
                return TaskAdmissionCoreResult(
                    TaskAdmissionCoreStatus::CapacityUnavailable,
                    std::nullopt
                );
            }

            /// Creates a successful admission outcome.
            static TaskAdmissionCoreResult Admitted(
                TIndex recordIndex,
                bool phase
            ) noexcept {
                return TaskAdmissionCoreResult(
                    TaskAdmissionCoreStatus::Admitted,
                    TaskRecordBinding<TIndex>{
                        recordIndex,
                        phase
                    }
                );
            }


            // Inspection.

            /// Returns the operational admission status.
            TaskAdmissionCoreStatus Status() const noexcept {
                return _status;
            }

            /// Indicates whether a record was admitted.
            bool IsAdmitted() const noexcept {
                return _status == TaskAdmissionCoreStatus::Admitted &&
                    _binding.has_value();
            }

            /// Returns the admitted record binding when admission succeeded.
            const std::optional<TaskRecordBinding<TIndex>>& Binding() const noexcept {
                return _binding;
            }

    };


    template<class TIndex>
    class TaskWorkerClaimResult final {

        private:

            // Worker-claim outcome.

            /// Operational Worker claim status.
            TaskWorkerClaimStatus _status;

            /// Claimed record binding, present only after successful claim.
            std::optional<TaskRecordBinding<TIndex>> _binding;

            // Internal construction.

            /// Creates one normalized Worker claim outcome.
            TaskWorkerClaimResult(
                TaskWorkerClaimStatus status,
                std::optional<TaskRecordBinding<TIndex>> binding
            ) noexcept :
                _status(status),
                _binding(std::move(binding)) {}

        public:

            // Construction.

            /// Creates an empty-queue Worker claim outcome.
            static TaskWorkerClaimResult QueueEmpty() noexcept {
                return TaskWorkerClaimResult(
                    TaskWorkerClaimStatus::QueueEmpty,
                    std::nullopt
                );
            }

            /// Creates a successful Worker claim outcome.
            static TaskWorkerClaimResult Claimed(
                TIndex recordIndex,
                bool phase
            ) noexcept {
                return TaskWorkerClaimResult(
                    TaskWorkerClaimStatus::Claimed,
                    TaskRecordBinding<TIndex>{
                        recordIndex,
                        phase
                    }
                );
            }


            // Inspection.

            /// Returns the Worker claim status.
            TaskWorkerClaimStatus Status() const noexcept {
                return _status;
            }

            /// Indicates whether one queued record was claimed for execution.
            bool IsClaimed() const noexcept {
                return _status == TaskWorkerClaimStatus::Claimed &&
                    _binding.has_value();
            }

            /// Returns the claimed record binding when Worker claim succeeded.
            const std::optional<TaskRecordBinding<TIndex>>& Binding() const noexcept {
                return _binding;
            }

    };


    class TaskCancellationCoreResult final {

        private:

            // Cancellation outcome.

            /// Public cancellation result returned to the Task owner.
            TaskCancelResult _result;

            /// Indicates whether this operation newly published terminal Cancelled state.
            bool _terminalPublished;

        public:

            // Construction.

            /// Creates one cancellation outcome.
            TaskCancellationCoreResult(
                TaskCancelResult result,
                bool terminalPublished
            ) noexcept :
                _result(result),
                _terminalPublished(terminalPublished) {}


            // Inspection.

            /// Returns the public Task cancellation result.
            TaskCancelResult Result() const noexcept {
                return _result;
            }

            /// Indicates whether this operation newly published terminal state.
            bool IsTerminalPublished() const noexcept {
                return _terminalPublished;
            }

    };


    template<std::size_t TRecordCapacity, std::size_t TCallableCapacity, std::size_t TResultCapacity, class TAtomicWord8Provider>
    class TaskFacilityCore final {

        static_assert(
            TRecordCapacity > 0U,
            "TaskFacilityCore requires positive record capacity"
        );

        static_assert(
            TCallableCapacity > 0U,
            "TaskFacilityCore requires positive callable capacity"
        );

        private:

            // Internal Types.

            /// Concrete bounded Task record owned by this facility.
            using RecordType = TaskRecord<
                TCallableCapacity,
                TResultCapacity,
                TRecordCapacity,
                TAtomicWord8Provider
            >;

            /// Smallest record-index Type satisfying the configured facility capacity.
            using IndexType = typename RecordType::Index;


            // Bounded facility storage.

            /// Statically provisioned Task records.
            RecordType _records[TRecordCapacity];

            /// Structural free-record publication bitmap.
            AvailabilityBitmap<TRecordCapacity> _availability;

            /// Intrusive FIFO over Queued record indices.
            IntrusiveTaskQueue<TRecordCapacity> _queue;


            // Internal validation.

            /// Indicates whether the supplied index and Phase identify the same record incarnation.
            ///
            /// This check intentionally reads only atomic record control. Worker execution and
            /// public handle observation may occur outside the facility lock, while the byte-level
            /// availability bitmap is mutated under that lock and therefore must not be read here.
            bool IsCurrentIncarnation(
                IndexType recordIndex,
                bool phase
            ) const noexcept {
                return recordIndex < static_cast<IndexType>(TRecordCapacity) &&
                    _records[recordIndex].Control.Phase() == phase;
            }

        public:

            // Core Types.

            /// Concrete bounded Task record owned by this facility.
            using Record = RecordType;

            /// Smallest record-index Type satisfying the configured facility capacity.
            using Index = IndexType;

            /// Structured admission outcome for this facility.
            using AdmissionResult = TaskAdmissionCoreResult<Index>;

            /// Structured Worker-claim outcome for this facility.
            using WorkerClaimResult = TaskWorkerClaimResult<Index>;


            // Admission.

            /// Moves one callable into a newly admitted bounded Task record and appends it to FIFO order.
            ///
            /// The owning facility runtime must serialize this operation with queue mutation,
            /// cancellation, Worker claim, owner release and reclamation.
            template<class TCallable>
            AdmissionResult Admit(
                TCallable&& callable
            ) noexcept {
                using StoredCallable = std::decay_t<TCallable>;
                using Result = CallableResultT<StoredCallable>;

                static_assert(
                    std::is_move_constructible_v<StoredCallable>,
                    "Task callable must be move constructible"
                );

                static_assert(
                    std::is_nothrow_constructible_v<StoredCallable, TCallable&&>,
                    "Task admission requires no-throw construction into deterministic Task storage"
                );

                static_assert(
                    !std::is_reference_v<Result>,
                    "Task results must not be reference Types"
                );

                static_assert(
                    sizeof(StoredCallable) <= TCallableCapacity,
                    "Task callable exceeds the configured facility callable capacity"
                );

                static_assert(
                    alignof(StoredCallable) <= alignof(std::max_align_t),
                    "Over-aligned Task callable Types are not supported by the v1 bounded payload"
                );

                if constexpr (!std::is_void_v<Result>) {
                    static_assert(
                        TResultCapacity > 0U,
                        "Non-void Tasks require positive result capacity"
                    );

                    static_assert(
                        sizeof(Result) <= TResultCapacity,
                        "Task result exceeds the configured facility result capacity"
                    );

                    static_assert(
                        std::is_nothrow_move_constructible_v<Result>,
                        "Task result transfer into deterministic storage must be no-throw"
                    );

                    static_assert(
                        alignof(Result) <= alignof(std::max_align_t),
                        "Over-aligned Task result Types are not supported by the v1 bounded payload"
                    );
                }

                std::size_t claimedIndex = 0U;

                if (!_availability.TryClaim(
                    claimedIndex
                )) {
                    return AdmissionResult::CapacityUnavailable();
                }

                const auto recordIndex = static_cast<Index>(
                    claimedIndex
                );
                auto& record = _records[recordIndex];

                record.Control.InitializeQueued();

                new (record.Payload) StoredCallable(
                    std::forward<TCallable>(
                        callable
                    )
                );

                using Adapter = TaskPayloadAdapter<
                    Record,
                    StoredCallable,
                    Result
                >;

                record.PayloadOperations = &Adapter::Operations;
                record.QueueNext = IntrusiveTaskQueue<TRecordCapacity>::InvalidIndex;

                _queue.Push(
                    _records,
                    recordIndex
                );

                return AdmissionResult::Admitted(
                    recordIndex,
                    record.Control.Phase()
                );
            }


            // Worker claim and execution.

            /// Claims the oldest queued Task for Worker execution.
            ///
            /// The owning facility runtime must serialize this operation with queue mutation.
            WorkerClaimResult ClaimNextForWorker() noexcept {
                const auto recordIndex = _queue.Pop(
                    _records
                );

                if (recordIndex == IntrusiveTaskQueue<TRecordCapacity>::InvalidIndex) {
                    return WorkerClaimResult::QueueEmpty();
                }

                auto& record = _records[recordIndex];

                record.Control.SetState(
                    TaskOperationalState::Running
                );

                return WorkerClaimResult::Claimed(
                    recordIndex,
                    record.Control.Phase()
                );
            }

            /// Executes one Worker-owned Task payload outside the facility serialization boundary.
            TaskInvocationOutcome Invoke(
                Index recordIndex,
                bool phase
            ) {
                auto& record = _records[recordIndex];

                if (!IsCurrentIncarnation(
                    recordIndex,
                    phase
                )) {
                    return TaskInvocationOutcome::Cancelled;
                }

                return record.PayloadOperations->Invoke(
                    record
                );
            }

            /// Publishes one Worker invocation outcome after result/callable lifetime transition is complete.
            ///
            /// The owning facility runtime must serialize this operation with Task control operations
            /// and must wake matching waiters after this terminal publication.
            void PublishInvocationOutcome(
                Index recordIndex,
                bool phase,
                TaskInvocationOutcome outcome
            ) noexcept {
                if (!IsCurrentIncarnation(
                    recordIndex,
                    phase
                )) {
                    return;
                }

                auto& record = _records[recordIndex];

                if (outcome == TaskInvocationOutcome::Completed) {
                    if (!record.Control.HasOwner()) {
                        record.PayloadOperations->DestroyResult(
                            record
                        );
                    }

                    record.Control.SetState(
                        TaskOperationalState::Completed
                    );
                    return;
                }

                record.Control.SetState(
                    TaskOperationalState::Cancelled
                );
            }


            // Task observation.

            /// Returns the public lifecycle state for one current Task incarnation.
            TaskState PublicState(
                Index recordIndex,
                bool phase
            ) const noexcept {
                if (!IsCurrentIncarnation(
                    recordIndex,
                    phase
                )) {
                    return TaskState::Cancelled;
                }

                switch (_records[recordIndex].Control.State()) {
                    case TaskOperationalState::Queued:
                        return TaskState::Queued;
                    case TaskOperationalState::Running:
                    case TaskOperationalState::RunningCancelRequested:
                        return TaskState::Running;
                    case TaskOperationalState::Completed:
                        return TaskState::Completed;
                    case TaskOperationalState::Cancelled:
                        return TaskState::Cancelled;
                }

                return TaskState::Cancelled;
            }

            /// Indicates whether the identified Task incarnation has reached terminal lifecycle state.
            bool IsTerminal(
                Index recordIndex,
                bool phase
            ) const noexcept {
                const auto state = PublicState(
                    recordIndex,
                    phase
                );

                return state == TaskState::Completed ||
                    state == TaskState::Cancelled;
            }


            // Task cancellation.

            /// Applies cooperative cancellation semantics to one Task owner request.
            ///
            /// The owning facility runtime must serialize this operation with Worker claim,
            /// terminal publication and queue mutation.
            TaskCancellationCoreResult Cancel(
                Index recordIndex,
                bool phase
            ) noexcept {
                if (!IsCurrentIncarnation(
                    recordIndex,
                    phase
                )) {
                    return TaskCancellationCoreResult(
                        TaskCancelResult::AlreadyFinished,
                        false
                    );
                }

                auto& record = _records[recordIndex];

                switch (record.Control.State()) {
                    case TaskOperationalState::Queued:
                        static_cast<void>(
                            _queue.Remove(
                                _records,
                                recordIndex
                            )
                        );

                        record.PayloadOperations->DestroyCallable(
                            record
                        );

                        record.Control.SetState(
                            TaskOperationalState::Cancelled
                        );

                        return TaskCancellationCoreResult(
                            TaskCancelResult::Accepted,
                            true
                        );

                    case TaskOperationalState::Running:
                        static_cast<void>(
                            record.Control.CompareExchangeState(
                                TaskOperationalState::Running,
                                TaskOperationalState::RunningCancelRequested
                            )
                        );

                        return TaskCancellationCoreResult(
                            TaskCancelResult::Accepted,
                            false
                        );

                    case TaskOperationalState::RunningCancelRequested:
                        return TaskCancellationCoreResult(
                            TaskCancelResult::Accepted,
                            false
                        );

                    case TaskOperationalState::Completed:
                    case TaskOperationalState::Cancelled:
                        return TaskCancellationCoreResult(
                            TaskCancelResult::AlreadyFinished,
                            false
                        );
                }

                return TaskCancellationCoreResult(
                    TaskCancelResult::AlreadyFinished,
                    false
                );
            }


            // Public ownership release.

            /// Releases the sole public Task ownership interest and performs any required payload cleanup.
            ///
            /// The owning facility runtime must serialize this operation. A TerminalPublished effect
            /// means queued abandonment published Cancelled and matching waiters must be woken.
            TaskReleaseEffect ReleaseOwner(
                Index recordIndex,
                bool phase
            ) noexcept {
                if (!IsCurrentIncarnation(
                    recordIndex,
                    phase
                )) {
                    return TaskReleaseEffect::OwnershipReleased;
                }

                auto& record = _records[recordIndex];

                switch (record.Control.State()) {
                    case TaskOperationalState::Queued:
                        static_cast<void>(
                            _queue.Remove(
                                _records,
                                recordIndex
                            )
                        );

                        record.PayloadOperations->DestroyCallable(
                            record
                        );

                        record.Control.SetState(
                            TaskOperationalState::Cancelled
                        );
                        record.Control.ReleaseOwner();
                        return TaskReleaseEffect::TerminalPublished;

                    case TaskOperationalState::Running:
                    case TaskOperationalState::RunningCancelRequested:
                        record.Control.ReleaseOwner();
                        return TaskReleaseEffect::OwnershipReleased;

                    case TaskOperationalState::Completed:
                        record.PayloadOperations->DestroyResult(
                            record
                        );
                        record.Control.ReleaseOwner();
                        return TaskReleaseEffect::OwnershipReleased;

                    case TaskOperationalState::Cancelled:
                        record.Control.ReleaseOwner();
                        return TaskReleaseEffect::OwnershipReleased;
                }

                return TaskReleaseEffect::OwnershipReleased;
            }


            // Result extraction.

            /// Moves a completed result to caller-provided typed storage and consumes public ownership.
            TaskTakeStatus TakeResult(
                Index recordIndex,
                bool phase,
                void* destination
            ) {
                if (!IsCurrentIncarnation(
                    recordIndex,
                    phase
                )) {
                    return TaskTakeStatus::NotCompleted;
                }

                auto& record = _records[recordIndex];

                switch (record.Control.State()) {
                    case TaskOperationalState::Completed:
                        record.PayloadOperations->MoveResult(
                            record,
                            destination
                        );
                        record.Control.ReleaseOwner();
                        return TaskTakeStatus::Succeeded;

                    case TaskOperationalState::Cancelled:
                        return TaskTakeStatus::Cancelled;

                    case TaskOperationalState::Queued:
                    case TaskOperationalState::Running:
                    case TaskOperationalState::RunningCancelRequested:
                        return TaskTakeStatus::NotCompleted;
                }

                return TaskTakeStatus::NotCompleted;
            }


            // Reclamation.

            /// Indicates whether terminal record state is free of the public ownership interest.
            ///
            /// The owning facility runtime must additionally prove that no waiter registration or
            /// in-flight Worker/internal interest remains before calling Reclaim.
            bool IsOwnerlessTerminal(
                Index recordIndex,
                bool phase
            ) const noexcept {
                if (!IsCurrentIncarnation(
                    recordIndex,
                    phase
                )) {
                    return false;
                }

                const auto& record = _records[recordIndex];
                const auto state = record.Control.State();

                return !record.Control.HasOwner() &&
                    (
                        state == TaskOperationalState::Completed ||
                        state == TaskOperationalState::Cancelled
                    );
            }

            /// Republishes one fully quiescent terminal record as structurally available.
            ///
            /// This method checks the intrinsic owner/terminal preconditions only. The owning
            /// facility runtime must prove absence of matching waiter/internal interests.
            TaskReclaimResult Reclaim(
                Index recordIndex,
                bool phase
            ) noexcept {
                if (
                    !IsOwnerlessTerminal(
                        recordIndex,
                        phase
                    ) ||
                    _availability.IsAvailable(recordIndex)
                ) {
                    return TaskReclaimResult::NotEligible;
                }

                auto& record = _records[recordIndex];
                record.PayloadOperations = nullptr;
                record.QueueNext = IntrusiveTaskQueue<TRecordCapacity>::InvalidIndex;

                _availability.Release(
                    recordIndex
                );

                return TaskReclaimResult::Reclaimed;
            }


            // Bounded observability.

            /// Returns the configured Task-record capacity.
            static constexpr std::size_t RecordCapacity() noexcept {
                return TRecordCapacity;
            }

            /// Returns the number of currently structurally allocated Task records.
            ///
            /// The owning facility runtime must serialize this scan with admission/reclamation.
            std::size_t RecordsInUse() const noexcept {
                std::size_t inUse = 0U;

                for (std::size_t index = 0U; index < TRecordCapacity; ++index) {
                    if (!_availability.IsAvailable(index)) {
                        ++inUse;
                    }
                }

                return inUse;
            }

            /// Returns the number of currently queued Tasks.
            ///
            /// The owning facility runtime must serialize this traversal with queue mutation.
            std::size_t QueuedTasks() const noexcept {
                std::size_t queued = 0U;
                auto current = _queue.Head();

                while (current != IntrusiveTaskQueue<TRecordCapacity>::InvalidIndex) {
                    ++queued;
                    current = _records[current].QueueNext;
                }

                return queued;
            }

    };

} // ESPressio::Threading::Detail
