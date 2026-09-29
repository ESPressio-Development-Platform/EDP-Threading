# Public API

The principal public types are `TaskExecutionFacility<...>`, `DedicatedWorkerLease<TaskIdentity,...>`, `DedicatedThread<ThreadIdentity,...>`, `ThreadingTopology<...>`, `DedicatedThreadBinding<...>`, `BindDedicatedThread<...>()`, `Thread<TIdentity>`, `ThreadContext`, `OrdinaryMutex<TIdentity>`, `OrdinaryMutexProvider<TIdentity,TMutexProvider>` and `StaticThreadingRuntime<...>`.

Task handles are move-only and non-owning with respect to runtime infrastructure. Task state progresses through queued/running/terminal states with cooperative cancellation. Dedicated Threads have persistent infrastructure contexts whose semantic activation may be started, stopped and restarted without recreating the underlying execution provider. `Thread::Wake()` publishes an advisory wake to the existing context signal; `ThreadContext::Wait()`, `WaitFor()` and `WaitUntil()` consume that same signal. `OrdinaryMutexProvider` exposes keyed ordinary-context non-recursive mutual exclusion without leaking Platform mutex vocabulary to higher domains.

Exact declarations remain authoritative in the exported headers.
