# src/threading/detail/TaskFacilityRuntime.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/TaskFacilityRuntime.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `optional`
- `utility`
- `ESPressio_Platform.hpp`
- `../Task.hpp`
- `MonotonicWaitBudget.hpp`
- `TaskFacilityCore.hpp`
- `WaitRegistration.hpp`
- `WorkerLeaseScheduler.hpp`

## Documented declarations

### `TMutexProvider`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskFacilityRuntime`.
- **Template parameter `TMutexProvider`:** Concrete Platform Mutex provider Type protecting resource-local state.
- **Template parameter `TManagedContextRouter`:** Managed-context router Type used for identity, interruption, and targeted wakes.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type or bounded capacity.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type or byte capacity.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type or byte capacity.
- **Template parameter `TWorkerCount`:** Compile-time template parameter `TWorkerCount` used by this declaration.
- **Template parameter `TFirstWorkerContextIndex`:** Compile-time template parameter `TFirstWorkerContextIndex` used by this declaration.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<std::size_t TRecordCapacity, std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TWorkerCount, std::size_t TFirstWorkerContextIndex, std::size_t TExecutionContextCapacity, class TMutexProvider, class TManagedContextRouter>
    class TaskFacilityRuntime final
```

### `Core`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Deterministic Task record/FIFO state core.

```cpp
using Core = TaskFacilityCore<
                TRecordCapacity,
```

### `Index`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Compact record index selected by the deterministic core.

```cpp
using Index = typename Core::Index;
```

### `ContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Dense managed execution-context index used by this topology.

```cpp
using ContextIndex = typename Core::ExecutionContextIndex;
```

### `WaitRegistration`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Target-owned Task waiter registration.

```cpp
using WaitRegistration = TaskWaitRegistration<
                Index,
```

### `Waiters`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bounded Task waiter registrations sufficient for every managed sequential context.

```cpp
using Waiters = RegistrationSet<
                WaitRegistration,
```

### `AdmissionWaitRegistrationType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Facility-level pre-admission capacity waiter registration.

```cpp
using AdmissionWaitRegistrationType = AdmissionWaitRegistration<
                ContextIndex
            >;
```

### `AdmissionWaiters`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bounded pre-admission waiters; no Task record exists while one is active.

```cpp
using AdmissionWaiters = RegistrationSet<
                AdmissionWaitRegistrationType,
```

### `WorkerScheduler`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bounded availability scheduler for this facility's statically configured Workers.

```cpp
using WorkerScheduler = WorkerLeaseScheduler<
                TWorkerCount,
```

### `_core`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Deterministic record/FIFO/lifecycle state.

```cpp
Core _core;
```

### `_mutex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Facility-local serialization primitive.

```cpp
mutable TMutexProvider _mutex;
```

### `_waiters`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Target-owned bounded Task waiter registrations.

```cpp
Waiters _waiters;
```

### `_admissionWaiters`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Facility-owned bounded pre-admission waiter registrations.

```cpp
AdmissionWaiters _admissionWaiters;
```

### `_workerScheduler`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Facility-owned Worker availability state.

```cpp
WorkerScheduler _workerScheduler;
```

### `_router`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Non-owning topology-stable managed-context router.

```cpp
TManagedContextRouter* _router;
```

### `Facility`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Facility whose authoritative record state is observed by the running callable.

```cpp
TaskFacilityRuntime* Facility;
```

### `RecordIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Record slot currently executing through this invocation-local context.

```cpp
Index RecordIndex;
```

### `Phase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Incarnation Phase identifying the admitted Task within the record slot.

```cpp
bool Phase;
```

### `IsCancellationRequestedThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resolves cooperative cancellation for TaskContext through the invocation-local observation record.

```cpp
static bool IsCancellationRequestedThunk(
                const void* context
            ) noexcept
```

### `AcquireLock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Acquires the facility serialization boundary indefinitely.

