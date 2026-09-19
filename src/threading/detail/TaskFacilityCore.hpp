#pragma once

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

#include <ESPressio_Platform.hpp>

#include "../Task.hpp"
#include "CallableTraits.hpp"
#include "TaskPayloadAdapter.hpp"
#include "TaskStorage.hpp"

namespace ESPressio::Threading::Detail {

    template<
        std::size_t TRecordCapacity,
        std::size_t TCallableCapacity,
        std::size_t TResultCapacity,
        class TAtomicWord8Provider,
        class TAtomicWord32Provider,
        class TMutexProvider
    >
    class TaskFacilityCore final {

        static_assert(
            TRecordCapacity > 0U,
            "TaskFacilityCore requires positive record capacity"
        );

        public:

            using Record = TaskRecord<
                TAtomicWord8Provider,
                TCallableCapacity,
                TResultCapacity,
                TRecordCapacity
            >;

            using Index = typename SmallestIndex<TRecordCapacity>::Type;

        private:

            // Deterministic Task storage.

            /// Fixed Task record population.
            Record _records[TRecordCapacity];

            /// Facility-level free-record bitmap.
            TaskAvailabilityBitmap<
                TRecordCapacity,
                TAtomicWord32Provider
            > _availability;

            /// Intrusive FIFO over queued Task records.
            TaskFifo<
                Record,
                TRecordCapacity
            > _queue;


            // Queue synchronization.

            /// Protects FIFO linkage and queue-head/tail mutation.
            TMutexProvider _queueMutex;


            // Queue locking.

            /// Acquires the short-lived facility queue lock.
            bool AcquireQueueLock() noexcept {
                return _queueMutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                ) == ESPressio::Platform::Synchronization::LockAcquireResult::Acquired;
            }

            /// Releases the short-lived facility queue lock.
            void ReleaseQueueLock() noexcept {
                (void)_queueMutex.Release();
            }


            // Record reclamation.

            /// Republishes a record only after payload and public ownership no longer retain it.
            void TryReclaim(
                Record& record
            ) noexcept {
                const auto state = record.Control.State();

                if (
                    record.Control.HasOwner() ||
                    (
                        state != TaskOperationalState::Completed &&
                        state != TaskOperationalState::Cancelled
                    )
                ) {
                    return;
                }

                if (record.PayloadOperations != nullptr) {
                    record.PayloadOperations->Destroy(
                        record
                    );
                    record.PayloadOperations = nullptr;
                }

                const auto index = static_cast<std::size_t>(
                    &record - _records
                );

                _availability.Release(index);
            }


            // Type-erased Task handle operations.

            /// Resolves the owning concrete facility.
            static TaskFacilityCore& Owner(
                void* owner
            ) noexcept {
                return *static_cast<TaskFacilityCore*>(owner);
            }

            /// Resolves one concrete Task record.
            static Record& TaskRecordFrom(
                void* record
            ) noexcept {
                return *static_cast<Record*>(record);
            }

            /// Resolves one concrete Task record for observation.
            static const Record& TaskRecordFrom(
                const void* record
            ) noexcept {
                return *static_cast<const Record*>(record);
            }

