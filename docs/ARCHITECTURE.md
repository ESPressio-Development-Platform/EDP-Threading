# EDP-Threading Architecture

## Ownership boundary

Threading owns semantic Task and Dedicated Thread behaviour. EDP-Platform owns native execution/synchronization mechanisms. EDP-Clock owns canonical monotonic time. Static Composition declares entitlement; Bootstrap realizes it.

A `ThreadingTopology` is a compile-time list of ordinary Task facilities, Dedicated Worker leases and Dedicated Threads. Each resource contributes a fixed number of managed sequential execution contexts. Prefix-sum topology metadata assigns every context a dense `ExecutionContextIndex`.

## Concrete realization

`StaticThreadingRuntime` owns, in deterministic order:

1. the authoritative infrastructure lifecycle;
2. one targeted wake primitive per managed context;
3. the structural managed-context router;
4. application Dedicated Thread callable bindings;
5. directly constructed heterogeneous runtime resources;
6. the structural context resolver.

Construction is two-phase for routing: resources are constructed against the wake-capable router, then the completed structural resolver is bound. No managed execution is permitted during this construction phase.

Resources are stored recursively in place. This avoids requiring already-constructed Platform provider objects or synchronization objects to be movable merely to place them in a heterogeneous tuple.

## Resource categories

### TaskExecutionFacility

Owns finite Task records, bounded callable/result payload storage, availability bitmap, intrusive FIFO admission state, bounded target-owned wait registrations and one persistent Platform execution context per declared Worker.

### DedicatedWorkerLease

Owns exactly one isolated Worker and one bounded Task facility. Ordinary Task facilities cannot consume that Worker. Storage and execution properties are declared with the lease.

### DedicatedThread

Owns one persistent Platform execution context and one stateful application callable. Infrastructure Start enters the persistent trampoline; semantic `StartThread` controls activations after global operational commit.

## Wake and interruption routing

Every managed context has one reusable targeted wake primitive. Wait targets store the smallest dense context index rather than context pointers. Current-context and interruption resolution scan the statically owned resource structure. There is no runtime service locator or per-context pointer registry.

## Empty topology

`ThreadingTopology<>` is valid. It owns no Platform wake providers or managed execution resources. Wake-set, router and structural-resolver specializations compile down to empty objects.