```cpp
TaskFacilityLockResult AcquireLock() const noexcept
```

### `ReleaseLock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Releases the facility serialization boundary.

```cpp
ESPressio::Platform::Synchronization::LockReleaseResult ReleaseLock() const noexcept
```

### `IsCancellationRequested`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Reads cooperative cancellation through the authoritative facility serialization boundary.

```cpp
bool IsCancellationRequested(
                Index recordIndex,
                bool phase
            ) noexcept
```

### `Matches`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether one Task waiter targets the supplied record incarnation.

```cpp
static bool Matches(
                const WaitRegistration& registration,
                Index recordIndex,
                bool phase
            ) noexcept
```

### `MatchingWaiterCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Returns the number of active waiters targeting one Task incarnation.

```cpp
std::size_t MatchingWaiterCount(
                Index recordIndex,
                bool phase
            ) const
```

### `WakeMatchingWaiters`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Wakes every managed context waiting on one terminal Task incarnation.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult WakeMatchingWaiters(
                Index recordIndex,
                bool phase
            )
```

### `WakeAdmissionWaiters`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Wakes every managed context currently blocked for Task-record admission capacity.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult WakeAdmissionWaiters()
```

### `ScheduleAvailableWorkers`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Grants queued Tasks to currently available Workers in FIFO opportunity order.

The facility lock must be held throughout this scheduling pass. A sleeping Worker is
successfully woken before its Task grant is published, while the currently executing
Worker can accept a grant directly because it does not require a wake.

```cpp
ESPressio::Platform::Synchronization::SignalNotifyResult ScheduleAvailableWorkers()
```

### `ReclaimIfQuiescent`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Reclaims one ownerless terminal record when no waiter still retains its incarnation.

```cpp
TaskReclaimResult ReclaimIfQuiescent(
                Index recordIndex,
                bool phase
            ) noexcept
```

### `UnregisterWaiter`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Removes one wait registration and applies final waiter-dependent reclamation.

```cpp
TaskReclaimResult UnregisterWaiter(
                std::size_t registrationIndex,
                Index recordIndex,
                bool phase
            ) noexcept
```

### `WaitWithBudget`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Waits for one Task incarnation according to a canonical non-restarting wait budget.

```cpp
TaskWaitResult WaitWithBudget(
                Index recordIndex,
                bool phase,
                const MonotonicWaitBudget& budget
            )
```

### `StateThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Reads Task state through the type-erased public handle operation table.

```cpp
static TaskState StateThunk(
                const void* owner,
                std::uint32_t recordIndex,
                bool phase
            ) noexcept
```

### `WaitThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Performs indefinite Task wait through the public handle operation table.

```cpp
static TaskWaitResult WaitThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase
            )
```

### `WaitForThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Performs relative Task wait through the public handle operation table.

```cpp
static TaskWaitResult WaitForThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase,
                Duration duration
            )
```

### `WaitUntilThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Performs deadline Task wait through the public handle operation table.

```cpp
static TaskWaitResult WaitUntilThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase,
                MonotonicTimestamp deadline
            )
```

### `CancelThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Performs Task cancellation through the public handle operation table.

```cpp
static TaskCancelResult CancelThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase
            ) noexcept
```

### `ReleaseThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Releases public Task ownership through the public handle operation table.

```cpp
static void ReleaseThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase
            ) noexcept
```

### `TakeResultThunk`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Performs consuming Task result extraction through the public handle operation table.

```cpp
static TaskTakeStatus TakeResultThunk(
                void* owner,
                std::uint32_t recordIndex,
                bool phase,
                void* destination
            )
```

### `WaitForAdmissionCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Waits for bounded Task-record admission capacity using the caller's original monotonic budget.

```cpp
TaskDispatchStatus WaitForAdmissionCapacity(
                ContextIndex contextIndex,
                const MonotonicWaitBudget& budget
            )
```

