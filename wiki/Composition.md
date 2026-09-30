# Composition

Threading declares a static application topology but continues to consume Platform/Clock capabilities through compile-time contracts. Bootstrap owns concrete provider instances and Threading resources.

Initialization and Start order are explicit. Optional `InitializeInOrder<...>` and `StartInOrder<...>` require a complete permutation of topology resources rather than allowing partial ad-hoc ordering.

`FailClosedOrdinaryMutexProvider` is also a Threading Composition provider. It offers the distinct exclusive `FailClosedOrdinaryMutex<TIdentity>` capability and is parameterized by the concrete Platform Mutex and independent Platform SpinLock providers selected by Bootstrap. The higher-domain consumer therefore binds one Threading provider while Platform-specific failure-publication mechanics remain below the Threading boundary.
