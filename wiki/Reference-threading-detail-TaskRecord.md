# src/threading/detail/TaskRecord.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `a13bc9a96ed4d371456b914741dd1a93a68357db`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/a13bc9a96ed4d371456b914741dd1a93a68357db/src/threading/detail/TaskRecord.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `../ThreadingTypes.hpp`
- `TopologyIndex.hpp`

## Documented declarations

### `TaskControlTransitionResult`

**Classification:** PRIVATE IMPLEMENTATION

Outcome of an internal Task control-state transition.

```cpp
enum class TaskControlTransitionResult : std::uint8_t
```

### `_value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Packed operational state, public ownership and incarnation Phase.

```cpp
std::uint8_t _value;
```

### `StateMask`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bit mask selecting the internal Task operational state.

```cpp
static constexpr std::uint8_t StateMask = 0x07U;
```

### `OwnerMask`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bit mask selecting the sole public Task ownership interest.

```cpp
static constexpr std::uint8_t OwnerMask = 0x08U;
```

### `PhaseMask`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bit mask selecting the record-incarnation Phase.

```cpp
static constexpr std::uint8_t PhaseMask = 0x10U;
```

### `State`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the internal operational state.

```cpp
TaskOperationalState State() const noexcept
```

### `HasOwner`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether the sole public Task ownership interest exists.

```cpp
bool HasOwner() const noexcept
```

### `Phase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the current one-bit record-incarnation Phase.

```cpp
bool Phase() const noexcept
```

### `IsCancellationRequested`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether cooperative cancellation has been requested while Running.

```cpp
bool IsCancellationRequested() const noexcept
```

### `InitializeQueued`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Initializes a newly claimed record and toggles its incarnation Phase.

```cpp
void InitializeQueued() noexcept
```

### `SetState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Replaces only the operational state while preserving owner and Phase bits.

```cpp
void SetState(
                TaskOperationalState state
            ) noexcept
```

### `CompareExchangeState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Changes the operational state only when the expected state still owns the transition.

The owning facility runtime serializes every mutation through its mutex.

```cpp
TaskControlTransitionResult CompareExchangeState(
                TaskOperationalState expectedState,
                TaskOperationalState desiredState
            ) noexcept
```

### `ReleaseOwner`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Releases the sole public ownership interest.

The owning facility runtime serializes every mutation through its mutex.

```cpp
void ReleaseOwner() noexcept
```

### `TTaskRecord`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskPayloadOperations`.
- **Template parameter `TTaskRecord`:** Task-record Type inspected by the helper.

```cpp
template<class TTaskRecord>
    struct TaskPayloadOperations final
```

### `TaskInvocationOutcome`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Executes the callable and establishes any result payload without publishing terminal lifecycle state.

```cpp
TaskInvocationOutcome (*Invoke)(TTaskRecord&, TaskContext&);
```

### `void`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys the callable payload before execution has consumed it.

```cpp
void (*DestroyCallable)(TTaskRecord&) noexcept;
```

### `void`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Destroys a completed result payload when one exists.

```cpp
void (*DestroyResult)(TTaskRecord&) noexcept;
```

### `void`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Moves a completed result into caller-provided typed storage.

```cpp
void (*MoveResult)(
            TTaskRecord&,
            void*
        );
```

### `TaskRecord`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskRecord`.
- **Template parameter `TCallableCapacity`:** Callable-storage capacity declaration Type.
- **Template parameter `TResultCapacity`:** Result-storage capacity declaration Type.
- **Template parameter `TRecordCapacity`:** Task-record capacity declaration Type.
- **Template parameter `TExecutionContextCapacity`:** Total managed execution-context capacity of the topology.

```cpp
template<std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TRecordCapacity, std::size_t TExecutionContextCapacity>
    struct TaskRecord final
```

### `ScratchCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Largest index capacity needed by the mutually exclusive queue/Worker scratch field.

```cpp
static constexpr std::size_t ScratchCapacity =
            TRecordCapacity > TExecutionContextCapacity
                ? TRecordCapacity
                : TExecutionContextCapacity;
```

### `PayloadCapacity`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Payload bytes shared by callable storage and result storage.

```cpp
static constexpr std::size_t PayloadCapacity =
            TCallableCapacity > TResultCapacity
                ? TCallableCapacity
                : TResultCapacity;
```

### `QueueIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Strong bounded Task-record identity used only at the shared availability/queue topology boundary. It does not replace the lifecycle-reused raw scratch scalar.

```cpp
using QueueIndex = typename TopologyIndexTraits<
            TaskRecordIndexSpace,
            TRecordCapacity
        >::Strong;
```

### `Index`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Smallest raw Task-record index Type selected by the shared bounded-index representation for existing Threading lifecycle APIs.

```cpp
using Index = typename TopologyIndexTraits<
            TaskRecordIndexSpace,
            TRecordCapacity
        >::Storage;
```

### `ExecutionContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Smallest managed execution-context index Type satisfying the complete topology capacity.

```cpp
using ExecutionContextIndex = typename TopologyIndexTraits<
            ManagedContextIndexSpace,
            TExecutionContextCapacity
        >::Storage;
```

### `InvalidExecutionContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Sentinel which cannot identify a valid managed execution context.

```cpp
static constexpr ExecutionContextIndex InvalidExecutionContextIndex =
            TopologyIndexTraits<
                ManagedContextIndexSpace,
                TExecutionContextCapacity
            >::Invalid;
```

### `ScratchIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Smallest scratch Type able to hold either a queue link or an execution-context index.

```cpp
using ScratchIndex = typename TopologyIndexTraits<
            TaskScratchIndexSpace,
            ScratchCapacity
        >::Storage;
```

### `alignas`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Storage reused first by the callable and then by its result.

```cpp
alignas(std::max_align_t) std::byte Payload[PayloadCapacity];
```

### `QueueOrExecutionContext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Queued: next record index. Running: managed execution-context index owning execution.

```cpp
ScratchIndex QueueOrExecutionContext =
            static_cast<ScratchIndex>(
                QueueIndex::InvalidValue
            );
```

### `Control`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

One-byte Task lifecycle/ownership/Phase representation.

```cpp
TaskControl Control;
```

### `PayloadOperations`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

One pointer to immutable Type-specific payload operations shared by all records of that callable/result pairing.

```cpp
const TaskPayloadOperations<TaskRecord>* PayloadOperations = nullptr;
```

### `SetQueueNext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Stores the next queued record index in the mutually exclusive scratch field.

```cpp
void SetQueueNext(
            QueueIndex recordIndex
        ) noexcept
```

### `QueueNext`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the next queued strong record identity reconstructed from the mutually exclusive raw scratch field. Invalid raw queue state maps to `QueueIndex::Invalid()`.

```cpp
QueueIndex QueueNext() const noexcept
```

### `SetExecutionContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Stores the managed execution-context index after this record is granted to a Worker.

```cpp
void SetExecutionContextIndex(
            ExecutionContextIndex contextIndex
        ) noexcept
```

### `CurrentExecutionContextIndex`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the managed execution-context index owning current Worker execution.

```cpp
ExecutionContextIndex CurrentExecutionContextIndex() const noexcept
```

