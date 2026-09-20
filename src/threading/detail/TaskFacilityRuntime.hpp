#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

#include <ESPressio_Platform.hpp>

#include "../Task.hpp"
#include "MonotonicWaitBudget.hpp"
#include "TaskFacilityCore.hpp"
#include "WaitRegistration.hpp"
#include "WorkerLeaseScheduler.hpp"

namespace ESPressio::Threading::Detail {

    enum class TaskFacilitySynchronizationResult : std::uint8_t {
        Ready = 0,
        ProviderFailure = 1
    };


    enum class TaskFacilityLockResult : std::uint8_t {
        Acquired = 0,
        ProviderFailure = 1
    };


    /// Defines the compile-time contract for `TaskFacilityRuntime`.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TRecordCapacity Task-record capacity declaration Type or bounded capacity.
    /// @tparam TCallableCapacity Callable-storage capacity declaration Type or byte capacity.
    /// @tparam TResultCapacity Result-storage capacity declaration Type or byte capacity.
    /// @tparam TWorkerCount Compile-time template parameter `TWorkerCount` used by this declaration.
    /// @tparam TFirstWorkerContextIndex Compile-time template parameter `TFirstWorkerContextIndex` used by this declaration.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<std::size_t TRecordCapacity, std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TWorkerCount, std::size_t TFirstWorkerContextIndex, std::size_t TExecutionContextCapacity, class TMutexProvider, class TManagedContextRouter>
    class TaskFacilityRuntime final {

        static_assert(
            TExecutionContextCapacity == TManagedContextRouter::ContextCapacity,
            "Task facility and managed-context router must use the same execution-context capacity"
        );

        private:

            // Core Types.

            /// Deterministic Task record/FIFO state core.
            using Core = TaskFacilityCore<
                TRecordCapacity,
                TCallableCapacity,
                TResultCapacity,
                TExecutionContextCapacity
            >;

            /// Compact record index selected by the deterministic core.
            using Index = typename Core::Index;

            /// Dense managed execution-context index used by this topology.
            using ContextIndex = typename Core::ExecutionContextIndex;

            /// Target-owned Task waiter registration.
            using WaitRegistration = TaskWaitRegistration<
                Index,
                ContextIndex
            >;

            /// Bounded Task waiter registrations sufficient for every managed sequential context.
            using Waiters = RegistrationSet<
                WaitRegistration,
                TExecutionContextCapacity
            >;

            /// Facility-level pre-admission capacity waiter registration.
            using AdmissionWaitRegistrationType = AdmissionWaitRegistration<
                ContextIndex
            >;

            /// Bounded pre-admission waiters; no Task record exists while one is active.
            using AdmissionWaiters = RegistrationSet<
                AdmissionWaitRegistrationType,
                TExecutionContextCapacity
            >;

            /// Bounded availability scheduler for this facility's statically configured Workers.
            using WorkerScheduler = WorkerLeaseScheduler<
                TWorkerCount,
                TFirstWorkerContextIndex,
                TExecutionContextCapacity
            >;


            // Runtime state.

            /// Deterministic record/FIFO/lifecycle state.
            Core _core;

            /// Facility-local serialization primitive.
            TMutexProvider _mutex;

            /// Target-owned bounded Task waiter registrations.
            Waiters _waiters;

            /// Facility-owned bounded pre-admission waiter registrations.
            AdmissionWaiters _admissionWaiters;

            /// Facility-owned Worker availability state.
            WorkerScheduler _workerScheduler;

            /// Non-owning topology-stable managed-context router.
            TManagedContextRouter* _router;


            // Invocation-local cancellation observation.

            struct CancellationObservation final {

                TaskFacilityRuntime* Facility;
                Index RecordIndex;
                bool Phase;

            };


            static bool IsCancellationRequestedThunk(
                const void* context
            ) noexcept {
                const auto* observation = static_cast<const CancellationObservation*>(
                    context
                );

                return observation->Facility->IsCancellationRequested(
                    observation->RecordIndex,
                    observation->Phase
                );
            }


            // Facility serialization.

            /// Acquires the facility serialization boundary indefinitely.
            TaskFacilityLockResult AcquireLock() noexcept {
                const auto result = _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                );

                return result == ESPressio::Platform::Synchronization::LockAcquireResult::Acquired
                    ? TaskFacilityLockResult::Acquired
                    : TaskFacilityLockResult::ProviderFailure;
            }

