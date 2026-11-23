# Optional current-context binding

Include `<ESPressio_Threading_ContextBinding.hpp>` and explicitly select a `ContextBindingPolicy<BindingProvider, ContextCount, SignalProvider, SpinLockProvider>` as the seventh `StaticThreadingRuntime` template argument. This owner integration uses the typed Platform `CurrentContextBinding` capability. It is independent of Profiling; it creates no capture gateway, recorder, native thread, identity allocator or clock.

The existing six-argument runtime selects an empty policy and the original router Type. It adds no binding lookup, policy pointer, acknowledgement signal, per-context facts, startup wait or compiler TLS instance. Existing `Start()`, `StartInOrder()` and `FinalizeShutdown()` retain their previous semantics. The selected runtime instead requires explicit finite `StartFor(Duration)` / `StartInOrderFor<...>(Duration)` and `FinalizeShutdownFor(Duration)` calls. No timeout default is invented.

## Bootstrap and identity ownership

1. Bind the application's one canonical EDP-Clock monotonic source before runtime operations.
2. Allocate address-stable records and a `Policy::Mapping` in existing Bootstrap storage. Each mapping entry is `ContextBindingRegistration<Record>{nonzeroSessionIdentity, record}`. Array position is the existing dense topology context slot: ordinary Workers in resource/worker declaration order, then the declared Dedicated Workers and Dedicated Threads in their actual topology positions. The mapping capacity must equal `Topology::ManagedExecutionContextCount`.
3. Construct the policy from the mapping, then construct the runtime from the Dedicated callable-binding tuple and policy reference. The records, mapping, policy, runtime and selected providers must outlive every native use and outstanding capture/export borrow. Freeze the mapping before initialization; a `const` reference does not prevent a separate mutable alias from breaking this contract.
4. `Initialize()` validates every nonzero identity and rejects duplicate identities or record addresses before creating native contexts. It accepts the mapping only once. The caller owns uniqueness across all application runtimes and the entire session, and must make the payload's context identity/direct capture slot agree with that mapping. Threading cannot introspect an arbitrary domain-neutral payload or prove historical session uniqueness.
5. Call `StartFor(explicitBudget)` or its ordered counterpart. Every actual native outer entry binds before its persistent inner Worker/activation body. Successful startup requires every required `Bound` acknowledgement. Semantic dispatch/activation remains closed until that barrier commits. This is setup work, never method-probe work.

The selected Platform header currently requires `EDP-Platform/feature/profiling-v1`; the qualified ESP32/ESP32-S3 native carrier and corrected execution owner are on `EDP-Platform-ESP-IDF/main`. Select their actual qualified revisions together. Threading itself includes neither SDK nor Profiling headers. Other binding implementations must satisfy the existing exact Composition/provider-trait contract.

## Failed startup and bounded continuation

`AlreadyBound` and `UnsupportedContext` refuse eligible work in that native entry; an existing binding is preserved. `StartFor` returns the existing `ThreadingStartResult::ProviderFailure` for registration failure, missing/late acknowledgement, exhausted finite budget or native Start failure. `policy.StartupResult()` and `policy.Snapshot(index)` distinguish the setup outcome and exact per-context binding/native facts. A timeout may leave a `Pending` entry which later runs and retires normally; late registration never reopens failed startup.

One canonical Clock budget covers the complete acknowledgement barrier, including time consumed by native start calls. Provider operations themselves must satisfy their own latency contracts; Threading cannot make an intrinsically blocking provider operation bounded by passing it a duration. The selected policy refuses a Forever acknowledgement budget.

On failure the topology publishes terminal start rollback before waking resources. A partly started Task facility does not privately wait Forever before that publication. No forced native deletion or false cleanup occurs. The runtime, mapping, policy and native backing remain owned and address stable. Call `FinalizeShutdownFor` with a separately supplied finite budget to continue cooperative cleanup, and retain the complete owner if it returns anything other than `Completed`. Each attempt shares its one deadline across all resources/Workers. Already destroyed contexts are skipped on later attempts; successful Join is retained if Destroy must be retried.

