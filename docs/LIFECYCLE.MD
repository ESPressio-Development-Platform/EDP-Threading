# Threading Lifecycle and Ownership

## Tasks

A Task has public states Queued, Running, Completed and Cancelled. Running cancellation-request state remains an internal state and projects publicly as Running.

`Task<TResult>` is move-only and owns one public observation/result interest. Destruction is nonblocking and does not implicitly wait. Successful non-void `TakeResult()` consumes both the in-place result and the public Task ownership interest.

Cancellation is cooperative. Cancelling queued work prevents callable execution. Cancelling running work requests cooperation; a callable that ignores the request and returns normally completes normally.

## Dedicated Threads

`Thread<TThreadIdentity>` is a move-only non-owning control handle to a topology-owned resource. Destroying the handle does not stop or join the resource.

Dedicated Threads are explicitly activated through `Start()`. `RequestStop()` is cooperative. A stopped Thread may restart without a prior Join.

Join binds to the activation captured when Join begins. A concurrent later restart cannot retarget an already captured Join. Starting while an activation is Running or internally stop-requested returns AlreadyRunning; no deferred restart is queued.

## Shutdown

Shutdown is terminal. New Task admission and Dedicated Thread activation cease, queued Tasks are cancelled, running Tasks receive cancellation requests and active Dedicated Threads receive stop requests. Arbitrary user code is never forcefully terminated.
