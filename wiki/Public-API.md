# Public API

The principal public types are `TaskExecutionFacility<...>`, `DedicatedWorkerLease<TaskIdentity,...>`, `DedicatedThread<ThreadIdentity,...>`, `ThreadingTopology<...>`, `DedicatedThreadBinding<...>`, `BindDedicatedThread<...>()` and `StaticThreadingRuntime<...>`.

Task handles are move-only and non-owning with respect to runtime infrastructure. Task state progresses through queued/running/terminal states with cooperative cancellation. Dedicated Threads have persistent infrastructure contexts whose semantic activation may be started, stopped and restarted without recreating the underlying execution provider.

Exact declarations remain authoritative in the exported headers.
