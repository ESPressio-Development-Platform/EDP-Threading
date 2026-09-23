# Composition

Threading declares a static application topology but continues to consume Platform/Clock capabilities through compile-time contracts. Bootstrap owns concrete provider instances and Threading resources.

Initialization and Start order are explicit. Optional `InitializeInOrder<...>` and `StartInOrder<...>` require a complete permutation of topology resources rather than allowing partial ad-hoc ordering.