### `WithdrawUnreturnedDispatch`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Releases an admitted Task that cannot be returned to the caller and reclaims it when immediately eligible.

```cpp
TaskReclaimResult WithdrawUnreturnedDispatch(
                Index recordIndex,
                bool phase
            ) noexcept
```

### `WaitForWorkerGrant`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Waits for an admitted Task to receive a Worker grant without restarting its dispatch timeout budget.

```cpp
TaskDispatchStatus WaitForWorkerGrant(
                ContextIndex contextIndex,
                Index recordIndex,
                bool phase,
                const MonotonicWaitBudget& budget
            )
```

### `RecordIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Compact Task record index Type.

```cpp
using RecordIndex = Index;
```

### `ManagedContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Dense managed execution-context index Type.

```cpp
using ManagedContextIndex = ContextIndex;
```

### `AdmissionResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Structured deterministic admission result.

```cpp
using AdmissionResult = typename Core::AdmissionResult;
```

### `WorkerClaimResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Structured deterministic Worker claim result.

```cpp
using WorkerClaimResult = typename Core::WorkerClaimResult;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Public Task handle Type produced for one callable Type.
- **Template parameter `TCallable`:** Callable Type whose result determines the concrete Task handle Type.

```cpp
template<class TCallable>
            using TaskForCallable = Task<
                CallableResultT<std::decay_t<TCallable>>
            >;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Structured public dispatch result produced for one callable Type.
- **Template parameter `TCallable`:** Callable Type whose Task handle is carried by a successful dispatch result.

```cpp
template<class TCallable>
            using DispatchResultFor = TaskDispatchResult<
                TaskForCallable<TCallable>
            >;
```

### `TaskFacilityRuntime`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Binds this static Task facility to its topology-owned managed-context router.

```cpp
explicit TaskFacilityRuntime(
                TManagedContextRouter& router
            ) noexcept :
                _router(&router) {}
```

### `ValidateSynchronization`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Validates that the facility mutex provider can acquire and release before execution starts.

```cpp
TaskFacilitySynchronizationResult ValidateSynchronization() noexcept
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Dispatches one callable according to the selected admission policy and optional monotonic timeout.
- **Template parameter `TCallable`:** Callable Type admitted into bounded facility storage.

```cpp
template<class TCallable>
            DispatchResultFor<TCallable> Dispatch(
                TCallable&& callable,
                TaskDispatchPolicy policy,
                Duration timeout = Duration{}
            )
```

### `TaskType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Task handle Type instantiated for the callable being dispatched.

```cpp
using TaskType = TaskForCallable<TCallable>;
```

### `DispatchResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Typed dispatch result returned for the callable being dispatched.

```cpp
using DispatchResult = DispatchResultFor<TCallable>;
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Admits one callable into deterministic facility storage when record capacity is available.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.

```cpp
template<class TCallable>
            AdmissionResult Admit(
                TCallable&& callable
            ) noexcept
```

### `WorkerBecameAvailable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Publishes one infrastructure-started Worker as available and grants queued work when present.

```cpp
WorkerAvailabilityResult WorkerBecameAvailable(
                ContextIndex contextIndex
            ) noexcept
```

### `AssignedTaskForContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the Task currently granted to one Worker context, when one exists.

```cpp
std::optional<TaskRecordBinding<Index>> AssignedTaskForContext(
                ContextIndex contextIndex
            ) noexcept
```

### `Invoke`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Invokes one Worker-owned Task payload outside the facility lock.

```cpp
TaskInvocationOutcome Invoke(
                Index recordIndex,
                bool phase
            )
```

### `CompleteWorkerTask`

**Classification:** PRIVATE IMPLEMENTATION

Publishes one Worker outcome, releases that Worker Lease, and atomically schedules FIFO work.

```cpp
TaskFacilityCompletionResult CompleteWorkerTask(
                ContextIndex contextIndex,
                Index recordIndex,
                bool phase,
                TaskInvocationOutcome outcome
            ) noexcept
```

