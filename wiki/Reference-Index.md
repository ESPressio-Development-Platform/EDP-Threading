# Reference Index

Every production header under `src/` has a source-derived reference page. Public Task/Thread/topology contracts are separated from the extensive private static-runtime machinery.

| Source header | Classification | Reference |
|---|---|---|
| `src/ESPressio_Threading.hpp` | PUBLIC ENTRY POINT | [open](Reference-ESPressio-Threading) |
| `src/threading/detail/CallableTraits.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-CallableTraits) |
| `src/threading/detail/DedicatedThreadControl.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-DedicatedThreadControl) |
| `src/threading/detail/DedicatedThreadOwnedRuntime.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-DedicatedThreadOwnedRuntime) |
| `src/threading/detail/DedicatedThreadRuntime.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-DedicatedThreadRuntime) |
| `src/threading/detail/DedicatedWorkerLeaseRuntime.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-DedicatedWorkerLeaseRuntime) |
| `src/threading/detail/DedicatedWorkerOwnedRuntime.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-DedicatedWorkerOwnedRuntime) |
| `src/threading/detail/ExecutionControl.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-ExecutionControl) |
| `src/threading/detail/FacilityStorage.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-FacilityStorage) |
| `src/threading/detail/InfrastructureLifecycle.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-InfrastructureLifecycle) |
| `src/threading/detail/ManagedContextRouter.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-ManagedContextRouter) |
| `src/threading/detail/ManagedContextWakeSet.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-ManagedContextWakeSet) |
| `src/threading/detail/MonotonicWaitBudget.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-MonotonicWaitBudget) |
| `src/threading/detail/ShutdownCoordinator.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-ShutdownCoordinator) |
| `src/threading/detail/ShutdownWaitRuntime.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-ShutdownWaitRuntime) |
| `src/threading/detail/StaticTopologyOwner.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-StaticTopologyOwner) |
| `src/threading/detail/StaticTopologyPlan.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-StaticTopologyPlan) |
| `src/threading/detail/StaticTopologyResourceStorage.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-StaticTopologyResourceStorage) |
| `src/threading/detail/StaticTopologyResourceTypes.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-StaticTopologyResourceTypes) |
| `src/threading/detail/StructuralContextResolver.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-StructuralContextResolver) |
| `src/threading/detail/TaskFacilityCore.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-TaskFacilityCore) |
| `src/threading/detail/TaskFacilityOwnedRuntime.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-TaskFacilityOwnedRuntime) |
| `src/threading/detail/TaskFacilityRuntime.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-TaskFacilityRuntime) |
| `src/threading/detail/TaskPayloadAdapter.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-TaskPayloadAdapter) |
| `src/threading/detail/TaskRecord.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-TaskRecord) |
| `src/threading/detail/TaskWorkerExecutionContext.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-TaskWorkerExecutionContext) |
| `src/threading/detail/ThreadingBootstrap.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-ThreadingBootstrap) |
| `src/threading/detail/TopologyIndex.hpp` | PRIVATE IMPLEMENTATION / INTERNAL DOMAIN ADAPTATION | [open](Reference-threading-detail-TopologyIndex) |
| `src/threading/detail/TopologyResourceLookup.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-TopologyResourceLookup) |
| `src/threading/detail/WaitRegistration.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-WaitRegistration) |
| `src/threading/detail/WorkerLeaseScheduler.hpp` | PRIVATE IMPLEMENTATION | [open](Reference-threading-detail-WorkerLeaseScheduler) |
| `src/threading/Task.hpp` | PUBLIC API | [open](Reference-threading-Task) |
| `src/threading/TaskCompletion.hpp` | PUBLIC API | [open](Reference-threading-TaskCompletion) |
| `src/threading/TaskResults.hpp` | PUBLIC API | [open](Reference-threading-TaskResults) |
| `src/threading/Thread.hpp` | PUBLIC API | [open](Reference-threading-Thread) |
| `src/threading/Threading.hpp` | PUBLIC API | [open](Reference-threading-Threading) |
| `src/threading/ThreadingComposition.hpp` | PUBLIC COMPOSITION / TOPOLOGY API | [open](Reference-threading-ThreadingComposition) |
| `src/threading/ThreadingTypes.hpp` | PUBLIC API | [open](Reference-threading-ThreadingTypes) |

> Bounded-topology migration reference pages are pinned individually to the exact source commit they describe. Unchanged pages retain their previous exact-source baseline until their source changes.
