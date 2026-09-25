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
        NotEligible = 1,
        ProviderFailure = 2
    };


    /// Defines the compile-time contract for `TaskRecordBinding`.
    /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
    template<class TIndex>
    struct TaskRecordBinding final {

        /// Bound Task record index.
        TIndex RecordIndex;

        /// Bound record incarnation Phase.
        bool Phase;

    };


    /// Defines the compile-time contract for `TaskAdmissionCoreResult`.
    /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
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


    /// Defines the compile-time contract for `TaskWorkerClaimResult`.
    /// @tparam TIndex Compile-time resource or tuple index used by recursive traversal.
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


    /// Defines the compile-time contract for `TaskFacilityCore`.
    /// @tparam TRecordCapacity Task-record capacity declaration Type or bounded capacity.
    /// @tparam TCallableCapacity Callable-storage capacity declaration Type or byte capacity.
    /// @tparam TResultCapacity Result-storage capacity declaration Type or byte capacity.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<std::size_t TRecordCapacity, std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TExecutionContextCapacity>
    class TaskFacilityCore final {

        static_assert(
            TRecordCapacity > 0U,
            "TaskFacilityCore requires positive record capacity"
        );

        static_assert(
            TCallableCapacity > 0U,
            "TaskFacilityCore requires positive callable capacity"
        );

        static_assert(
            TExecutionContextCapacity > 0U,
            "TaskFacilityCore requires positive managed execution-context capacity"
        );

        private:

            // Internal Types.

            /// Concrete bounded Task record owned by this facility.
            using RecordType = TaskRecord<
                TCallableCapacity,
                TResultCapacity,
                TRecordCapacity,
                TExecutionContextCapacity
            >;

            /// Smallest record-index Type satisfying the configured facility capacity.
            using IndexType = typename RecordType::Index;

            /// Smallest managed execution-context index Type satisfying the topology capacity.
            using ExecutionContextIndexType = typename RecordType::ExecutionContextIndex;

            /// Shared bounded availability-set Type for this facility's Task records.
            using AvailabilityType = TaskRecordAvailabilitySet<TRecordCapacity>;

            /// Shared intrusive FIFO Type for this facility's queued Task records.
            using QueueType = TaskRecordQueue<TRecordCapacity>;

            /// Strong Task-record identity used by the shared availability and queue topology.
            using TopologyIndexType = typename QueueType::Index;


            // Bounded facility storage.

            /// Statically provisioned Task records.
            RecordType _records[TRecordCapacity];

            /// Structural free-record publication set; one bit per Task record.
            AvailabilityType _availability;

            /// Intrusive FIFO over Queued record identities.
            QueueType _queue;


            // Index adaptation.

            /// Converts one already range-valid raw Threading record index into its strong topology identity.
            static TopologyIndexType ToTopologyIndex(
                IndexType recordIndex
            ) noexcept {
                return TopologyIndexType::FromUnchecked(
                    static_cast<std::size_t>(
                        recordIndex
                    )
                );
            }


            // Internal validation.

            /// Indicates whether the supplied index and Phase identify the same record incarnation.
            ///
            /// The owning facility runtime must serialize this check with every control mutation.
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

            /// Dense managed execution-context index Type used for targeted wake routing.
            using ExecutionContextIndex = ExecutionContextIndexType;


            // Construction.

            /// Creates an empty Task facility with every statically provisioned Task record available.
            TaskFacilityCore() noexcept {
                _availability.SetAll();
            }


            // Admission.

            /// Indicates whether structural Task-record capacity is currently available.
            ///
            /// The owning facility runtime must serialize this query with admission/reclamation.
            bool HasRecordCapacity() const noexcept {
                return _availability.IsAnySet();
            }

            /// Moves one callable into a newly admitted bounded Task record and appends it to FIFO order.
            ///
            /// The owning facility runtime must serialize this operation with queue mutation,
            /// cancellation, Worker claim, owner release and reclamation.
            /// @tparam TCallable Callable Type being dispatched or adapted.
            template<class TCallable>
            AdmissionResult Admit(
                TCallable&& callable
            ) noexcept {
                /// Decayed callable Type retained in bounded Task payload storage.
                using StoredCallable = std::decay_t<TCallable>;
                /// Result Type produced by the stored callable.
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
                    std::is_nothrow_destructible_v<StoredCallable>,
                    "Task callable teardown from deterministic Task storage must be no-throw"
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
                        std::is_nothrow_destructible_v<Result>,
                        "Task result teardown from deterministic Task storage must be no-throw"
                    );

                    static_assert(
                        alignof(Result) <= alignof(std::max_align_t),
                        "Over-aligned Task result Types are not supported by the v1 bounded payload"
                    );
                }

                const auto topologyIndex = _availability.FindFirstSet();

                if (!topologyIndex.IsValid()) {
                    return AdmissionResult::CapacityUnavailable();
                }

                if (
                    _availability.Clear(
                        topologyIndex
                    ) != ESPressio::BoundedTopology::BoundedIndexSetMutationResult::Succeeded
                ) {
                    return AdmissionResult::CapacityUnavailable();
                }

                const auto recordIndex = static_cast<Index>(
                    topologyIndex.Value()
                );
                auto& record = _records[recordIndex];

                record.Control.InitializeQueued();

                new (record.Payload) StoredCallable(
                    std::forward<TCallable>(
                        callable
                    )
                );

                /// Payload adapter Type responsible for callable invocation and result publication.
                using Adapter = TaskPayloadAdapter<
                    Record,
                    StoredCallable,
                    Result
                >;

                record.PayloadOperations = &Adapter::Operations;

                static_cast<void>(
                    _queue.Push(
                        _records,
                        topologyIndex
                    )
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
            WorkerClaimResult ClaimNextForWorker(
                ExecutionContextIndex contextIndex
            ) noexcept {
                TopologyIndexType topologyIndex;

                if (
                    _queue.Pop(
                        _records,
                        topologyIndex
                    ) != ESPressio::BoundedTopology::IntrusiveQueuePopResult::Succeeded
                ) {
                    return WorkerClaimResult::QueueEmpty();
                }

                const auto recordIndex = static_cast<Index>(
                    topologyIndex.Value()
                );
                auto& record = _records[recordIndex];

                record.SetExecutionContextIndex(
                    contextIndex
                );

                record.Control.SetState(
                    TaskOperationalState::Running
                );

                return WorkerClaimResult::Claimed(
                    recordIndex,
                    record.Control.Phase()
                );
            }

            /// Returns the Task binding currently granted to one managed Worker context.
            std::optional<TaskRecordBinding<Index>> AssignedTaskForContext(
                ExecutionContextIndex contextIndex
            ) const noexcept {
                for (std::size_t index = 0U; index < TRecordCapacity; ++index) {
                    const auto& record = _records[index];
                    const auto state = record.Control.State();

                    if (
                        (
                            state == TaskOperationalState::Running ||
                            state == TaskOperationalState::RunningCancelRequested
                        ) &&
                        record.CurrentExecutionContextIndex() == contextIndex
                    ) {
                        return TaskRecordBinding<Index>{
                            static_cast<Index>(
                                index
                            ),
                            record.Control.Phase()
                        };
                    }
                }

                return std::nullopt;
            }

            /// Executes one Worker-owned Task payload outside the facility serialization boundary.
            TaskInvocationOutcome Invoke(
                Index recordIndex,
                TaskContext& context
            ) {
                auto& record = _records[recordIndex];

                return record.PayloadOperations->Invoke(
                    record,
                    context
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


            /// Indicates whether one Task incarnation has ever won a Worker Lease.
            ///
            /// Queued cancellation/abandonment normalizes scratch to the invalid context sentinel,
            /// while every granted Task retains its valid executing context index through terminal
            /// publication. This lets dispatch timeout distinguish "never granted" Cancelled from
            /// "grant already won, then later Cancelled" without another control bit.
            bool HasWorkerGrant(
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

                if (state == TaskOperationalState::Queued) {
                    return false;
                }

                if (state == TaskOperationalState::Cancelled) {
                    return record.CurrentExecutionContextIndex() !=
                        Record::InvalidExecutionContextIndex;
                }

                return true;
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
                                ToTopologyIndex(
                                    recordIndex
                                )
                            )
                        );

                        record.PayloadOperations->DestroyCallable(
                            record
                        );

                        record.SetExecutionContextIndex(
                            Record::InvalidExecutionContextIndex
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
                                ToTopologyIndex(
                                    recordIndex
                                )
                            )
                        );

                        record.PayloadOperations->DestroyCallable(
                            record
                        );

                        record.SetExecutionContextIndex(
                            Record::InvalidExecutionContextIndex
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
                    _availability.IsSet(
                        ToTopologyIndex(
                            recordIndex
                        )
                    )
                ) {
                    return TaskReclaimResult::NotEligible;
                }

                auto& record = _records[recordIndex];
                record.PayloadOperations = nullptr;
                record.SetQueueNext(
                    TopologyIndexType::Invalid()
                );

                static_cast<void>(
                    _availability.Set(
                        ToTopologyIndex(
                            recordIndex
                        )
                    )
                );

                return TaskReclaimResult::Reclaimed;
            }


            // Bounded traversal.

            /// Visits every structurally allocated Task incarnation while the owning facility lock is held.
            /// @tparam TVisitor Visitor callable Type invoked for each selected bounded record.
            template<class TVisitor>
            void VisitAllocated(
                TVisitor&& visitor
            ) {
                for (std::size_t index = 0U; index < TRecordCapacity; ++index) {
                    const auto recordIndex = static_cast<Index>(
                        index
                    );

                    if (_availability.IsSet(
                        ToTopologyIndex(
                            recordIndex
                        )
                    )) {
                        continue;
                    }

                    visitor(
                        TaskRecordBinding<Index>{
                            recordIndex,
                            _records[recordIndex].Control.Phase()
                        }
                    );
                }
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
                    if (
                        !_availability.IsSet(
                            TopologyIndexType::FromUnchecked(
                                index
                            )
                        )
                    ) {
                        ++inUse;
                    }
                }

                return inUse;
            }

            /// Indicates whether any admitted Task still has execution work outstanding.
            ///
            /// Completed/Cancelled records retained solely by public ownership or waiters do not
            /// keep Worker infrastructure alive during terminal shutdown.
            bool HasExecutionWork() const noexcept {
                for (std::size_t index = 0U; index < TRecordCapacity; ++index) {
                    const auto recordIndex = static_cast<Index>(
                        index
                    );

                    if (_availability.IsSet(
                        ToTopologyIndex(
                            recordIndex
                        )
                    )) {
                        continue;
                    }

                    const auto state = _records[recordIndex].Control.State();

                    if (
                        state == TaskOperationalState::Queued ||
                        state == TaskOperationalState::Running ||
                        state == TaskOperationalState::RunningCancelRequested
                    ) {
                        return true;
                    }
                }

                return false;
            }


            /// Returns the number of currently queued Tasks.
            ///
            /// The owning facility runtime must serialize this traversal with queue mutation.
            std::size_t QueuedTasks() const noexcept {
                std::size_t queued = 0U;
                auto current = _queue.Head();

                while (current.IsValid()) {
                    ++queued;
                    current = _records[
                        static_cast<std::size_t>(
                            current.Value()
                        )
                    ].QueueNext();
                }

                return queued;
            }


            // Cooperative cancellation observation.

            /// Reads cancellation for one current Task incarnation.
            ///
            /// The owning facility runtime must serialize this read with every control mutation.
            bool IsCancellationRequested(
                Index recordIndex,
                bool phase
            ) const noexcept {
                return
                    IsCurrentIncarnation(
                        recordIndex,
                        phase
                    ) &&
                    _records[recordIndex].Control.IsCancellationRequested();
            }


            // Managed-context interruption discovery.

            /// Returns the Worker execution-context index for one Running Task incarnation.
            std::optional<ExecutionContextIndex> ExecutionContextFor(
                Index recordIndex,
                bool phase
            ) const noexcept {
                if (!IsCurrentIncarnation(
                    recordIndex,
                    phase
                )) {
                    return std::nullopt;
                }

                const auto& record = _records[recordIndex];
                const auto state = record.Control.State();

                if (
                    state != TaskOperationalState::Running &&
                    state != TaskOperationalState::RunningCancelRequested
                ) {
                    return std::nullopt;
                }

                return record.CurrentExecutionContextIndex();
            }

            /// Indicates whether the supplied managed Worker context is executing a cancellation-requested Task.
            ///
            /// The owning facility runtime must serialize this bounded scan with Worker claim and
            /// terminal publication because the scratch field is non-atomic and lifecycle-reused.
            bool IsCancellationRequestedForContext(
                ExecutionContextIndex contextIndex
            ) const noexcept {
                for (std::size_t index = 0U; index < TRecordCapacity; ++index) {
                    const auto& record = _records[index];

                    if (
                        record.Control.State() != TaskOperationalState::RunningCancelRequested ||
                        record.CurrentExecutionContextIndex() != contextIndex
                    ) {
                        continue;
                    }

                    return true;
                }

                return false;
            }

    };

} // ESPressio::Threading::Detail
