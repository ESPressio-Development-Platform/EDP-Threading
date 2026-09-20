# Threading Lifecycle and Ownership

## Tasks

A Task has public states Queued, Running, Completed and Cancelled. Running cancellation-request state remains internal and projects publicly as Running.

`Task<TResult>` is move-only and owns one public observation/result interest. Destruction is nonblocking and does not implicitly wait. Successful non-void `TakeResult()` consumes both the in-place result and the public Task ownership interest.

Cancellation is cooperative. Cancelling queued work prevents callable execution. Cancelling running work requests cooperation; a callable that ignores the request and returns normally completes normally.

Task waits are predicate-driven. Wait registration is bounded and target-owned, and every wake re-evaluates authoritative record/Phase state.

## Dedicated Threads

`Thread<TThreadIdentity>` is a move-only non-owning control handle to a topology-owned resource. Destroying the handle does not stop or join the resource.

Dedicated Threads have one persistent Platform execution context and are semantically activated through `Start()`. `RequestStop()` is cooperative. A stopped Thread may restart without a prior Join.

Join captures the current one-bit activation Phase. A concurrent later restart cannot retarget an already captured Join. Starting while an activation is Running or internally stop-requested returns AlreadyRunning; no deferred restart is queued.

A moved-from Thread handle is invalid but safe to observe/control: it does not dereference its former resource.

## Initialization

Construction owns all statically configured backing but starts no EDP-managed execution context. `Initialize()` validates targeted wake providers and initializes Platform execution providers in declaration order. A failure destroys the failing provider and rolls back already initialized resources. Only successful completion commits the global initialization barrier.

## Infrastructure Start

`Start()` starts resources in topology declaration order. The operation is transactional: the first failure prevents later starts, requests termination of already-started trampolines, joins them and destroys initialized resources. Semantic Task dispatch and Dedicated Thread activation remain rejected until Start commits successfully.

## Semantic execution

After operational commit:

- ordinary Tasks are dispatched through their Pool identity;
- Dedicated Worker work is dispatched through its critical Task identity;
- Dedicated Threads are addressed through Thread identity and explicitly activated;
- `Yield`, `SleepFor`, `SleepUntil`, Task waits and Thread joins remain predicate-driven and interruption-aware.

## Shutdown

Shutdown is terminal and initiation is nonblocking. `BeginShutdown()` changes the authoritative lifecycle first, then structurally traverses the topology. New Task admission and Dedicated Thread activation cease, queued Tasks are cancelled, running Tasks receive cancellation requests and active Dedicated Threads receive stop requests.

`IsExecutionQuiescent()` scans every resource. Execution quiescence is distinct from Task-record retention: completed/cancelled records may remain because a public handle or waiter still exists without keeping Worker infrastructure alive. Arbitrary user code is never forcefully terminated; a noncooperating callable may therefore prevent terminal shutdown forever.

After quiescence, `FinalizeShutdown()` requests infrastructure termination, joins and destroys all persistent contexts, publishes `ShutdownComplete`, then wakes the runtime-owned bounded shutdown waiters. Applications may observe completion with `WaitForShutdown()`, `WaitForShutdownFor()` or `WaitForShutdownUntil()`. Completion publication occurs once and only after teardown.
