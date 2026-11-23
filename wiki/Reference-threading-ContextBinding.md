# src/threading/ContextBinding.hpp

**PUBLIC API:** ContextBindingPolicy configuration/lifetime, ContextBindingRegistration, ContextBindingSnapshot, result/phase/native-state vocabulary and caller inspection. **INTERNAL OWNER CONTRACT:** policy entry/native publication/cleanup and BindingManagedContextRouter. **PRIVATE IMPLEMENTATION:** synchronized fact packing and barrier helpers. [Normative behavior](../docs/CONTEXT-BINDING.MD).

## Values and record types

ContextBindingPhase has explicit values Pending=0 (no entry acknowledgement), Bound=1 (own record accepted), AlreadyBound=2 (occupied/foreign record preserved), UnsupportedContext=3 (binding refused), Retired=4 (expected own record cleared), RetirementNotBound=5, RetirementDifferentBinding=6 and RetirementUnsupportedContext=7 (exact anomalous retirement, never normal completion).

ContextBindingStartupResult has NotAttempted=0, Ready=1, InvalidMapping=2, BindingRejected=3, TimedOut=4, ProviderFailure=5. Prepare may validate successfully while StartupResult remains NotAttempted: Ready is published by the acknowledgement barrier, not construction. ContextBindingNativeState has Uninitialized=0, Initialized=1, Started=2, Joined=3, Destroyed=4; native completion is independent of registration/export.

ContextBindingRegistration<TRecord> borrows an existing record via std::addressof and stores its owner-supplied uint64 Identity and Record pointer. Its constructor accepts an lvalue only; deleted mutable/const rvalue overloads prevent obvious dangling borrows. Identity must be nonzero/session-unique and its record payload must agree. ContextBindingSnapshot is a temporary coherent observation with Phase, NativeState and HasAbnormalExit; no payload for an invalid index is returned by Snapshot.

## ContextBindingPolicy<TBindingProvider,TCapacity,TSignalProvider,TSpinLockProvider>

The policy is noncopyable/nonmovable and address stable. BindingTraits validates the Platform capability, bounded properties and exact methods/record/noexcept contract; LockTraits validates the existing SpinLock. ContextRecord and Mapping are exact selected record/fixed-array aliases; ContextCapacity and IsSelected are compile-time facts. Router<TCount,TWakeProvider> selects the typed router adapter and compile-refuses a capacity mismatch.

| Retained member | Ownership/publication purpose |
|---|---|
| _mapping | Borrowed immutable fixed Bootstrap mapping; never a lookup registry |
| _provider | One qualified carrier instance, current-context operations only |
| _lock | Existing SpinLock protects per-context publication and Snapshot/barrier reads |
| _acknowledged | One existing Signal for all startup acknowledgements |
| _facts | Exactly TCapacity bytes; low four bits phase, next three native state, high bit abnormal exit; subfield writes preserve concurrent facts |
| _startup | Serialized control-owner setup outcome, not a native completion flag |
| _prepared | One-shot preparation guard; never reset for native slot reuse |

| Operation | Preconditions, outcome and architectural role |
|---|---|
| constructor / deleted Mapping rvalue overloads | Borrow stable mapping; reject temporary mapping lifetime |
| deleted copying/assignment | Synchronization and native evidence cannot be duplicated; implicit move unavailable |
| Prepare | Serialized setup before native initialization; bounded scan rejects zero/duplicate IDs, null/duplicate record addresses and repeated preparation; no session allocator |
| Await | Serialized startup, finite shared MonotonicWaitBudget only; scans fixed facts outside probes and waits on existing Signal; exact ready/rejected/timeout/provider outcome |
| StartupResult | Serialized owner inspection of the last setup outcome |
| RecordNativeStartFailure | Internal setup publication of native Start failure; leaves slot facts intact |
| Enter | Internal actual native entry with validated direct index; exact Platform Bind result, no overwrite |
| Leave | Internal expected-record retirement after final inner scope; retains every exact failure |
| MarkInitialized / MarkStarted | Internal successful native lifecycle publication independent of binding phase |
| MarkAbnormalExit | Sticky exceptional-exit bit, no clearing of semantic activity or fabricated completion |
| Snapshot | Locked coherent direct-slot inspection; nullopt on out-of-range input |
| Cleanup<TExecutionProvider> | Serialized owner-only native continuation under caller's one existing budget; skips already-destroyed/uninitialized slots, retains Join success, refuses anomalous/missing retirement and abnormal exit, destroys only after independent native proof |
| PublishPhase / PublishNative | Private synchronized subfield updates; phase publication additionally notifies startup |
| ReadBarrier | Private bounded setup scan; pending means wait, any failure/early retirement means rejected, all Bound means ready |

Mapping, record lifetime and historical session uniqueness remain application responsibilities. Publicly accessible owner-integration methods are not ordinary consumer mutation hooks. A genuinely new native incarnation needs fresh identity, a fresh policy and independently retired native/capture/export borrows.

## BindingManagedContextRouter<TCapacity,TSignalProvider,TPolicy>

INTERNAL owner adapter composes the final original ManagedContextRouter in _base and borrows _binding. Base, WakeSet and ContextIndex retain original vocabulary; ContextCapacity is the exact direct-slot bound; BindingOutcome is the Platform bind enum; HasContextBinding is true. Constructor validates policy/topology capacity. BindTopology, IsTopologyBound, CurrentContextIndex, IsInterrupted, Wake and Wait forward the original mechanisms without introducing another resolver/waiter. BindingPolicy exposes the one selected owner, EnterContext delegates direct-slot registration and LeaveContext delegates exact retirement. No provider search, heap or probe scan is added.
