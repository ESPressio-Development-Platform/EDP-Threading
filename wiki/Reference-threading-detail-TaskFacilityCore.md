# src/threading/detail/TaskFacilityCore.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/TaskFacilityCore.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `new`
- `optional`
- `type_traits`
- `utility`
- `../ThreadingTypes.hpp`
- `CallableTraits.hpp`
- `FacilityStorage.hpp`
- `TaskPayloadAdapter.hpp`
- `TaskRecord.hpp`

## Documented declarations

### `TIndex`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskRecordBinding`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<class TIndex>
    struct TaskRecordBinding final
```

### `RecordIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Bound Task record index.

```cpp
TIndex RecordIndex;
```

### `Phase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Bound record incarnation Phase.

```cpp
bool Phase;
```

### `TIndex`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskAdmissionCoreResult`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<class TIndex>
    class TaskAdmissionCoreResult final
```

### `_status`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Operational admission status.

```cpp
TaskAdmissionCoreStatus _status;
```

### `_binding`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Admitted record binding, present only after successful admission.

```cpp
std::optional<TaskRecordBinding<TIndex>> _binding;
```

### `TaskAdmissionCoreResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates one normalized admission outcome.

```cpp
TaskAdmissionCoreResult(
                TaskAdmissionCoreStatus status,
                std::optional<TaskRecordBinding<TIndex>> binding
            ) noexcept :
                _status(status),
```

### `CapacityUnavailable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates a capacity-unavailable admission outcome.

```cpp
static TaskAdmissionCoreResult CapacityUnavailable() noexcept
```

### `Admitted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates a successful admission outcome.

```cpp
static TaskAdmissionCoreResult Admitted(
                TIndex recordIndex,
                bool phase
            ) noexcept
```

### `Status`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the operational admission status.

```cpp
TaskAdmissionCoreStatus Status() const noexcept
```

### `IsAdmitted`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether a record was admitted.

```cpp
bool IsAdmitted() const noexcept
```

### `Binding`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the admitted record binding when admission succeeded.

```cpp
const std::optional<TaskRecordBinding<TIndex>>& Binding() const noexcept
```

### `TIndex`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskWorkerClaimResult`.
- **Template parameter `TIndex`:** Compile-time resource or tuple index used by recursive traversal.

```cpp
template<class TIndex>
    class TaskWorkerClaimResult final
```

### `_status`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Operational Worker claim status.

```cpp
TaskWorkerClaimStatus _status;
```

### `_binding`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Claimed record binding, present only after successful claim.

```cpp
std::optional<TaskRecordBinding<TIndex>> _binding;
```

### `TaskWorkerClaimResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates one normalized Worker claim outcome.

```cpp
TaskWorkerClaimResult(
                TaskWorkerClaimStatus status,
                std::optional<TaskRecordBinding<TIndex>> binding
            ) noexcept :
                _status(status),
```

### `QueueEmpty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates an empty-queue Worker claim outcome.

```cpp
static TaskWorkerClaimResult QueueEmpty() noexcept
```

### `Claimed`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates a successful Worker claim outcome.

```cpp
static TaskWorkerClaimResult Claimed(
                TIndex recordIndex,
                bool phase
            ) noexcept
```

### `Status`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the Worker claim status.

```cpp
TaskWorkerClaimStatus Status() const noexcept
```

### `IsClaimed`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether one queued record was claimed for execution.

```cpp
bool IsClaimed() const noexcept
```

### `Binding`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the claimed record binding when Worker claim succeeded.

```cpp
const std::optional<TaskRecordBinding<TIndex>>& Binding() const noexcept
```

### `_result`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Public cancellation result returned to the Task owner.

```cpp
TaskCancelResult _result;
```

### `_terminalPublished`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether this operation newly published terminal Cancelled state.

```cpp
bool _terminalPublished;
```

### `TaskCancellationCoreResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates one cancellation outcome.

```cpp
TaskCancellationCoreResult(
                TaskCancelResult result,
                bool terminalPublished
            ) noexcept :
                _result(result),
```

### `Result`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the public Task cancellation result.

```cpp
TaskCancelResult Result() const noexcept
```

### `IsTerminalPublished`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this operation newly published terminal state.

```cpp
bool IsTerminalPublished() const noexcept
```

### `TaskFacilityCore`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskFacilityCore`.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type or bounded capacity.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type or byte capacity.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type or byte capacity.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<std::size_t TRecordCapacity, std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TExecutionContextCapacity>
    class TaskFacilityCore final
```

### `RecordType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Concrete bounded Task record owned by this facility.

