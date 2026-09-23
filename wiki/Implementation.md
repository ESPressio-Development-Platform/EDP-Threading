# Private Implementation

Task records reuse one bounded payload area for callable/result storage. Constructors/destructors run under carefully controlled ownership rather than hidden allocation. Running Task cancellation is cooperative; no forced execution termination is introduced.

Dedicated-thread Join captures an activation phase so a later restart cannot accidentally satisfy a wait targeting the previous activation. BeginShutdown is terminal and rejects new admission while propagating cancellation/stop requests.