            /// Projects the compact internal state onto the public Task lifecycle.
            static TaskState HandleState(
                const void*,
                const void* record
            ) noexcept {
                switch (TaskRecordFrom(record).Control.State()) {
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

            /// Placeholder until bounded waiter registration is attached in the next runtime layer.
            static TaskWaitResult HandleWait(
                void*,
                void*
            ) {
                return TaskWaitResult::Interrupted;
            }

            /// Placeholder until bounded timed waiter registration is attached in the next runtime layer.
            static TaskWaitResult HandleWaitFor(
                void*,
                void*,
                Duration
            ) {
                return TaskWaitResult::Interrupted;
            }

            /// Placeholder until canonical-deadline waiter registration is attached in the next runtime layer.
            static TaskWaitResult HandleWaitUntil(
                void*,
                void*,
                MonotonicTimestamp
            ) {
                return TaskWaitResult::Interrupted;
            }

            /// Requests queued cancellation or running cooperative cancellation.
            static TaskCancelResult HandleCancel(
                void* owner,
                void* recordAddress
            ) noexcept {
                auto& facility = Owner(owner);
                auto& record = TaskRecordFrom(recordAddress);

                if (
                    record.Control.CompareExchangeState(
                        TaskOperationalState::Running,
                        TaskOperationalState::RunningCancelRequested
                    )
                ) {
                    return TaskCancelResult::Accepted;
                }

                if (record.Control.State() == TaskOperationalState::Queued) {
                    if (!facility.AcquireQueueLock()) {
                        return TaskCancelResult::Accepted;
                    }

                    const auto index = static_cast<Index>(
                        &record - facility._records
                    );

                    const auto removed = facility._queue.Remove(
                        facility._records,
                        index
                    );

                    facility.ReleaseQueueLock();

                    if (
                        removed &&
                        record.Control.CompareExchangeState(
                            TaskOperationalState::Queued,
                            TaskOperationalState::Cancelled
                        )
                    ) {
                        if (record.PayloadOperations != nullptr) {
                            record.PayloadOperations->Destroy(
                                record
                            );
                            record.PayloadOperations = nullptr;
                        }

                        facility.TryReclaim(record);
                        return TaskCancelResult::Accepted;
                    }
                }

                const auto state = record.Control.State();

                return (
                    state == TaskOperationalState::Completed ||
                    state == TaskOperationalState::Cancelled
                )
                    ? TaskCancelResult::AlreadyFinished
                    : TaskCancelResult::Accepted;
            }

            /// Releases the sole public ownership interest.
            static void HandleRelease(
                void* owner,
                void* recordAddress
            ) noexcept {
                auto& facility = Owner(owner);
                auto& record = TaskRecordFrom(recordAddress);

                record.Control.ReleaseOwner();

                if (record.Control.State() == TaskOperationalState::Queued) {
                    (void)HandleCancel(
                        owner,
                        recordAddress
                    );
                }

                facility.TryReclaim(record);
            }

            /// Moves a completed result out and consumes public ownership.
            static TaskTakeStatus HandleTakeResult(
                void* owner,
                void* recordAddress,
                void* destination
            ) {
                auto& facility = Owner(owner);
                auto& record = TaskRecordFrom(recordAddress);
                const auto state = record.Control.State();

                if (state == TaskOperationalState::Cancelled) {
                    return TaskTakeStatus::Cancelled;
                }

                if (state != TaskOperationalState::Completed) {
                    return TaskTakeStatus::NotCompleted;
                }

                record.PayloadOperations->MoveResult(
                    record,
                    destination
                );
                record.PayloadOperations = nullptr;
                record.Control.ReleaseOwner();
                facility.TryReclaim(record);

                return TaskTakeStatus::Succeeded;
            }

            /// Shared Task-handle operation table for this concrete facility.
            inline static const TaskHandleOperations HandleOperations {
                &HandleState,
                &HandleWait,
                &HandleWaitFor,
                &HandleWaitUntil,
                &HandleCancel,
                &HandleRelease,
                &HandleTakeResult
            };

        public:

            // Admission.

            /// Attempts immediate bounded admission and queues one callable without waiting for capacity.
            template<class TCallable>
            auto TryAdmit(
                TCallable&& callable
            ) {
                using StoredCallable = std::decay_t<TCallable>;
                using Result = CallableResultT<StoredCallable>;
                using Adapter = TaskPayloadAdapter<
                    Record,
                    StoredCallable,
                    Result
                >;

                static_assert(
                    sizeof(StoredCallable) <= TCallableCapacity,
                    "Task callable exceeds TaskExecutionFacility CallableCapacity"
                );

                static_assert(
                    std::is_void_v<Result> ||
                    sizeof(Result) <= TResultCapacity,
                    "Task result exceeds TaskExecutionFacility ResultCapacity"
                );

                std::size_t claimedIndex = 0U;

                if (!_availability.TryClaim(
                    claimedIndex
                )) {
                    return TaskDispatchResult<Task<Result>>(
                        TaskDispatchStatus::Unavailable
                    );
                }

                auto& record = _records[claimedIndex];

                new (record.Payload) StoredCallable(
                    std::forward<TCallable>(callable)
                );

                record.PayloadOperations = &Adapter::Operations;
                record.Control.InitializeQueued();

                if (!AcquireQueueLock()) {
                    record.PayloadOperations->Destroy(
                        record
                    );
                    record.PayloadOperations = nullptr;
                    record.Control.SetState(
                        TaskOperationalState::Cancelled
                    );
                    record.Control.ReleaseOwner();
                    _availability.Release(claimedIndex);

                    return TaskDispatchResult<Task<Result>>(
                        TaskDispatchStatus::Unavailable
                    );
                }

                _queue.Push(
                    _records,
                    static_cast<Index>(claimedIndex)
                );

                ReleaseQueueLock();

                return TaskDispatchResult<Task<Result>>(
                    Task<Result>(
                        this,
                        &record,
                        HandleOperations
                    )
                );
            }


            // Worker consumption.

            /// Claims the next queued record for one Worker without executing it.
            Record* TryTakeNext() noexcept {
                if (!AcquireQueueLock()) {
                    return nullptr;
                }

                for (;;) {
                    const auto index = _queue.Pop(
                        _records
                    );

                    if (index == SmallestIndex<TRecordCapacity>::Invalid) {
                        ReleaseQueueLock();
                        return nullptr;
                    }

                    auto& record = _records[index];

                    if (
                        record.Control.CompareExchangeState(
                            TaskOperationalState::Queued,
                            TaskOperationalState::Running
                        )
                    ) {
                        ReleaseQueueLock();
                        return &record;
                    }
                }
            }

            /// Executes one Worker-owned record and then evaluates reclamation.
            void Execute(
                Record& record
            ) {
                record.PayloadOperations->Invoke(
                    record
                );

                TryReclaim(record);
            }

    };

} // ESPressio::Threading::Detail