```cpp
using RecordType = TaskRecord<
                TCallableCapacity,
```

### `IndexType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Smallest record-index Type satisfying the configured facility capacity.

```cpp
using IndexType = typename RecordType::Index;
```

### `ExecutionContextIndexType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Smallest managed execution-context index Type satisfying the topology capacity.

```cpp
using ExecutionContextIndexType = typename RecordType::ExecutionContextIndex;
```

### `RecordType _records[TRecordCapacity];`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Statically provisioned Task records.

```cpp
RecordType _records[TRecordCapacity];
```

### `_availability`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Structural free-record publication bitmap.

```cpp
AvailabilityBitmap<TRecordCapacity> _availability;
```

### `_queue`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Intrusive FIFO over Queued record indices.

```cpp
IntrusiveTaskQueue<TRecordCapacity> _queue;
```

### `IsCurrentIncarnation`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether the supplied index and Phase identify the same record incarnation.

The owning facility runtime must serialize this check with every control mutation.

```cpp
bool IsCurrentIncarnation(
                IndexType recordIndex,
                bool phase
            ) const noexcept
```

### `Record`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Concrete bounded Task record owned by this facility.

```cpp
using Record = RecordType;
```

### `Index`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Smallest record-index Type satisfying the configured facility capacity.

```cpp
using Index = IndexType;
```

### `AdmissionResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Structured admission outcome for this facility.

```cpp
using AdmissionResult = TaskAdmissionCoreResult<Index>;
```

### `WorkerClaimResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Structured Worker-claim outcome for this facility.

```cpp
using WorkerClaimResult = TaskWorkerClaimResult<Index>;
```

### `ExecutionContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Dense managed execution-context index Type used for targeted wake routing.

```cpp
using ExecutionContextIndex = ExecutionContextIndexType;
```

### `HasRecordCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether structural Task-record capacity is currently available.

The owning facility runtime must serialize this query with admission/reclamation.

```cpp
bool HasRecordCapacity() const noexcept
```

### `TCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Moves one callable into a newly admitted bounded Task record and appends it to FIFO order.

The owning facility runtime must serialize this operation with queue mutation,
cancellation, Worker claim, owner release and reclamation.
- **Template parameter `TCallable`:** Callable Type being dispatched or adapted.

```cpp
template<class TCallable>
            AdmissionResult Admit(
                TCallable&& callable
            ) noexcept
```

### `StoredCallable`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Decayed callable Type retained in bounded Task payload storage.

```cpp
using StoredCallable = std::decay_t<TCallable>;
```

### `Result`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Result Type produced by the stored callable.

```cpp
using Result = CallableResultT<StoredCallable>;
```

### `Adapter`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Payload adapter Type responsible for callable invocation and result publication.

```cpp
using Adapter = TaskPayloadAdapter<
                    Record,
```

### `ClaimNextForWorker`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Claims the oldest queued Task for Worker execution.

The owning facility runtime must serialize this operation with queue mutation.

```cpp
WorkerClaimResult ClaimNextForWorker(
                ExecutionContextIndex contextIndex
            ) noexcept
```

### `AssignedTaskForContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the Task binding currently granted to one managed Worker context.

```cpp
std::optional<TaskRecordBinding<Index>> AssignedTaskForContext(
                ExecutionContextIndex contextIndex
            ) const noexcept
```

### `Invoke`

**Classification:** PRIVATE IMPLEMENTATION

Executes one Worker-owned Task payload outside the facility serialization boundary.

```cpp
TaskInvocationOutcome Invoke(
                Index recordIndex,
                TaskContext& context
            )
```

### `PublishInvocationOutcome`

**Classification:** PRIVATE IMPLEMENTATION

Publishes one Worker invocation outcome after result/callable lifetime transition is complete.

The owning facility runtime must serialize this operation with Task control operations
and must wake matching waiters after this terminal publication.

```cpp
void PublishInvocationOutcome(
                Index recordIndex,
                bool phase,
                TaskInvocationOutcome outcome
            ) noexcept
```

### `PublicState`

**Classification:** PRIVATE IMPLEMENTATION

Returns the public lifecycle state for one current Task incarnation.

```cpp
TaskState PublicState(
                Index recordIndex,
                bool phase
            ) const noexcept
```

### `IsTerminal`

**Classification:** PRIVATE IMPLEMENTATION

Indicates whether the identified Task incarnation has reached terminal lifecycle state.

```cpp
bool IsTerminal(
                Index recordIndex,
                bool phase
            ) const noexcept
```

### `HasWorkerGrant`

**Classification:** PRIVATE IMPLEMENTATION

Indicates whether one Task incarnation has ever won a Worker Lease.

