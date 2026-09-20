# Threading Lifecycle and Ownership

## Tasks

A Task has public states Queued, Running, Completed and Cancelled. Running cancellation-request state remains internal and projects publicly as Running.

`Task<TResult>` is move-only and owns one public observation/result interest. Destruction is nonblocking and does not implicitly wait. Successful non-void `TakeResult()` consumes both the in-place result and the public Task ownership interest.

Cancellation is cooperative. Cancelling queued work prevents callable execution. Cancelling running work requests cooperation; a callable that ignores the request and returns normally completes normally.

Task waits are predicate-driven. Wait registration is bounded and target-owned, and every wake re-evaluates the authoritative record/Phase state.

## Dedicated Threads

`Thread<TThreadIdentity>` is a move-only non-owning control handle to a topology-owned resource. Destroying the handle does not stop or join the resource.

Dedicated Threads have one persistent Platform execution context and are semantically activated through `Start()`. `RequestStop()` is cooperative. A stopped Thread may restart without a prior Join.

Join captures the current one-bit activation Phase. A concurrent later restart cannot retarget an already captured Join. Starting while an activation is Running or internally stop-requested returns AlreadyRunning; no deferred restart is queued.

## Startup

Threading Initialization starts no managed execution. Infrastructure Start occurs only after the application initialization barrier and is transactional. Semantic Task dispatch and Dedicated Thread activation are rejected until Start commits successfully.

## Shutdown

Shutdown is terminal and initiation is nonblocking. New Task admission and Dedicated Thread activation cease, queued Tasks are cancelled, running Tasks receive cancellation requests and active Dedicated Threads receive stop requests.

Execution quiescence is distinct from Task-record retention: completed/cancelled records may remain because a public handle or waiter still exists without keeping Worker infrastructure alive.

After all semantic execution cooperatively ceases, persistent contexts are woken, joined and destroyed, terminal shutdown is published and registered shutdown waiters are targeted. Arbitrary user code is never forcefully terminated; a noncooperating callable may therefore prevent terminal shutdown forever.