### `HandleOperations`

**Classification:** PRIVATE IMPLEMENTATION

Returns the immutable operation table shared by every Task handle owned by this facility.

```cpp
static const TaskHandleOperations& HandleOperations() noexcept
```

### `State`

**Classification:** PRIVATE IMPLEMENTATION

Returns one Task incarnation's public lifecycle state.

```cpp
TaskState State(
                Index recordIndex,
                bool phase
            ) const noexcept
```

### `Wait`

**Classification:** PRIVATE IMPLEMENTATION

Waits indefinitely for one Task incarnation to become terminal.

```cpp
TaskWaitResult Wait(
                Index recordIndex,
                bool phase
            )
```

### `WaitFor`

**Classification:** PRIVATE IMPLEMENTATION

Waits for one Task incarnation using a single relative canonical monotonic budget.

```cpp
TaskWaitResult WaitFor(
                Index recordIndex,
                bool phase,
                Duration duration
            )
```

### `WaitUntil`

**Classification:** PRIVATE IMPLEMENTATION

Waits for one Task incarnation until one canonical monotonic deadline.

```cpp
TaskWaitResult WaitUntil(
                Index recordIndex,
                bool phase,
                MonotonicTimestamp deadline
            )
```

### `Cancel`

**Classification:** PRIVATE IMPLEMENTATION

Requests queued cancellation or cooperative running cancellation.

```cpp
TaskCancelResult Cancel(
                Index recordIndex,
                bool phase
            ) noexcept
```

### `ReleaseOwner`

**Classification:** PRIVATE IMPLEMENTATION

Releases one Task handle's sole public ownership interest without blocking.

```cpp
TaskFacilityOwnerReleaseResult ReleaseOwner(
                Index recordIndex,
                bool phase
            ) noexcept
```

### `TakeResult`

**Classification:** PRIVATE IMPLEMENTATION

Moves one completed result to caller storage and consumes the public ownership interest.

```cpp
TaskTakeStatus TakeResult(
                Index recordIndex,
                bool phase,
                void* destination
            )
```

### `IsCancellationRequestedForContext`

**Classification:** PRIVATE IMPLEMENTATION

Indicates whether one Worker context is executing a Task with cancellation requested.

```cpp
bool IsCancellationRequestedForContext(
                ContextIndex contextIndex
            ) noexcept
```

### `BeginShutdownCancellation`

**Classification:** PRIVATE IMPLEMENTATION

Cancels queued Tasks, requests cooperative cancellation of running Tasks, and wakes affected waiters/contexts.

```cpp
TaskFacilityShutdownCancellationResult BeginShutdownCancellation() noexcept
```

### `IsExecutionQuiescent`

**Classification:** PRIVATE IMPLEMENTATION

Indicates whether this facility has no queued or actively executing Task work.

```cpp
bool IsExecutionQuiescent() noexcept
```

### `RecordCapacity`

**Classification:** PRIVATE IMPLEMENTATION

Returns the configured Task-record capacity.

```cpp
static constexpr std::size_t RecordCapacity() noexcept
```

### `WorkerCapacity`

**Classification:** PRIVATE IMPLEMENTATION

Returns the configured Worker capacity.

```cpp
static constexpr std::size_t WorkerCapacity() noexcept
```

### `WorkersInUse`

**Classification:** PRIVATE IMPLEMENTATION

Returns the current number of Workers granted to Tasks.

```cpp
std::size_t WorkersInUse() noexcept
```

### `RecordsInUse`

**Classification:** PRIVATE IMPLEMENTATION

Returns the current number of structurally allocated Task records.

```cpp
std::size_t RecordsInUse() noexcept
```

### `QueuedTasks`

**Classification:** PRIVATE IMPLEMENTATION

Returns the current number of queued Tasks.

```cpp
std::size_t QueuedTasks() noexcept
```

