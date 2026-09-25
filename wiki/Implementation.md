# Private Implementation

Task records reuse one bounded payload area for callable/result storage. Constructors/destructors run under carefully controlled ownership rather than hidden allocation. Running Task cancellation is cooperative; no forced execution termination is introduced.

Compact index width selection now delegates to `EDP-BoundedTopology::BoundedIndex`. Task-record and Worker availability use shared one-bit `BoundedIndexSet` storage, and queued Tasks use the shared two-endpoint `IntrusiveQueue`. Threading supplies the lifecycle semantics around those mechanics.

The critical scratch optimization remains unchanged: `TaskRecord::QueueOrExecutionContext` is one raw scalar whose interpretation changes from next-record identity while Queued to managed execution-context identity after Worker grant. The implementation never stores both identities simultaneously.

Dedicated-thread Join captures an activation phase so a later restart cannot accidentally satisfy a wait targeting the previous activation. BeginShutdown is terminal and rejects new admission while propagating cancellation/stop requests.