Queued cancellation/abandonment normalizes scratch to the invalid context sentinel,
while every granted Task retains its valid executing context index through terminal
publication. This lets dispatch timeout distinguish "never granted" Cancelled from
"grant already won, then later Cancelled" without another control bit.

```cpp
bool HasWorkerGrant(
                Index recordIndex,
                bool phase
            ) const noexcept
```

### `Cancel`

**Classification:** PRIVATE IMPLEMENTATION

Applies cooperative cancellation semantics to one Task owner request.

The owning facility runtime must serialize this operation with Worker claim,
terminal publication and queue mutation.

```cpp
TaskCancellationCoreResult Cancel(
                Index recordIndex,
                bool phase
            ) noexcept
```

### `ReleaseOwner`

**Classification:** PRIVATE IMPLEMENTATION

Releases the sole public Task ownership interest and performs any required payload cleanup.

The owning facility runtime must serialize this operation. A TerminalPublished effect
means queued abandonment published Cancelled and matching waiters must be woken.

```cpp
TaskReleaseEffect ReleaseOwner(
                Index recordIndex,
                bool phase
            ) noexcept
```

### `TakeResult`

**Classification:** PRIVATE IMPLEMENTATION

Moves a completed result to caller-provided typed storage and consumes public ownership.

```cpp
TaskTakeStatus TakeResult(
                Index recordIndex,
                bool phase,
                void* destination
            )
```

### `IsOwnerlessTerminal`

**Classification:** PRIVATE IMPLEMENTATION

Indicates whether terminal record state is free of the public ownership interest.

The owning facility runtime must additionally prove that no waiter registration or
in-flight Worker/internal interest remains before calling Reclaim.

```cpp
bool IsOwnerlessTerminal(
                Index recordIndex,
                bool phase
            ) const noexcept
```

### `Reclaim`

**Classification:** PRIVATE IMPLEMENTATION

Republishes one fully quiescent terminal record as structurally available.

This method checks the intrinsic owner/terminal preconditions only. The owning
facility runtime must prove absence of matching waiter/internal interests.

```cpp
TaskReclaimResult Reclaim(
                Index recordIndex,
                bool phase
            ) noexcept
```

### `TVisitor`

**Classification:** PRIVATE IMPLEMENTATION

Visits every structurally allocated Task incarnation while the owning facility lock is held.
- **Template parameter `TVisitor`:** Visitor callable Type invoked for each selected bounded record.

```cpp
template<class TVisitor>
            void VisitAllocated(
                TVisitor&& visitor
            )
```

### `RecordCapacity`

**Classification:** PRIVATE IMPLEMENTATION

Returns the configured Task-record capacity.

```cpp
static constexpr std::size_t RecordCapacity() noexcept
```

### `RecordsInUse`

**Classification:** PRIVATE IMPLEMENTATION

Returns the number of currently structurally allocated Task records.

The owning facility runtime must serialize this scan with admission/reclamation.

```cpp
std::size_t RecordsInUse() const noexcept
```

### `HasExecutionWork`

**Classification:** PRIVATE IMPLEMENTATION

Indicates whether any admitted Task still has execution work outstanding.

Completed/Cancelled records retained solely by public ownership or waiters do not
keep Worker infrastructure alive during terminal shutdown.

```cpp
bool HasExecutionWork() const noexcept
```

### `QueuedTasks`

**Classification:** PRIVATE IMPLEMENTATION

Returns the number of currently queued Tasks.

The owning facility runtime must serialize this traversal with queue mutation.

```cpp
std::size_t QueuedTasks() const noexcept
```

### `IsCancellationRequested`

**Classification:** PRIVATE IMPLEMENTATION

Reads cancellation for one current Task incarnation.

The owning facility runtime must serialize this read with every control mutation.

```cpp
bool IsCancellationRequested(
                Index recordIndex,
                bool phase
            ) const noexcept
```

### `ExecutionContextFor`

**Classification:** PRIVATE IMPLEMENTATION

Returns the Worker execution-context index for one Running Task incarnation.

```cpp
std::optional<ExecutionContextIndex> ExecutionContextFor(
                Index recordIndex,
                bool phase
            ) const noexcept
```

### `IsCancellationRequestedForContext`

**Classification:** PRIVATE IMPLEMENTATION

Indicates whether the supplied managed Worker context is executing a cancellation-requested Task.

The owning facility runtime must serialize this bounded scan with Worker claim and
terminal publication because the scratch field is non-atomic and lifecycle-reused.

```cpp
bool IsCancellationRequestedForContext(
                ExecutionContextIndex contextIndex
            ) const noexcept
```

