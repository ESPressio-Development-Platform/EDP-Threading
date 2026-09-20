# Threading Architecture

## Purpose and ownership boundary

EDP-Threading owns two execution models: finite `Task<TResult>` work executed by bounded Task facilities, and persistent topology-owned Dedicated Threads controlled through `Thread<TThreadIdentity>`.

Tasks do not intrinsically own native execution contexts. Workers are internal reusable contexts. Dedicated Threads own independent persistent Platform execution contexts and never consume Task Worker capacity. A `DedicatedWorkerLease` is a separate critical-Task facility with exactly one isolated Worker; it does not borrow from an ordinary pool.

EDP-Platform owns native execution and synchronization mechanisms. EDP-Clock owns canonical monotonic time. Static Composition declares entitlement; Bootstrap realizes it. Threading does not expose FreeRTOS, ESP-IDF, Arduino or operating-system vocabulary through its public execution semantics.

## Composition and topology

`ThreadingTopology<...>` is the compile-time list of ordinary Task facilities, Dedicated Worker leases and Dedicated Threads. Each resource contributes a fixed number of managed sequential execution contexts. Prefix-sum topology metadata assigns every context a dense `ExecutionContextIndex`.

Requirements may constrain semantic identity, minimum stack capacity, minimum Worker concurrency, logical priority and affinity. Invalid or insufficient topology choices fail at compile time. Composition validates architecture; it is not a runtime service locator or dependency-injection container.

## Concrete realization

`StaticThreadingRuntime` owns, in deterministic order:

1. the authoritative infrastructure lifecycle;
2. one targeted wake primitive per managed context;
3. the structural managed-context router;
4. application Dedicated Thread callable bindings;
5. directly constructed heterogeneous runtime resources;
6. the structural context resolver;
7. the bounded shutdown-completion wait surface.

Routing is constructed in two phases: resources are constructed against the wake-capable router, then the completed structural resolver is bound. No managed execution is permitted during construction.

Resources are stored recursively in place, avoiding a requirement that already-constructed Platform provider or synchronization objects be movable merely to place them in a heterogeneous tuple.

## Resource categories

### TaskExecutionFacility

Owns finite Task records, bounded callable/result payload storage, availability bitmap, intrusive FIFO admission state, bounded target-owned wait registrations and one persistent Platform execution context per declared Worker.

A Task is published as Running for a sleeping Worker only after that Worker's targeted wake has succeeded while the facility mutex is held. The currently executing Worker may accept queued work directly without a wake. This ordering prevents a failed wake from stranding a Task in Running state on a Worker that never became runnable.

### DedicatedWorkerLease

Owns exactly one isolated Worker and one bounded Task facility. Ordinary Task facilities cannot consume that Worker. Storage and execution properties are declared with the lease.

### DedicatedThread

Owns one persistent Platform execution context and one stateful application callable. Infrastructure Start enters the persistent trampoline; semantic activation is controlled separately after global operational commit.

## Platform and time boundaries

Logical execution priority is Low, Normal, High or Critical. Processor affinity uses Platform's logical affinity abstraction.

All monotonic deadlines and physical-duration calculations derive from the application-selected canonical EDP-Clock monotonic timeline. Native scheduler ticks are implementation mechanisms only. Internal wakes never restart a caller's timeout budget.

## Wake and interruption routing

Every managed sequential context owns one reusable targeted wake primitive. Wait targets store the smallest dense context index rather than context pointers. Current-context and interruption resolution scan the statically owned resource structure. There is no runtime service locator or per-context pointer registry.

## Bootstrap

Initialization and infrastructure Start are separate phases. Initialization validates and realizes resources but starts no EDP-managed execution context. Infrastructure contexts start only after initialization commits. Declaration order is the default deterministic order; applications that require an absolute cross-resource order can supply a compile-time permutation with `InitializeInOrder<...>()` and `StartInOrder<...>()`. The permutation must name every resource exactly once.

Start is transactional: the first Platform Start failure prevents later Starts, requests cooperative termination of already-started trampolines, joins them, destroys initialized context resources and never commits semantic activation.

Task dispatch and Dedicated Thread semantic activation are valid only while the global Threading lifecycle is operational.

## Empty topology

`ThreadingTopology<>` is valid. It owns no Platform wake providers or managed execution resources. Wake-set, router, structural-resolver and shutdown-registration storage compile away.