## Retirement and completion

Registration surrounds the complete persistent inner native body. Ordinary dispatch, blocking/yield and repeated Dedicated activation keep the same record and identity. A local outer guard retires exactly the expected record after the last inner scope returns. Binding/carrier/outer infrastructure and their lower dependencies must be excluded from future AST instrumentation; an automatic first-statement method probe there would run too early or recurse. The persistent inner body is the intended instrumentation boundary.

All exact retirement outcomes remain visible: `Retired`, `NotBound`, `DifferentBinding`, `UnsupportedContext`. A successful native Join cannot manufacture missing retirement evidence. An anomalous retirement prevents normal reclamation/completion; the control owner must preserve storage and diagnose it. No API clears foreign bindings or blindly resets activity flags. Where C++ exceptions are enabled, selected outer wrappers catch an exceptional inner exit after unwinding its scopes, retain `HasAbnormalExit`, and retire the expected binding. They do not falsely complete an abandoned Task/activation; normal finalization remains incomplete. Unselected exception behavior is unchanged. No unwinding guarantee exists for abort, reset, power loss or a toolchain with exceptions disabled.

Native `Join` must independently prove nonexecution before `Destroy`. The corrected ESP-IDF owner distinguishes the completion notification from stable task parking. Record/native retirement is not capture quiescence, exporter completion or durable acknowledgement. The single future capture lifecycle owner must apply D8.4's independent closure/quiescence protocol before reusing a capture slot or descriptor.

## Memory and concurrency

The selected policy retains one mapping pointer, one concrete binding provider, one existing SpinLock, one existing Signal, one packed fact byte per native context, a startup-result byte and a preparation bit (subject to ordinary alignment). The selected router adds one typed policy pointer. No per-Worker field or second wait mechanism is added. `Snapshot` returns a temporary decoded value; its extra fields are not duplicated in retained storage.

The control owner serializes preparation/start/shutdown/finalization and startup-result access. Native entries independently publish their direct slot's facts under the selected SpinLock; cross-core publication preserves the concurrently updated binding/native subfields. Scanning/identity validation/waiting happens only in setup/control paths. Probe lookup remains the selected Platform carrier's bounded O(1) operation, with no new Threading scan, allocation, lock, I/O or callback search. ISR calls are outside Threading's supported application contract.

## Validation

`tests/run-context-binding-tests.sh` exercises actual Threading ownership over independent host threads with explicitly synthetic binding/native fixtures: persistence across dispatch/activation, precise bind refusal, a partly started multi-Worker facility, late entry, retained storage, fresh incarnation identities, invalid mapping, exact retirement anomalies and exceptional owner exit. The exception subprocess intentionally ends while the failed owner remains retained; it does not claim production reclamation. Existing default tests remain separate. Native SDK builds, physical owner acceptance and compiled default-removal evidence must be recorded separately; host fixtures do not prove them.

## Complete source-led example

The [three-form project](../demos/current-context-binding/README.MD) includes the complete record/provider/topology/Clock/start/dispatch/activation/retirement integration. Its three Example.hpp copies are identical; only the framework entry differs. Public demo dependency references are GitHub URLs. The native execution concrete is selected explicitly under both frameworks so Join/Destroy uses the qualified native owner.

## Native failure acceptance project

The maintained [SDK acceptance fixture](../tests/context-binding-hil/README.MD) uses actual Threading ownership and native TLS/execution over explicit fault-forwarding test adapters. It exercises foreign binding preservation, UnsupportedContext, partial native startup, bounded late acknowledgement, repeated fixed-backing incarnations and retained retirement mismatch on both boards/frameworks. The terminal failed owner remains live until reset. These are owner-lifetime checks, not capture or durable-export acceptance.

The [same-Worker migration project](../tests/context-binding-migration/README.MD) forces one actual unpinned production Worker from core zero to core one, checks identical native task/record/identity, then verifies another dispatch and normal retirement/Destroy. This is separate from the earlier raw-carrier migration fixture. Both hardware acceptance projects are built by tests/run-context-binding-hil-builds.sh and the SDK CI job.