            /// Releases the facility serialization boundary.
            void ReleaseLock() noexcept {
                static_cast<void>(
                    _mutex.Release()
                );
            }

            /// Reads cooperative cancellation through the authoritative facility serialization boundary.
            bool IsCancellationRequested(
                Index recordIndex,
                bool phase
            ) noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return true;
                }

                const auto result = _core.IsCancellationRequested(
                    recordIndex,
                    phase
                );

                ReleaseLock();
                return result;
            }


            // Waiter matching and reclamation.

            /// Indicates whether one Task waiter targets the supplied record incarnation.
            static bool Matches(
                const WaitRegistration& registration,
                Index recordIndex,
                bool phase
            ) noexcept {
                return registration.RecordIndex == recordIndex &&
                    registration.Phase == phase;
            }

            /// Returns the number of active waiters targeting one Task incarnation.
            std::size_t MatchingWaiterCount(
                Index recordIndex,
                bool phase
            ) const {
                return _waiters.MatchingCount(
                    [recordIndex, phase](
                        const WaitRegistration& registration
                    ) {
                        return Matches(
                            registration,
                            recordIndex,
                            phase
                        );
                    }
                );
            }

            /// Wakes every managed context waiting on one terminal Task incarnation.
            void WakeMatchingWaiters(
                Index recordIndex,
                bool phase
            ) {
                _waiters.VisitActive(
                    [this, recordIndex, phase](
                        WaitRegistration& registration
                    ) {
                        if (!Matches(
                            registration,
                            recordIndex,
                            phase
                        )) {
                            return;
                        }

                        static_cast<void>(
                            _router->Wake(
                                registration.WaitingContextIndex
                            )
                        );
                    }
                );
            }

            /// Wakes every managed context currently blocked for Task-record admission capacity.
            void WakeAdmissionWaiters() {
                _admissionWaiters.VisitActive(
                    [this](
                        AdmissionWaitRegistrationType& registration
                    ) {
                        static_cast<void>(
                            _router->Wake(
                                registration.WaitingContextIndex
                            )
                        );
                    }
                );
            }

            /// Grants queued Tasks to currently available Workers in FIFO opportunity order.
            ///
            /// The facility lock must be held throughout this scheduling pass. Every successful
            /// grant publishes Running with the selected dense Worker context before either the
            /// dispatch waiter or Worker context is woken.
            void ScheduleAvailableWorkers() {
                for (;;) {
                    const auto contextIndex = _workerScheduler.TryClaimAvailable();

                    if (!contextIndex.has_value()) {
                        return;
                    }

                    const auto claim = _core.ClaimNextForWorker(
                        contextIndex.value()
                    );

                    if (!claim.IsClaimed()) {
                        static_cast<void>(
                            _workerScheduler.MarkAvailable(
                                contextIndex.value()
                            )
                        );
                        return;
                    }

                    const auto& binding = claim.Binding().value();

                    WakeMatchingWaiters(
                        binding.RecordIndex,
                        binding.Phase
                    );

                    static_cast<void>(
                        _router->Wake(
                            contextIndex.value()
                        )
                    );
                }
            }

            /// Reclaims one ownerless terminal record when no waiter still retains its incarnation.
            TaskReclaimResult ReclaimIfQuiescent(
                Index recordIndex,
                bool phase
            ) noexcept {
                if (
                    !_core.IsOwnerlessTerminal(
                        recordIndex,
                        phase
                    ) ||
                    MatchingWaiterCount(
                        recordIndex,
                        phase
                    ) != 0U
                ) {
                    return TaskReclaimResult::NotEligible;
                }

                const auto reclaimResult = _core.Reclaim(
                    recordIndex,
                    phase
                );

                if (reclaimResult != TaskReclaimResult::Reclaimed) {
                    return reclaimResult;
                }

                WakeAdmissionWaiters();
                return TaskReclaimResult::Reclaimed;
            }


            // Wait registration lifecycle.

            /// Removes one wait registration and applies final waiter-dependent reclamation.
            void UnregisterWaiter(
                std::size_t registrationIndex,
                Index recordIndex,
                bool phase
            ) noexcept {
                _waiters.Unregister(
                    registrationIndex
                );

                ReclaimIfQuiescent(
                    recordIndex,
                    phase
                );
            }


            // Predicate-driven waiting.

            /// Waits for one Task incarnation according to a canonical non-restarting wait budget.
            TaskWaitResult WaitWithBudget(
                Index recordIndex,
                bool phase,
                const MonotonicWaitBudget& budget
            ) {
                // An already-terminal target satisfies Wait immediately without requiring
                // a managed waiting context or consulting cooperative interruption state.
                if (_core.IsTerminal(
                    recordIndex,
                    phase
                )) {
                    return TaskWaitResult::Finished;
                }

                const auto contextIndex = _router->CurrentContextIndex();

                if (!contextIndex.has_value()) {
                    return TaskWaitResult::Interrupted;
                }

                if (_router->IsInterrupted(
                    contextIndex.value()
                )) {
                    return TaskWaitResult::Interrupted;
                }

                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return TaskWaitResult::Interrupted;
                }

                // Re-observe after entering the target-resource serialization boundary because
                // terminal publication may have won between the first observation and this lock.
                if (_core.IsTerminal(
                    recordIndex,
                    phase
                )) {
                    ReleaseLock();
                    return TaskWaitResult::Finished;
                }

                WaitRegistration registration;
                registration.RecordIndex = recordIndex;
                registration.Phase = phase;
                registration.WaitingContextIndex = contextIndex.value();

                std::size_t registrationIndex = 0U;

                if (
                    _waiters.Register(
                        registration,
                        registrationIndex
                    ) != WaitRegistrationStatus::Registered
                ) {
                    ReleaseLock();
                    return TaskWaitResult::Interrupted;
                }

                // Re-observe while still inside the same target-resource serialization boundary.
                // Terminal publication cannot pass between the first observation and registration.
                if (_core.IsTerminal(
                    recordIndex,
                    phase
                )) {
                    UnregisterWaiter(
                        registrationIndex,
                        recordIndex,
                        phase
                    );
                    ReleaseLock();
                    return TaskWaitResult::Finished;
                }

                ReleaseLock();

                for (;;) {
                    const auto remaining = budget.Remaining();

                    if (remaining.IsNoWait()) {
                        if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                            return TaskWaitResult::Interrupted;
                        }

                        if (_core.IsTerminal(
                            recordIndex,
                            phase
                        )) {
                            UnregisterWaiter(
                                registrationIndex,
                                recordIndex,
                                phase
                            );
                            ReleaseLock();
                            return TaskWaitResult::Finished;
                        }

                        UnregisterWaiter(
                            registrationIndex,
                            recordIndex,
                            phase
                        );
                        ReleaseLock();
                        return TaskWaitResult::TimedOut;
                    }

                    const auto waitResult = _router->Wait(
                        contextIndex.value(),
                        remaining
                    );

                    if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                        return TaskWaitResult::Interrupted;
                    }

                    if (_core.IsTerminal(
                        recordIndex,
                        phase
                    )) {
                        UnregisterWaiter(
                            registrationIndex,
                            recordIndex,
                            phase
                        );
                        ReleaseLock();
                        return TaskWaitResult::Finished;
                    }

                    ReleaseLock();

                    if (_router->IsInterrupted(
                        contextIndex.value()
                    )) {
                        if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                            return TaskWaitResult::Interrupted;
                        }

                        if (_core.IsTerminal(
                            recordIndex,
                            phase
                        )) {
                            UnregisterWaiter(
                                registrationIndex,
                                recordIndex,
                                phase
                            );
                            ReleaseLock();
                            return TaskWaitResult::Finished;
                        }

                        UnregisterWaiter(
                            registrationIndex,
                            recordIndex,
                            phase
                        );
                        ReleaseLock();
                        return TaskWaitResult::Interrupted;
                    }

                    if (
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::ProviderFailure
                    ) {
                        if (AcquireLock() == TaskFacilityLockResult::Acquired) {
                            UnregisterWaiter(
                                registrationIndex,
                                recordIndex,
                                phase
                            );
                            ReleaseLock();
                        }

                        return TaskWaitResult::Interrupted;
                    }

                    if (
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut &&
                        budget.Remaining().IsNoWait()
                    ) {
                        if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                            return TaskWaitResult::Interrupted;
                        }

                        if (_core.IsTerminal(
                            recordIndex,
                            phase
                        )) {
                            UnregisterWaiter(
                                registrationIndex,
                                recordIndex,
                                phase
                            );
                            ReleaseLock();
                            return TaskWaitResult::Finished;
                        }

                        UnregisterWaiter(
                            registrationIndex,
                            recordIndex,
                            phase
                        );
                        ReleaseLock();
                        return TaskWaitResult::TimedOut;
                    }

                    // Signaled or a premature coarse native timeout: authoritative state remains
                    // nonterminal and the waiting context remains uninterrupted, so re-wait using
                    // the original canonical budget rather than inventing a new timeout.
                }
            }


            // Task handle operation thunks.

            /// Reads Task state through the type-erased public handle operation table.
            static TaskState StateThunk(
                const void* owner,
                std::uint32_t recordIndex,
                bool phase
            ) noexcept {
                return static_cast<const TaskFacilityRuntime*>(owner)->State(
                    static_cast<Index>(
                        recordIndex
                    ),
                    phase
                );
            }

            /// Performs indefinite Task wait through the public handle operation table.
            static TaskWaitResult WaitThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase
            ) {
                return static_cast<TaskFacilityRuntime*>(owner)->Wait(
                    static_cast<Index>(
                        recordIndex
                    ),
                    phase
                );
            }

            /// Performs relative Task wait through the public handle operation table.
            static TaskWaitResult WaitForThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase,
                Duration duration
            ) {
                return static_cast<TaskFacilityRuntime*>(owner)->WaitFor(
                    static_cast<Index>(
                        recordIndex
                    ),
                    phase,
                    duration
                );
            }

            /// Performs deadline Task wait through the public handle operation table.
            static TaskWaitResult WaitUntilThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase,
                MonotonicTimestamp deadline
            ) {
                return static_cast<TaskFacilityRuntime*>(owner)->WaitUntil(
                    static_cast<Index>(
                        recordIndex
                    ),
                    phase,
                    deadline
                );
            }

            /// Performs Task cancellation through the public handle operation table.
            static TaskCancelResult CancelThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase
            ) noexcept {
                return static_cast<TaskFacilityRuntime*>(owner)->Cancel(
                    static_cast<Index>(
                        recordIndex
                    ),
                    phase
                );
            }

            /// Releases public Task ownership through the public handle operation table.
            static void ReleaseThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase
            ) noexcept {
                static_cast<TaskFacilityRuntime*>(owner)->ReleaseOwner(
                    static_cast<Index>(
                        recordIndex
                    ),
                    phase
                );
            }

            /// Performs consuming Task result extraction through the public handle operation table.
            static TaskTakeStatus TakeResultThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase,
                void* destination
            ) {
                return static_cast<TaskFacilityRuntime*>(owner)->TakeResult(
                    static_cast<Index>(
                        recordIndex
                    ),
                    phase,
                    destination
                );
            }

            // Dispatch waiting.

            TaskDispatchStatus WaitForAdmissionCapacity(
                ContextIndex contextIndex,
                const MonotonicWaitBudget& budget
            ) {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return TaskDispatchStatus::Interrupted;
                }

                if (_core.HasRecordCapacity()) {
                    ReleaseLock();
                    return TaskDispatchStatus::Succeeded;
                }

                AdmissionWaitRegistrationType registration;
                registration.WaitingContextIndex = contextIndex;
                std::size_t registrationIndex = 0U;

                if (_admissionWaiters.Register(registration, registrationIndex) != WaitRegistrationStatus::Registered) {
                    ReleaseLock();
                    return TaskDispatchStatus::Interrupted;
                }

                if (_core.HasRecordCapacity()) {
                    _admissionWaiters.Unregister(registrationIndex);
                    ReleaseLock();
                    return TaskDispatchStatus::Succeeded;
                }

                ReleaseLock();

                for (;;) {
                    const auto remaining = budget.Remaining();

                    if (remaining.IsNoWait()) {
                        if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                            return TaskDispatchStatus::Interrupted;
                        }

                        const auto available = _core.HasRecordCapacity();
                        _admissionWaiters.Unregister(registrationIndex);
                        ReleaseLock();
                        return available ? TaskDispatchStatus::Succeeded : TaskDispatchStatus::TimedOut;
                    }

                    const auto waitResult = _router->Wait(contextIndex, remaining);

                    if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                        return TaskDispatchStatus::Interrupted;
                    }

                    if (_core.HasRecordCapacity()) {
                        _admissionWaiters.Unregister(registrationIndex);
                        ReleaseLock();
                        return TaskDispatchStatus::Succeeded;
                    }

                    const auto interrupted = _router->IsInterrupted(contextIndex);
                    const auto expired = budget.Remaining().IsNoWait();
                    const auto providerFailed =
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::ProviderFailure;

                    if (interrupted || expired || providerFailed) {
                        _admissionWaiters.Unregister(registrationIndex);
                        ReleaseLock();

                        if (interrupted || providerFailed) {
                            return TaskDispatchStatus::Interrupted;
                        }

                        return TaskDispatchStatus::TimedOut;
                    }

                    ReleaseLock();
                }
            }

            void WithdrawUnreturnedDispatch(
                Index recordIndex,
                bool phase
            ) noexcept {
                static_cast<void>(_core.ReleaseOwner(recordIndex, phase));
                static_cast<void>(ReclaimIfQuiescent(recordIndex, phase));
            }

            TaskDispatchStatus WaitForWorkerGrant(
                ContextIndex contextIndex,
                Index recordIndex,
                bool phase,
                const MonotonicWaitBudget& budget
            ) {
                WaitRegistration registration;
                registration.RecordIndex = recordIndex;
                registration.Phase = phase;
                registration.WaitingContextIndex = contextIndex;
                std::size_t registrationIndex = 0U;

                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return TaskDispatchStatus::Interrupted;
                }

                if (_core.HasWorkerGrant(recordIndex, phase)) {
                    ReleaseLock();
                    return TaskDispatchStatus::Succeeded;
                }

                if (_waiters.Register(registration, registrationIndex) != WaitRegistrationStatus::Registered) {
                    WithdrawUnreturnedDispatch(recordIndex, phase);
                    ReleaseLock();
                    return TaskDispatchStatus::Interrupted;
                }

                if (_core.HasWorkerGrant(recordIndex, phase)) {
                    UnregisterWaiter(registrationIndex, recordIndex, phase);
                    ReleaseLock();
                    return TaskDispatchStatus::Succeeded;
                }

                ReleaseLock();

                for (;;) {
                    const auto remaining = budget.Remaining();

                    if (remaining.IsNoWait()) {
                        if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                            return TaskDispatchStatus::Interrupted;
                        }

                        if (_core.HasWorkerGrant(recordIndex, phase)) {
                            UnregisterWaiter(registrationIndex, recordIndex, phase);
                            ReleaseLock();
                            return TaskDispatchStatus::Succeeded;
                        }

                        _waiters.Unregister(registrationIndex);
                        WithdrawUnreturnedDispatch(recordIndex, phase);
                        ReleaseLock();
                        return TaskDispatchStatus::TimedOut;
                    }

                    const auto waitResult = _router->Wait(contextIndex, remaining);

                    if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                        return TaskDispatchStatus::Interrupted;
                    }

                    if (_core.HasWorkerGrant(recordIndex, phase)) {
                        UnregisterWaiter(registrationIndex, recordIndex, phase);
                        ReleaseLock();
                        return TaskDispatchStatus::Succeeded;
                    }

                    const auto interrupted = _router->IsInterrupted(contextIndex);
                    const auto expired = budget.Remaining().IsNoWait();
                    const auto providerFailed =
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::ProviderFailure;

                    if (interrupted || expired || providerFailed) {
                        _waiters.Unregister(registrationIndex);
                        WithdrawUnreturnedDispatch(recordIndex, phase);
                        ReleaseLock();

                        if (interrupted || providerFailed) {
                            return TaskDispatchStatus::Interrupted;
                        }

                        return TaskDispatchStatus::TimedOut;
                    }

                    ReleaseLock();
                }
            }


        public:

            // Public Type aliases used by the owning topology runtime.

            /// Compact Task record index Type.
            using RecordIndex = Index;

            /// Dense managed execution-context index Type.
            using ManagedContextIndex = ContextIndex;

            /// Structured deterministic admission result.
            using AdmissionResult = typename Core::AdmissionResult;

            /// Structured deterministic Worker claim result.
            using WorkerClaimResult = typename Core::WorkerClaimResult;

            /// Public Task handle Type produced by one callable Type.
            /// @tparam TCallable Callable Type being dispatched or adapted.
            template<class TCallable>
            /// Task handle Type produced for the supplied callable.
            using TaskForCallable = Task<
                CallableResultT<std::decay_t<TCallable>>
            >;

            /// Structured public dispatch result produced for one callable Type.
            /// @tparam TCallable Callable Type being dispatched or adapted.
            template<class TCallable>
            /// Typed dispatch result produced for the supplied callable.
            using DispatchResultFor = TaskDispatchResult<
                TaskForCallable<TCallable>
            >;


            // Construction.

            /// Binds this static Task facility to its topology-owned managed-context router.
            explicit TaskFacilityRuntime(
                TManagedContextRouter& router
            ) noexcept :
                _router(&router) {}


            // Bootstrap synchronization validation.

            /// Validates that the facility mutex provider can acquire and release before execution starts.
            TaskFacilitySynchronizationResult ValidateSynchronization() noexcept {
                const auto acquireResult = _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
                );

                if (acquireResult != ESPressio::Platform::Synchronization::LockAcquireResult::Acquired) {
                    return TaskFacilitySynchronizationResult::ProviderFailure;
                }

                const auto releaseResult = _mutex.Release();

                return releaseResult == ESPressio::Platform::Synchronization::LockReleaseResult::Released
                    ? TaskFacilitySynchronizationResult::Ready
                    : TaskFacilitySynchronizationResult::ProviderFailure;
            }


            // Dispatch.

            /// Defines the compile-time contract for `Dispatch`.
            /// @tparam TCallable Callable Type being dispatched or adapted.
            template<class TCallable>
            DispatchResultFor<TCallable> Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            ) {
                /// Task handle Type instantiated for the callable being dispatched.
                using TaskType = TaskForCallable<TCallable>;
                /// Typed dispatch result returned for the callable being dispatched.
                using DispatchResult = DispatchResultFor<TCallable>;

                const auto contextIndex = _router->CurrentContextIndex();
                const auto budget = policy == TaskDispatchPolicy::QueueWithTimeout
                    ? MonotonicWaitBudget::For(timeout)
                    : MonotonicWaitBudget::Forever();

                for (;;) {
                    if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                        return DispatchResult(TaskDispatchStatus::Interrupted);
                    }

                    if (!_core.HasRecordCapacity()) {
                        ReleaseLock();

                        if (policy == TaskDispatchPolicy::AbandonImmediately) {
                            return DispatchResult(TaskDispatchStatus::Unavailable);
                        }

                        if (!contextIndex.has_value()) {
                            return DispatchResult(TaskDispatchStatus::Interrupted);
                        }

                        const auto capacity = WaitForAdmissionCapacity(contextIndex.value(), budget);

                        if (capacity != TaskDispatchStatus::Succeeded) {
                            return DispatchResult(capacity);
                        }

                        continue;
                    }

                    if (
                        policy == TaskDispatchPolicy::AbandonImmediately &&
                        (!_workerScheduler.IsAnyAvailable() || _core.QueuedTasks() != 0U)
                    ) {
                        ReleaseLock();
                        return DispatchResult(TaskDispatchStatus::Unavailable);
                    }

                    auto admission = _core.Admit(std::forward<TCallable>(callable));

                    if (!admission.IsAdmitted()) {
                        ReleaseLock();
                        continue;
                    }

                    const auto binding = admission.Binding().value();
                    ScheduleAvailableWorkers();
                    const auto workerGranted = _core.HasWorkerGrant(binding.RecordIndex, binding.Phase);

                    if (policy == TaskDispatchPolicy::AbandonImmediately && !workerGranted) {
                        WithdrawUnreturnedDispatch(binding.RecordIndex, binding.Phase);
                        ReleaseLock();
                        return DispatchResult(TaskDispatchStatus::Unavailable);
                    }

                    if (policy != TaskDispatchPolicy::QueueWithTimeout || workerGranted) {
                        TaskType task(
                            this,
                            static_cast<std::uint32_t>(binding.RecordIndex),
                            binding.Phase,
                            HandleOperations()
                        );
                        ReleaseLock();
                        return DispatchResult(std::move(task));
                    }

                    if (!contextIndex.has_value()) {
                        WithdrawUnreturnedDispatch(binding.RecordIndex, binding.Phase);
                        ReleaseLock();
                        return DispatchResult(TaskDispatchStatus::Interrupted);
                    }

                    ReleaseLock();
                    const auto granted = WaitForWorkerGrant(
                        contextIndex.value(),
                        binding.RecordIndex,
                        binding.Phase,
                        budget
                    );

                    if (granted != TaskDispatchStatus::Succeeded) {
                        return DispatchResult(granted);
                    }

                    TaskType task(
                        this,
                        static_cast<std::uint32_t>(binding.RecordIndex),
                        binding.Phase,
                        HandleOperations()
                    );
                    return DispatchResult(std::move(task));
                }
            }


            // Admission.

            /// Admits one callable into deterministic facility storage when record capacity is available.
            /// @tparam TCallable Callable Type being dispatched or adapted.
            template<class TCallable>
            AdmissionResult Admit(
                TCallable&& callable
            ) noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return AdmissionResult::CapacityUnavailable();
                }

                auto result = _core.Admit(
                    std::forward<TCallable>(
                        callable
                    )
                );

                if (result.IsAdmitted()) {
                    ScheduleAvailableWorkers();
                }

                ReleaseLock();
                return result;
            }


            // Worker execution.

            /// Publishes one infrastructure-started Worker as available and grants queued work when present.
            WorkerAvailabilityResult WorkerBecameAvailable(
                ContextIndex contextIndex
            ) noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return WorkerAvailabilityResult::OutsideFacilityRange;
                }

                const auto result = _workerScheduler.MarkAvailable(
                    contextIndex
                );

                if (result == WorkerAvailabilityResult::Available) {
                    ScheduleAvailableWorkers();
                }

                ReleaseLock();
                return result;
            }

            /// Returns the Task currently granted to one Worker context, when one exists.
            std::optional<TaskRecordBinding<Index>> AssignedTaskForContext(
                ContextIndex contextIndex
            ) noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return std::nullopt;
                }

                const auto result = _core.AssignedTaskForContext(
                    contextIndex
                );

                ReleaseLock();
                return result;
            }

            /// Invokes one Worker-owned Task payload outside the facility lock.
            TaskInvocationOutcome Invoke(
                Index recordIndex,
                bool phase
            ) {
                CancellationObservation observation {
                    this,
                    recordIndex,
                    phase
                };

                TaskContext context(
                    &observation,
                    &IsCancellationRequestedThunk
                );

                return _core.Invoke(
                    recordIndex,
                    context
                );
            }

            /// Publishes one Worker outcome, releases that Worker Lease, and atomically schedules FIFO work.
            void CompleteWorkerTask(
                ContextIndex contextIndex,
                Index recordIndex,
                bool phase,
                TaskInvocationOutcome outcome
            ) noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return;
                }

                _core.PublishInvocationOutcome(
                    recordIndex,
                    phase,
                    outcome
                );

                WakeMatchingWaiters(
                    recordIndex,
                    phase
                );

                static_cast<void>(
                    ReclaimIfQuiescent(
                        recordIndex,
                        phase
                    )
                );

                if (
                    _workerScheduler.MarkAvailable(
                        contextIndex
                    ) == WorkerAvailabilityResult::Available
                ) {
                    ScheduleAvailableWorkers();
                }

                ReleaseLock();
            }


            // Task handle binding.

            /// Returns the immutable operation table shared by every Task handle owned by this facility.
            static const TaskHandleOperations& HandleOperations() noexcept {
                static const TaskHandleOperations operations {
                    &StateThunk,
                    &WaitThunk,
                    &WaitForThunk,
                    &WaitUntilThunk,
                    &CancelThunk,
                    &ReleaseThunk,
                    &TakeResultThunk
                };

                return operations;
            }


            // Task observation.

            /// Returns one Task incarnation's public lifecycle state.
            TaskState State(
                Index recordIndex,
                bool phase
            ) const noexcept {
                return _core.PublicState(
                    recordIndex,
                    phase
                );
            }


            // Task waiting.

            /// Waits indefinitely for one Task incarnation to become terminal.
            TaskWaitResult Wait(
                Index recordIndex,
                bool phase
            ) {
                return WaitWithBudget(
                    recordIndex,
                    phase,
                    MonotonicWaitBudget::Forever()
                );
            }

            /// Waits for one Task incarnation using a single relative canonical monotonic budget.
            TaskWaitResult WaitFor(
                Index recordIndex,
                bool phase,
                Duration duration
            ) {
                return WaitWithBudget(
                    recordIndex,
                    phase,
                    MonotonicWaitBudget::For(
                        duration
                    )
                );
            }

            /// Waits for one Task incarnation until one canonical monotonic deadline.
            TaskWaitResult WaitUntil(
                Index recordIndex,
                bool phase,
                MonotonicTimestamp deadline
            ) {
                return WaitWithBudget(
                    recordIndex,
                    phase,
                    MonotonicWaitBudget::Until(
                        deadline
                    )
                );
            }


            // Task cancellation.

            /// Requests queued cancellation or cooperative running cancellation.
            TaskCancelResult Cancel(
                Index recordIndex,
                bool phase
            ) noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return TaskCancelResult::AlreadyFinished;
                }

                const auto runningContext = _core.ExecutionContextFor(
                    recordIndex,
                    phase
                );

                const auto result = _core.Cancel(
                    recordIndex,
                    phase
                );

                if (result.IsTerminalPublished()) {
                    WakeMatchingWaiters(
                        recordIndex,
                        phase
                    );
                } else if (
                    result.Result() == TaskCancelResult::Accepted &&
                    runningContext.has_value()
                ) {
                    static_cast<void>(
                        _router->Wake(
                            runningContext.value()
                        )
                    );
                }

                ReclaimIfQuiescent(
                    recordIndex,
                    phase
                );

                ReleaseLock();
                return result.Result();
            }


            // Public ownership release.

            /// Releases one Task handle's sole public ownership interest without blocking.
            void ReleaseOwner(
                Index recordIndex,
                bool phase
            ) noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return;
                }

                const auto effect = _core.ReleaseOwner(
                    recordIndex,
                    phase
                );

                if (effect == TaskReleaseEffect::TerminalPublished) {
                    WakeMatchingWaiters(
                        recordIndex,
                        phase
                    );
                }

                ReclaimIfQuiescent(
                    recordIndex,
                    phase
                );

                ReleaseLock();
            }


            // Result extraction.

            /// Moves one completed result to caller storage and consumes the public ownership interest.
            TaskTakeStatus TakeResult(
                Index recordIndex,
                bool phase,
                void* destination
            ) {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return TaskTakeStatus::NotCompleted;
                }

                const auto result = _core.TakeResult(
                    recordIndex,
                    phase,
                    destination
                );

                if (result == TaskTakeStatus::Succeeded) {
                    ReclaimIfQuiescent(
                        recordIndex,
                        phase
                    );
                }

                ReleaseLock();
                return result;
            }


            // Managed-context interruption discovery.

            /// Indicates whether one Worker context is executing a Task with cancellation requested.
            bool IsCancellationRequestedForContext(
                ContextIndex contextIndex
            ) noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return true;
                }

                const auto result = _core.IsCancellationRequestedForContext(
                    contextIndex
                );

                ReleaseLock();
                return result;
            }


            // Shutdown cooperation.

            void BeginShutdownCancellation() noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return;
                }

                _core.VisitAllocated(
                    [this](const auto& binding) {
                        const auto before = _core.PublicState(binding.RecordIndex, binding.Phase);
                        const auto cancellation = _core.Cancel(binding.RecordIndex, binding.Phase);

                        if (cancellation.Result() != TaskCancelResult::Accepted) {
                            return;
                        }

                        const auto after = _core.PublicState(binding.RecordIndex, binding.Phase);

                        if (before == TaskState::Queued && after == TaskState::Cancelled) {
                            WakeMatchingWaiters(binding.RecordIndex, binding.Phase);
                            return;
                        }

                        if (after == TaskState::Running) {
                            const auto contextIndex = _core.ExecutionContextFor(
                                binding.RecordIndex,
                                binding.Phase
                            );

                            if (contextIndex.has_value()) {
                                static_cast<void>(_router->Wake(contextIndex.value()));
                            }
                        }
                    }
                );

                WakeAdmissionWaiters();
                ReleaseLock();
            }

            bool IsExecutionQuiescent() noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return false;
                }

                const auto result = !_core.HasExecutionWork();
                ReleaseLock();
                return result;
            }


            // Bounded observability.

            /// Returns the configured Task-record capacity.
            static constexpr std::size_t RecordCapacity() noexcept {
                return Core::RecordCapacity();
            }

            /// Returns the configured Worker capacity.
            static constexpr std::size_t WorkerCapacity() noexcept {
                return TWorkerCount;
            }

            /// Returns the current number of Workers granted to Tasks.
            std::size_t WorkersInUse() noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return TWorkerCount;
                }

                const auto result = _workerScheduler.InUseCount();
                ReleaseLock();
                return result;
            }

            /// Returns the current number of structurally allocated Task records.
            std::size_t RecordsInUse() noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return TRecordCapacity;
                }

                const auto result = _core.RecordsInUse();
                ReleaseLock();
                return result;
            }

            /// Returns the current number of queued Tasks.
            std::size_t QueuedTasks() noexcept {
                if (AcquireLock() != TaskFacilityLockResult::Acquired) {
                    return TRecordCapacity;
                }

                const auto result = _core.QueuedTasks();
                ReleaseLock();
                return result;
            }

    };

} // ESPressio::Threading::Detail
