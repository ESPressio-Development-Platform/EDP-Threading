#include <cassert>
#include <cstdint>
#include <type_traits>
#include <tuple>

#include <ESPressio_Threading.hpp>

#include "../src/threading/detail/DedicatedThreadControl.hpp"
#include "../src/threading/detail/DedicatedThreadRuntime.hpp"
#include "../src/threading/detail/DedicatedThreadOwnedRuntime.hpp"
#include "../src/threading/detail/DedicatedWorkerLeaseRuntime.hpp"
#include "../src/threading/detail/DedicatedWorkerOwnedRuntime.hpp"
#include "../src/threading/detail/FacilityStorage.hpp"
#include "../src/threading/detail/ExecutionControl.hpp"
#include "../src/threading/detail/InfrastructureLifecycle.hpp"
#include "../src/threading/detail/ManagedContextRouter.hpp"
#include "../src/threading/detail/ShutdownWaitRuntime.hpp"
#include "../src/threading/detail/ShutdownCoordinator.hpp"
#include "../src/threading/detail/StaticTopologyPlan.hpp"
#include "../src/threading/detail/StaticTopologyResourceTypes.hpp"
#include "../src/threading/detail/StaticTopologyResourceStorage.hpp"
#include "../src/threading/detail/StaticTopologyOwner.hpp"
#include "../src/threading/detail/StructuralContextResolver.hpp"
#include "../src/threading/detail/TopologyResourceLookup.hpp"
#include "../src/threading/detail/ThreadingBootstrap.hpp"
#include "../src/threading/detail/TaskFacilityCore.hpp"
#include "../src/threading/detail/TaskFacilityRuntime.hpp"
#include "../src/threading/detail/TaskFacilityOwnedRuntime.hpp"
#include "../src/threading/detail/TaskPayloadAdapter.hpp"
#include "../src/threading/detail/TaskRecord.hpp"
#include "../src/threading/detail/TaskWorkerExecutionContext.hpp"
#include "../src/threading/detail/WaitRegistration.hpp"
#include "../src/threading/detail/WorkerLeaseScheduler.hpp"

namespace Test {

    class AtomicByteProvider final {

        public:

            class Word final {

                private:

                    // Test atomic state.

                    /// Test-owned byte value.
                    std::uint8_t _value = 0U;

                public:

                    // Test atomic operations.

                    /// Reads the test byte without ordering significance.
                    std::uint8_t LoadRelaxed() const noexcept {
                        return _value;
                    }

                    /// Reads the test byte with acquire-equivalent test semantics.
                    std::uint8_t LoadAcquire() const noexcept {
                        return _value;
                    }

                    /// Stores the test byte without ordering significance.
                    void StoreRelaxed(
                        std::uint8_t value
                    ) noexcept {
                        _value = value;
                    }

                    /// Stores the test byte with release-equivalent test semantics.
                    void StoreRelease(
                        std::uint8_t value
                    ) noexcept {
                        _value = value;
                    }

                    /// Replaces the byte when the expected value matches.
                    bool CompareExchangeAcqRel(
                        std::uint8_t& expected,
                        std::uint8_t desired
                    ) noexcept {
                        if (_value != expected) {
                            expected = _value;
                            return false;
                        }

                        _value = desired;
                        return true;
                    }

            };

    };


    namespace Framework = ESPressio::System::CompositionFramework;


    class ExecutionContextProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Provides<
            Framework::Offer<
                ESPressio::Platform::Execution::ExecutionContext,
                Framework::PropertyValue<ESPressio::Platform::Execution::CallerSuppliedStorage, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsPriority, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsProcessorAffinity, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsStackTelemetry, false>,
                Framework::PropertyValue<ESPressio::Platform::Execution::ControlStorageBytes, 24U>,
                Framework::PropertyValue<ESPressio::Platform::Execution::ControlStorageAlignment, 8U>,
                Framework::PropertyValue<ESPressio::Platform::Execution::StackStorageAlignment, 16U>,
                Framework::PropertyValue<ESPressio::Platform::Execution::StackAllocationGranularityBytes, 32U>,
                Framework::PropertyValue<ESPressio::Platform::Execution::JoinWaitResolutionNanoseconds, 1U>
            >
        >
    > {

        private:

            bool _initialized = false;

        public:

            ExecutionContextProvider() noexcept = default;
            ExecutionContextProvider(const ExecutionContextProvider&) = delete;
            ExecutionContextProvider& operator =(const ExecutionContextProvider&) = delete;
            ExecutionContextProvider(ExecutionContextProvider&&) = delete;
            ExecutionContextProvider& operator =(ExecutionContextProvider&&) = delete;

            ESPressio::Platform::Execution::ExecutionInitializationResult Initialize(
                const ESPressio::Platform::Execution::ExecutionStorage& storage,
                const ESPressio::Platform::Execution::ExecutionConfiguration&,
                ESPressio::Platform::Execution::ExecutionEntry,
                void*
            ) noexcept {
                if (
                    storage.ControlAddress == nullptr ||
                    storage.ControlBytes != 24U ||
                    storage.StackAddress == nullptr ||
                    storage.StackBytes != 128U
                ) {
                    return ESPressio::Platform::Execution::ExecutionInitializationResult::InvalidStorage;
                }

                _initialized = true;
                return ESPressio::Platform::Execution::ExecutionInitializationResult::Succeeded;
            }

            ESPressio::Platform::Execution::ExecutionStartResult Start() noexcept {
                return _initialized
                    ? ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
                    : ESPressio::Platform::Execution::ExecutionStartResult::InvalidState;
            }

            ESPressio::Platform::Execution::ExecutionJoinResult Join(
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                return _initialized
                    ? ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded
                    : ESPressio::Platform::Execution::ExecutionJoinResult::InvalidState;
            }

            ESPressio::Platform::Execution::ExecutionDestroyResult Destroy() noexcept {
                if (!_initialized) {
                    return ESPressio::Platform::Execution::ExecutionDestroyResult::InvalidState;
                }

                _initialized = false;
                return ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;
            }

            bool IsCurrentContext() const noexcept {
                return false;
            }

            ESPressio::Platform::Execution::ExecutionStackTelemetry GetStackTelemetry() const noexcept {
                return {};
            }

            static void Yield() noexcept {}

    };


    class SignalProvider final {

        private:

            bool _signaled = false;

        public:

            ESPressio::Platform::Synchronization::SignalNotifyResult Notify() noexcept {
                _signaled = true;
                return ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled;
            }

            ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                if (_signaled) {
                    _signaled = false;
                    return ESPressio::Platform::Synchronization::SignalWaitResult::Signaled;
                }

                return ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut;
            }

    };


    class MutexProvider final {

        public:

            ESPressio::Platform::Synchronization::LockAcquireResult Acquire(
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                return ESPressio::Platform::Synchronization::LockAcquireResult::Acquired;
            }

            ESPressio::Platform::Synchronization::LockReleaseResult Release() noexcept {
                return ESPressio::Platform::Synchronization::LockReleaseResult::Released;
            }

    };


    class ManagedContextRouter final {

        public:

            static constexpr std::size_t ContextCapacity = 3U;

        private:

            std::size_t _wakeCount = 0U;

        public:

            std::optional<std::uint8_t> CurrentContextIndex() const noexcept {
                return static_cast<std::uint8_t>(0U);
            }

            bool IsInterrupted(
                std::uint8_t
            ) const noexcept {
                return false;
            }

            ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                std::uint8_t,
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                return ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut;
            }

            ESPressio::Platform::Synchronization::SignalNotifyResult Wake(
                std::uint8_t
            ) noexcept {
                ++_wakeCount;
                return ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled;
            }

            std::size_t WakeCount() const noexcept {
                return _wakeCount;
            }

    };


    struct LifetimeResult final {

        inline static std::size_t DestructionCount = 0U;

        bool Active = true;

        LifetimeResult() noexcept = default;

        LifetimeResult(
            LifetimeResult&& other
        ) noexcept :
            Active(other.Active) {
            other.Active = false;
        }

        LifetimeResult(const LifetimeResult&) = delete;
        LifetimeResult& operator =(const LifetimeResult&) = delete;
        LifetimeResult& operator =(LifetimeResult&&) = delete;

        ~LifetimeResult() {
            if (Active) {
                ++DestructionCount;
            }
        }

    };


    struct LifetimeCallable final {

        inline static std::size_t DestructionCount = 0U;

        bool Active = true;

        LifetimeCallable() noexcept = default;

        LifetimeCallable(
            LifetimeCallable&& other
        ) noexcept :
            Active(other.Active) {
            other.Active = false;
        }

        LifetimeCallable(const LifetimeCallable&) = delete;
        LifetimeCallable& operator =(const LifetimeCallable&) = delete;
        LifetimeCallable& operator =(LifetimeCallable&&) = delete;

        ~LifetimeCallable() {
            if (Active) {
                ++DestructionCount;
            }
        }

        LifetimeResult operator ()() noexcept {
            return LifetimeResult{};
        }

    };


    class ShutdownTaskResource final {

        public:

            bool CancellationStarted = false;
            bool Quiescent = true;

            void BeginShutdownCancellation() noexcept {
                CancellationStarted = true;
            }

            bool IsExecutionQuiescent() noexcept {
                return Quiescent;
            }

    };


    class ShutdownThreadResource final {

        public:

            bool StopRequested = false;
            bool Quiescent = true;

            ESPressio::Threading::ThreadStopRequestResult RequestStop() noexcept {
                StopRequested = true;
                return ESPressio::Threading::ThreadStopRequestResult::Accepted;
            }

            bool IsExecutionQuiescent() noexcept {
                return Quiescent;
            }

    };


    class LifecycleResource final {

        private:

            bool _failStart;

        public:

            std::size_t StartCount = 0U;
            std::size_t TerminationWakeCount = 0U;
            std::size_t JoinCount = 0U;
            std::size_t DestroyCount = 0U;

            explicit LifecycleResource(
                bool failStart = false
            ) noexcept :
                _failStart(failStart) {}

            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                ++StartCount;

                return _failStart
                    ? ESPressio::Platform::Execution::ExecutionStartResult::ProviderFailure
                    : ESPressio::Platform::Execution::ExecutionStartResult::Succeeded;
            }

            void RequestInfrastructureTermination() noexcept {
                ++TerminationWakeCount;
            }

            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout
            ) noexcept {
                ++JoinCount;
                return ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded;
            }

            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                ++DestroyCount;
                return ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;
            }

    };


    struct DedicatedThreadIdentity final {};

    struct UndeclaredThreadIdentity final {};

    struct UnknownExecutionProperty final {};


    struct DedicatedCallable final {

        void operator ()(
            ESPressio::Threading::ThreadContext& context
        ) noexcept {
            static_cast<void>(
                context.IsStopRequested()
            );
        }

    };


    struct OrdinaryPool final {};


    struct TelemetryThread final {};


    struct QueueRecord final {

        /// Compact intrusive queue-link storage.
        ESPressio::Threading::Detail::SmallestIndex<3U>::Type QueueLink =
            ESPressio::Threading::Detail::SmallestIndex<3U>::Invalid;

        /// Stores the next queued record index.
        void SetQueueNext(
            ESPressio::Threading::Detail::SmallestIndex<3U>::Type recordIndex
        ) noexcept {
            QueueLink = recordIndex;
        }

        /// Returns the next queued record index.
        ESPressio::Threading::Detail::SmallestIndex<3U>::Type QueueNext() const noexcept {
            return QueueLink;
        }

    };


    struct ReturningCallable final {

        /// Produces one deterministic test result.
        int operator ()() const noexcept {
            return 42;
        }

    };


    struct CancellationAwareCallable final {

        /// Acknowledges cooperative cancellation when requested.
        ESPressio::Threading::TaskCompletion<int> operator ()(
            ESPressio::Threading::TaskContext& context
        ) const {
            if (context.IsCancellationRequested()) {
                return ESPressio::Threading::TaskCompletion<int>::Cancelled();
            }

            return ESPressio::Threading::TaskCompletion<int>::Completed(
                7
            );
        }

    };


    using Topology = ESPressio::Threading::ThreadingTopology<
        ESPressio::Threading::TaskExecutionFacility<
            OrdinaryPool,
            ESPressio::Threading::TaskRecordCapacity<8U>,
            ESPressio::Threading::CallableCapacity<64U>,
            ESPressio::Threading::ResultCapacity<32U>,
            ESPressio::Threading::Workers<
                ESPressio::Threading::Worker<
                    ESPressio::Threading::StackCapacity<4096U>,
                    ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::Normal>,
                    ESPressio::Threading::AnyAffinity
                >
            >
        >,
        ESPressio::Threading::DedicatedWorkerLease<
            ReturningCallable,
            ESPressio::Threading::TaskRecordCapacity<3U>,
            ESPressio::Threading::CallableCapacity<32U>,
            ESPressio::Threading::ResultCapacity<16U>,
            ESPressio::Threading::StackCapacity<2048U>,
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::Critical>,
            ESPressio::Threading::AnyAffinity
        >,
        ESPressio::Threading::DedicatedThread<
            TelemetryThread,
            ESPressio::Threading::StackCapacity<4096U>,
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::High>,
            ESPressio::Threading::AnyAffinity
        >
    >;


    using HeterogeneousPool = ESPressio::Threading::ThreadingTopology<
        ESPressio::Threading::TaskExecutionFacility<
            OrdinaryPool,
            ESPressio::Threading::TaskRecordCapacity<4U>,
            ESPressio::Threading::CallableCapacity<32U>,
            ESPressio::Threading::ResultCapacity<16U>,
            ESPressio::Threading::Workers<
                ESPressio::Threading::Worker<
                    ESPressio::Threading::StackCapacity<2048U>,
                    ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::Low>,
                    ESPressio::Threading::Affinity<0U>
                >,
                ESPressio::Threading::Worker<
                    ESPressio::Threading::StackCapacity<4096U>,
                    ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::High>,
                    ESPressio::Threading::Affinity<1U>
                >
            >
        >
    >;

    using HeterogeneousFacilityDeclaration = std::tuple_element_t<
        0U,
        typename HeterogeneousPool::Resources
    >;

    using HeterogeneousOwnedRuntime =
        ESPressio::Threading::Detail::TaskFacilityOwnedRuntime<
            HeterogeneousFacilityDeclaration,
            ManagedContextRouter,
            ExecutionContextProvider,
            AtomicByteProvider,
            MutexProvider,
            0U,
            ManagedContextRouter::ContextCapacity
        >;

    using HeterogeneousResolver =
        ESPressio::Threading::Detail::StructuralContextResolver<
            ManagedContextRouter::ContextCapacity,
            HeterogeneousOwnedRuntime
        >;

    static_assert(
        sizeof(HeterogeneousResolver) == sizeof(HeterogeneousOwnedRuntime*),
        "Structural context resolution must retain only static resource references, not a context pointer registry"
    );


    static_assert(
        HeterogeneousOwnedRuntime::WorkerCount == 2U,
        "Owned Task facility runtime must materialize every heterogeneous Worker declaration"
    );


    using FirstHeterogeneousWorker =
        ESPressio::Threading::Detail::FacilityWorkerDescriptor<
            HeterogeneousPool,
            0U,
            0U
        >;

    using SecondHeterogeneousWorker =
        ESPressio::Threading::Detail::FacilityWorkerDescriptor<
            HeterogeneousPool,
            0U,
            1U
        >;

    static_assert(
        FirstHeterogeneousWorker::ContextIndex == 0U &&
        FirstHeterogeneousWorker::Properties::StackCapacity == 2048U &&
        FirstHeterogeneousWorker::Properties::Priority == ESPressio::Threading::ThreadPriority::Low,
        "First heterogeneous Worker must preserve its own topology properties and dense index"
    );

    static_assert(
        SecondHeterogeneousWorker::ContextIndex == 1U &&
        SecondHeterogeneousWorker::Properties::StackCapacity == 4096U &&
        SecondHeterogeneousWorker::Properties::Priority == ESPressio::Threading::ThreadPriority::High,
        "Second heterogeneous Worker must preserve its own topology properties and dense index"
    );


    using TestTaskRecord = ESPressio::Threading::Detail::TaskRecord<
        32U,
        16U,
        8U,
        4U,
        AtomicByteProvider
    >;

    using TestFacilityCore = ESPressio::Threading::Detail::TaskFacilityCore<
        3U,
        32U,
        16U,
        4U,
        AtomicByteProvider
    >;


    using ResolvedWorkerProperties = ESPressio::Threading::ResourceProperties<
        ESPressio::Threading::StackCapacity<4096U>,
        ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::High>,
        ESPressio::Threading::Affinity<1U>
    >;

    static_assert(
        ResolvedWorkerProperties::StackCapacity == 4096U &&
        ResolvedWorkerProperties::Priority == ESPressio::Threading::ThreadPriority::High,
        "Execution resource properties must resolve directly into concrete Platform configuration"
    );


    static_assert(
        ESPressio::Threading::Detail::ValidExecutionResourceProperties<
            ESPressio::Threading::StackCapacity<4096U>,
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::Normal>,
            ESPressio::Threading::AnyAffinity
        >::Value,
        "One declaration per execution-resource property category must remain valid"
    );

    static_assert(
        !ESPressio::Threading::Detail::ValidExecutionResourceProperties<
            ESPressio::Threading::StackCapacity<2048U>,
            ESPressio::Threading::StackCapacity<4096U>
        >::Value,
        "Duplicate StackCapacity declarations must be rejected"
    );

    static_assert(
        !ESPressio::Threading::Detail::ValidExecutionResourceProperties<
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::Low>,
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::High>
        >::Value,
        "Duplicate Priority declarations must be rejected"
    );

    static_assert(
        !ESPressio::Threading::Detail::ValidExecutionResourceProperties<
            ESPressio::Threading::AnyAffinity,
            ESPressio::Threading::Affinity<0U>
        >::Value,
        "Conflicting affinity declarations must be rejected"
    );


    using TopologyPlan = ESPressio::Threading::Detail::StaticTopologyPlan<Topology>;

    static_assert(
        TopologyPlan::Resource<0U>::FirstContextIndex == 0U &&
        TopologyPlan::Resource<0U>::ContextCount == 1U,
        "First Task facility must own the first dense managed-context range"
    );

    static_assert(
        TopologyPlan::Resource<1U>::FirstContextIndex == 1U &&
        TopologyPlan::Resource<1U>::ContextCount == 1U,
        "Dedicated Worker must follow the preceding facility range without a runtime registry"
    );

    static_assert(
        TopologyPlan::Resource<2U>::FirstContextIndex == 2U &&
        TopologyPlan::Resource<2U>::ContextCount == 1U,
        "Dedicated Thread must follow the Dedicated Worker range without a runtime registry"
    );

    static_assert(
        TopologyPlan::ManagedExecutionContextCount == 3U,
        "Static topology plan must preserve the topology-wide managed-context count"
    );


    static_assert(
        Topology::HasTaskExecution,
        "Topology containing a Task facility must advertise Task execution"
    );

    static_assert(
        Topology::HasDedicatedThreadExecution,
        "Topology containing a Dedicated Thread must advertise Dedicated Thread execution"
    );

    static_assert(
        Topology::HasDedicatedWorkerLease &&
        Topology::TaskFacilityCount == 1U &&
        Topology::DedicatedWorkerLeaseCount == 1U &&
        Topology::DedicatedThreadCount == 1U &&
        Topology::ManagedExecutionContextCount == 3U,
        "Topology must expose compile-time execution-resource counts without runtime registry state"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::TaskPoolRequirement<OrdinaryPool>
        >,
        "Topology must satisfy requirements for its configured Task Pool identity"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedWorkerRequirement<ReturningCallable>
        >,
        "Topology must satisfy requirements for its configured DedicatedWorkerLease identity"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedThreadRequirement<TelemetryThread>
        >,
        "Topology must satisfy requirements for its configured Dedicated Thread identity"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::TaskPoolRequirement<
                OrdinaryPool,
                ESPressio::Threading::MinimumStackCapacity<4096U>,
                ESPressio::Threading::MinimumWorkerConcurrency<1U>,
                ESPressio::Threading::MinimumPriority<ESPressio::Threading::ThreadPriority::Normal>,
                ESPressio::Threading::RequiredAffinity<ESPressio::Threading::AnyAffinity>
            >
        >,
        "Task Pool requirements must validate Worker stack/concurrency/priority/affinity"
    );

    static_assert(
        !ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::TaskPoolRequirement<
                OrdinaryPool,
                ESPressio::Threading::MinimumWorkerConcurrency<2U>
            >
        >,
        "Task Pool requirement must reject insufficient Worker concurrency"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedWorkerRequirement<
                ReturningCallable,
                ESPressio::Threading::MinimumStackCapacity<2048U>,
                ESPressio::Threading::MinimumWorkerConcurrency<1U>,
                ESPressio::Threading::MinimumPriority<ESPressio::Threading::ThreadPriority::Critical>,
                ESPressio::Threading::RequiredAffinity<ESPressio::Threading::AnyAffinity>
            >
        >,
        "DedicatedWorkerLease requirements must validate its fixed one-Worker entitlement"
    );

    static_assert(
        !ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedWorkerRequirement<
                ReturningCallable,
                ESPressio::Threading::MinimumWorkerConcurrency<2U>
            >
        >,
        "DedicatedWorkerLease must reject requirements above its fixed cardinality of one"
    );

    static_assert(
        ESPressio::Threading::SatisfiesThreadingRequirement<
            Topology,
            ESPressio::Threading::DedicatedThreadRequirement<
                TelemetryThread,
                ESPressio::Threading::MinimumStackCapacity<4096U>,
                ESPressio::Threading::MinimumPriority<ESPressio::Threading::ThreadPriority::High>,
                ESPressio::Threading::RequiredAffinity<ESPressio::Threading::AnyAffinity>
            >
        >,
        "Dedicated Thread requirements must validate stack/priority/affinity"
    );


    static_assert(
        sizeof(ESPressio::Threading::Detail::TaskControl<AtomicByteProvider>) == 1U,
        "Task intrinsic control must remain one byte"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::DedicatedThreadControl<AtomicByteProvider>) == 1U,
        "Dedicated Thread intrinsic control must remain one byte"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::AvailabilityBitmap<10U>) == 2U,
        "Ten Task availability bits must occupy exactly two bytes"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::IntrusiveTaskQueue<3U>) == 2U,
        "Three-record Task queue endpoints must occupy two one-byte indices"
    );

    static_assert(
        TestTaskRecord::PayloadCapacity == 32U,
        "Task record payload must use the larger callable/result capacity"
    );

    using ContextIndex = ESPressio::Threading::Detail::ExecutionContextIndexTraits<4U>::Type;

    using TaskRegistration = ESPressio::Threading::Detail::TaskWaitRegistration<
        ESPressio::Threading::Detail::SmallestIndex<8U>::Type,
        ContextIndex
    >;

    static_assert(
        sizeof(ContextIndex) == 1U,
        "Four managed execution contexts must use a one-byte wake-routing index"
    );

    static_assert(
        sizeof(ESPressio::Threading::Detail::RegistrationSet<TaskRegistration, 4U>) ==
        sizeof(TaskRegistration) * 4U,
        "RegistrationSet must contain only target-owned registration records"
    );

    using TestFacilityRuntime = ESPressio::Threading::Detail::TaskFacilityRuntime<
        3U,
        32U,
        16U,
        1U,
        1U,
        ManagedContextRouter::ContextCapacity,
        AtomicByteProvider,
        MutexProvider,
        ManagedContextRouter
    >;


    using TestWorkerExecutionContext = ESPressio::Threading::Detail::TaskWorkerExecutionContext<
        ExecutionContextProvider,
        100U,
        TestFacilityRuntime,
        ManagedContextRouter
    >;


    static_assert(
        TestWorkerExecutionContext::ControlBackingBytes() == 24U,
        "Worker control backing must follow provider-declared physical storage"
    );

    static_assert(
        TestWorkerExecutionContext::StackBackingBytes() == 128U,
        "Worker stack backing must round semantic capacity to provider granularity"
    );


    using TestDedicatedWorkerLeaseRuntime = ESPressio::Threading::Detail::DedicatedWorkerLeaseRuntime<
        ReturningCallable,
        3U,
        32U,
        16U,
        100U,
        1U,
        ManagedContextRouter::ContextCapacity,
        AtomicByteProvider,
        MutexProvider,
        ExecutionContextProvider,
        ManagedContextRouter
    >;


    static_assert(
        TestDedicatedWorkerLeaseRuntime::WorkerCount == 1U,
        "DedicatedWorkerLease cardinality must remain exactly one Worker"
    );

    static_assert(
        TestDedicatedWorkerLeaseRuntime::RecordCapacity() == 3U,
        "DedicatedWorkerLease must retain its explicitly bounded isolated FIFO population"
    );


    using TestDedicatedThreadRuntime = ESPressio::Threading::Detail::DedicatedThreadRuntime<
        DedicatedThreadIdentity,
        DedicatedCallable,
        100U,
        ManagedContextRouter::ContextCapacity,
        AtomicByteProvider,
        MutexProvider,
        ExecutionContextProvider,
        ManagedContextRouter
    >;


    using DedicatedWorkerDeclaration = ESPressio::Threading::DedicatedWorkerLease<
        ReturningCallable,
        ESPressio::Threading::TaskRecordCapacity<3U>,
        ESPressio::Threading::CallableCapacity<32U>,
        ESPressio::Threading::ResultCapacity<16U>,
        ESPressio::Threading::StackCapacity<100U>,
        ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::Critical>,
        ESPressio::Threading::AnyAffinity
    >;

    static_assert(
        DedicatedWorkerDeclaration::RecordCapacity == 3U &&
        DedicatedWorkerDeclaration::CallableStorageCapacity == 32U &&
        DedicatedWorkerDeclaration::ResultStorageCapacity == 16U,
        "Dedicated Worker declaration must resolve bounded facility storage"
    );

    using TestDedicatedWorkerOwnedRuntime =
        ESPressio::Threading::Detail::DedicatedWorkerOwnedRuntime<
            DedicatedWorkerDeclaration,
            ManagedContextRouter,
            ExecutionContextProvider,
            AtomicByteProvider,
            MutexProvider,
            1U,
            ManagedContextRouter::ContextCapacity
        >;

    using DedicatedThreadDeclaration = ESPressio::Threading::DedicatedThread<
        DedicatedThreadIdentity,
        ESPressio::Threading::StackCapacity<100U>,
        ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::High>,
        ESPressio::Threading::AnyAffinity
    >;

    using DedicatedThreadBindingType = decltype(
        ESPressio::Threading::BindDedicatedThread<DedicatedThreadIdentity>(
            DedicatedCallable{}
        )
    );

    static_assert(
        ESPressio::Threading::Detail::DedicatedThreadBindingCount<
            DedicatedThreadIdentity,
            DedicatedThreadBindingType
        >::Value == 1U,
        "Dedicated Thread callable binding must resolve by semantic Thread identity"
    );


    using TestDedicatedThreadOwnedRuntime =
        ESPressio::Threading::Detail::DedicatedThreadOwnedRuntime<
            DedicatedThreadDeclaration,
            DedicatedCallable,
            ManagedContextRouter,
            ExecutionContextProvider,
            AtomicByteProvider,
            MutexProvider,
            2U,
            ManagedContextRouter::ContextCapacity
        >;


    using EmptyTopology = ESPressio::Threading::ThreadingTopology<>;

    static_assert(
        EmptyTopology::ResourceCount == 0U &&
        EmptyTopology::ManagedExecutionContextCount == 0U,
        "The optional empty Threading topology must remain valid"
    );

    using EmptyBindings = std::tuple<>;

    using EmptyWakeSet =
        ESPressio::Threading::Detail::ManagedContextWakeSet<
            0U,
            SignalProvider
        >;

    using EmptyRouter =
        ESPressio::Threading::Detail::ManagedContextRouter<
            0U,
            SignalProvider
        >;

    using EmptyResolver =
        ESPressio::Threading::Detail::StructuralContextResolver<
            0U
        >;

    static_assert(
        sizeof(EmptyWakeSet) == 1U &&
        sizeof(EmptyRouter) == 1U &&
        sizeof(EmptyResolver) == 1U,
        "Empty Threading topology support must compile away wake/router/resolver storage"
    );


    using EmptyShutdownWait =
        ESPressio::Threading::Detail::ShutdownWaitRuntime<
            ESPressio::Threading::Detail::InfrastructureLifecycle<AtomicByteProvider>,
            0U,
            MutexProvider,
            EmptyRouter
        >;

    static_assert(
        sizeof(EmptyShutdownWait) == sizeof(void*),
        "Empty Threading topology must not reserve waiter or mutex provider storage"
    );


    using EmptyOwner =
        ESPressio::Threading::Detail::StaticTopologyOwner<
            EmptyTopology,
            EmptyBindings,
            SignalProvider,
            ExecutionContextProvider,
            AtomicByteProvider,
            MutexProvider
        >;


    using MixedOwnedTopology = ESPressio::Threading::ThreadingTopology<
        std::tuple_element_t<
            0U,
            typename HeterogeneousPool::Resources
        >,
        DedicatedWorkerDeclaration,
        DedicatedThreadDeclaration
    >;

    using MixedOwnedBindings = std::tuple<
        DedicatedThreadBindingType
    >;

    using PublicMixedRuntime =
        ESPressio::Threading::StaticThreadingRuntime<
            MixedOwnedTopology,
            MixedOwnedBindings,
            SignalProvider,
            ExecutionContextProvider,
            AtomicByteProvider,
            MutexProvider
        >;

    static_assert(
        !std::is_copy_constructible_v<PublicMixedRuntime> &&
        !std::is_move_constructible_v<PublicMixedRuntime>,
        "Static Threading runtime ownership must remain address-stable"
    );


    using MixedOwnedResources =
        ESPressio::Threading::Detail::OwnedResourceTuple<
            MixedOwnedTopology,
            MixedOwnedBindings,
            ManagedContextRouter,
            ExecutionContextProvider,
            AtomicByteProvider,
            MutexProvider
        >;

    static_assert(
        !ESPressio::Threading::Detail::ValidDedicatedThreadBindings<
            MixedOwnedTopology,
            std::tuple<UnknownExecutionProperty>
        >::Value,
        "Dedicated Thread binding tuples must reject arbitrary non-binding Types"
    );

    static_assert(
        ESPressio::Threading::Detail::ValidDedicatedThreadBindings<
            MixedOwnedTopology,
            MixedOwnedBindings
        >::Value,
        "Every Dedicated Thread must have exactly one declared callable binding"
    );

    static_assert(
        !ESPressio::Threading::Detail::ValidExecutionResourceProperties<
            UnknownExecutionProperty
        >::Value,
        "Unknown execution resource properties must be rejected"
    );

    static_assert(
        ESPressio::Threading::Detail::ValidExecutionResourceProperties<
            ESPressio::Threading::StackCapacity<2048U>,
            ESPressio::Threading::Priority<ESPressio::Threading::ThreadPriority::High>,
            ESPressio::Threading::Affinity<1U>
        >::Value,
        "Known execution resource properties must remain valid"
    );


    static_assert(
        !ESPressio::Threading::Detail::ValidDedicatedThreadBindings<
            MixedOwnedTopology,
            std::tuple<
                DedicatedThreadBindingType,
                ESPressio::Threading::DedicatedThreadBinding<
                    UndeclaredThreadIdentity,
                    DedicatedCallable
                >
            >
        >::Value,
        "Callable bindings for undeclared Dedicated Threads must be rejected"
    );


    static_assert(
        ESPressio::Threading::Detail::TaskFacilityResourceIndex<
            MixedOwnedTopology,
            OrdinaryPool
        > == 0U,
        "Task facilities must be addressable by semantic Pool identity"
    );

    static_assert(
        ESPressio::Threading::Detail::DedicatedWorkerResourceIndex<
            MixedOwnedTopology,
            ReturningCallable
        > == 1U,
        "Dedicated Worker leases must be addressable by semantic Task identity"
    );

    static_assert(
        ESPressio::Threading::Detail::DedicatedThreadResourceIndex<
            MixedOwnedTopology,
            DedicatedThreadIdentity
        > == 2U,
        "Dedicated Threads must be addressable by semantic Thread identity"
    );


    using MixedActualRouter =
        ESPressio::Threading::Detail::ManagedContextRouter<
            MixedOwnedTopology::ManagedExecutionContextCount,
            SignalProvider
        >;

    using MixedOwnedStorage =
        ESPressio::Threading::Detail::StaticTopologyResourceStorage<
            MixedOwnedTopology,
            MixedOwnedBindings,
            MixedActualRouter,
            ExecutionContextProvider,
            AtomicByteProvider,
            MutexProvider,
            0U
        >;


    static_assert(
        std::tuple_size_v<MixedOwnedResources> == 3U,
        "Every heterogeneous topology declaration must map to exactly one owned runtime resource"
    );

    static_assert(
        std::is_same_v<
            std::tuple_element_t<1U, MixedOwnedResources>,
            ESPressio::Threading::Detail::DedicatedWorkerOwnedRuntime<
                DedicatedWorkerDeclaration,
                ManagedContextRouter,
                ExecutionContextProvider,
                AtomicByteProvider,
                MutexProvider,
                2U,
                MixedOwnedTopology::ManagedExecutionContextCount
            >
        >,
        "Dedicated Worker ownership must use its topology-derived dense context offset"
    );

    static_assert(
        std::is_same_v<
            std::tuple_element_t<2U, MixedOwnedResources>,
            ESPressio::Threading::Detail::DedicatedThreadOwnedRuntime<
                DedicatedThreadDeclaration,
                DedicatedCallable,
                ManagedContextRouter,
                ExecutionContextProvider,
                AtomicByteProvider,
                MutexProvider,
                3U,
                MixedOwnedTopology::ManagedExecutionContextCount
            >
        >,
        "Dedicated Thread ownership must use its semantic callable binding and dense context offset"
    );


    static_assert(
        TestDedicatedThreadRuntime::ControlBackingBytes() == 24U,
        "Dedicated Thread control backing must follow provider-declared physical storage"
    );

    static_assert(
        TestDedicatedThreadRuntime::StackBackingBytes() == 128U,
        "Dedicated Thread stack backing must round semantic capacity to provider granularity"
    );


    using TestWorkerScheduler = ESPressio::Threading::Detail::WorkerLeaseScheduler<
        3U,
        2U,
        8U
    >;

    static_assert(
        sizeof(TestWorkerScheduler) == 1U,
        "Three Worker availability bits must occupy one byte"
    );

} // Test


/// Exercises compact Threading foundation primitives.
int main() {
    Test::EmptyOwner emptyOwner(
        Test::EmptyBindings{}
    );

    assert(
        emptyOwner.Initialize() ==
        ESPressio::Threading::ThreadingInitializationResult::Succeeded
    );

    assert(
        emptyOwner.Start() ==
        ESPressio::Threading::ThreadingStartResult::Succeeded
    );

    assert(
        emptyOwner.BeginShutdown() ==
        ESPressio::Threading::ThreadingShutdownResult::Accepted
    );

    assert(
        emptyOwner.IsExecutionQuiescent()
    );

    assert(
        emptyOwner.FinalizeShutdown() ==
        ESPressio::Threading::ThreadingFinalizationResult::Completed
    );

    assert(
        emptyOwner.WaitForShutdown() ==
        ESPressio::Threading::ShutdownWaitResult::Completed
    );

    ESPressio::Threading::Detail::ManagedContextWakeSet<
        Test::MixedOwnedTopology::ManagedExecutionContextCount,
        Test::SignalProvider
    > mixedWakeSet;

    Test::MixedActualRouter mixedRouter(
        mixedWakeSet
    );

    ESPressio::Threading::Detail::InfrastructureLifecycle<
        Test::AtomicByteProvider
    > mixedLifecycle;

    Test::MixedOwnedBindings mixedBindings(
        ESPressio::Threading::BindDedicatedThread<Test::DedicatedThreadIdentity>(
            Test::DedicatedCallable{}
        )
    );

    Test::MixedOwnedStorage mixedStorage(
        mixedBindings,
        mixedRouter,
        mixedLifecycle
    );

    static_cast<void>(
        mixedStorage.template Get<0U>()
    );
    static_cast<void>(
        mixedStorage.template Get<1U>()
    );
    static_cast<void>(
        mixedStorage.template Get<2U>()
    );

    ESPressio::Threading::Detail::StaticTopologyOwner<
        Test::MixedOwnedTopology,
        Test::MixedOwnedBindings,
        Test::SignalProvider,
        Test::ExecutionContextProvider,
        Test::AtomicByteProvider,
        Test::MutexProvider
    > mixedOwner(
        Test::MixedOwnedBindings(
            ESPressio::Threading::BindDedicatedThread<Test::DedicatedThreadIdentity>(
                Test::DedicatedCallable{}
            )
        )
    );

    const auto gatedMixedDispatch = mixedOwner.template Dispatch<Test::OrdinaryPool>(
        []() {},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        gatedMixedDispatch.Status() ==
        ESPressio::Threading::TaskDispatchStatus::ShuttingDown
    );

    assert(
        mixedOwner.template StartThread<Test::DedicatedThreadIdentity>() ==
        ESPressio::Threading::ThreadStartResult::ShuttingDown
    );

    assert(
        mixedOwner.BeginShutdown() ==
        ESPressio::Threading::ThreadingShutdownResult::NotStarted
    );

    auto movedThreadHandle =
        mixedOwner.template ThreadHandle<Test::DedicatedThreadIdentity>();

    auto validThreadHandle = std::move(
        movedThreadHandle
    );

    assert(
        !movedThreadHandle.IsValid()
    );

    assert(
        movedThreadHandle.State() ==
        ESPressio::Threading::ThreadState::NeverStarted
    );

    assert(
        movedThreadHandle.RequestStop() ==
        ESPressio::Threading::ThreadStopRequestResult::NotRunning
    );

    assert(
        movedThreadHandle.Join() ==
        ESPressio::Threading::ThreadJoinResult::NeverStarted
    );

    assert(
        validThreadHandle.IsValid()
    );

    ESPressio::Threading::Detail::TaskControl<Test::AtomicByteProvider> control;

    control.InitializeQueued();

    assert(
        control.State() ==
        ESPressio::Threading::Detail::TaskOperationalState::Queued
    );

    assert(
        control.HasOwner()
    );

    const auto firstPhase = control.Phase();

    control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::RunningCancelRequested
    );

    assert(
        control.IsCancellationRequested()
    );

    control.ReleaseOwner();

    assert(
        !control.HasOwner()
    );

    control.InitializeQueued();

    assert(
        control.Phase() != firstPhase
    );


    ESPressio::Threading::Detail::AvailabilityBitmap<10U> availability;
    std::size_t claimedIndex = 0U;

    for (std::size_t expectedIndex = 0U; expectedIndex < 10U; ++expectedIndex) {
        assert(
            availability.TryClaim(
                claimedIndex
            )
        );

        assert(
            claimedIndex == expectedIndex
        );
    }

    assert(
        !availability.TryClaim(
            claimedIndex
        )
    );

    availability.Release(
        4U
    );

    assert(
        availability.IsAvailable(
            4U
        )
    );

    assert(
        availability.TryClaim(
            claimedIndex
        )
    );

    assert(
        claimedIndex == 4U
    );


    Test::QueueRecord records[3U];
    ESPressio::Threading::Detail::IntrusiveTaskQueue<3U> queue;

    queue.Push(
        records,
        0U
    );

    queue.Push(
        records,
        1U
    );

    queue.Push(
        records,
        2U
    );

    assert(
        queue.Head() == 0U
    );

    assert(
        queue.Remove(
            records,
            1U
        )
    );

    assert(
        queue.Pop(
            records
        ) == 0U
    );

    assert(
        queue.Pop(
            records
        ) == 2U
    );

    assert(
        queue.Pop(
            records
        ) ==
        ESPressio::Threading::Detail::IntrusiveTaskQueue<3U>::InvalidIndex
    );


    ESPressio::Threading::Detail::DedicatedThreadControl<Test::AtomicByteProvider> threadControl;
    bool activationPhase = false;

    assert(
        threadControl.TryStart(
            activationPhase
        )
    );

    assert(
        threadControl.State() ==
        ESPressio::Threading::Detail::DedicatedThreadOperationalState::Running
    );

    assert(
        threadControl.TryRequestStop()
    );

    assert(
        threadControl.IsStopRequested()
    );

    assert(
        threadControl.TryPublishStopped(
            activationPhase
        )
    );

    assert(
        threadControl.State() ==
        ESPressio::Threading::Detail::DedicatedThreadOperationalState::Stopped
    );

    const auto firstActivationPhase = activationPhase;

    assert(
        threadControl.TryStart(
            activationPhase
        )
    );

    assert(
        activationPhase != firstActivationPhase
    );


    Test::TestTaskRecord completedRecord;
    completedRecord.Control.InitializeQueued();
    completedRecord.Control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::Running
    );

    using ReturningAdapter = ESPressio::Threading::Detail::TaskPayloadAdapter<
        Test::TestTaskRecord,
        Test::ReturningCallable,
        int
    >;

    new (completedRecord.Payload) Test::ReturningCallable();
    completedRecord.PayloadOperations = &ReturningAdapter::Operations;

    assert(
        completedRecord.PayloadOperations->Invoke(
            completedRecord
        ) == ESPressio::Threading::Detail::TaskInvocationOutcome::Completed
    );

    assert(
        completedRecord.Control.State() ==
        ESPressio::Threading::Detail::TaskOperationalState::Running
    );

    completedRecord.Control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::Completed
    );

    alignas(int) std::byte resultStorage[sizeof(int)];

    completedRecord.PayloadOperations->MoveResult(
        completedRecord,
        resultStorage
    );

    assert(
        *reinterpret_cast<int*>(resultStorage) == 42
    );


    Test::TestTaskRecord cancelledRecord;
    cancelledRecord.Control.InitializeQueued();
    cancelledRecord.Control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::RunningCancelRequested
    );

    using CancellationAdapter = ESPressio::Threading::Detail::TaskPayloadAdapter<
        Test::TestTaskRecord,
        Test::CancellationAwareCallable,
        int
    >;

    new (cancelledRecord.Payload) Test::CancellationAwareCallable();
    cancelledRecord.PayloadOperations = &CancellationAdapter::Operations;

    assert(
        cancelledRecord.PayloadOperations->Invoke(
            cancelledRecord
        ) == ESPressio::Threading::Detail::TaskInvocationOutcome::Cancelled
    );

    assert(
        cancelledRecord.Control.State() ==
        ESPressio::Threading::Detail::TaskOperationalState::RunningCancelRequested
    );

    cancelledRecord.Control.SetState(
        ESPressio::Threading::Detail::TaskOperationalState::Cancelled
    );


    Test::TestFacilityCore facility;

    auto completedAdmission = facility.Admit(
        Test::ReturningCallable{}
    );

    assert(
        completedAdmission.IsAdmitted()
    );

    assert(
        facility.RecordsInUse() == 1U
    );

    assert(
        facility.QueuedTasks() == 1U
    );

    assert(
        facility.PublicState(
            completedAdmission.Binding()->RecordIndex,
            completedAdmission.Binding()->Phase
        ) == ESPressio::Threading::TaskState::Queued
    );

    const auto completedClaim = facility.ClaimNextForWorker(
        0U
    );

    assert(
        completedClaim.IsClaimed()
    );

    assert(
        completedClaim.Binding()->RecordIndex == completedAdmission.Binding()->RecordIndex
    );

    assert(
        facility.PublicState(
            completedClaim.Binding()->RecordIndex,
            completedClaim.Binding()->Phase
        ) == ESPressio::Threading::TaskState::Running
    );

    const auto completedOutcome = facility.Invoke(
        completedClaim.Binding()->RecordIndex,
        completedClaim.Binding()->Phase
    );

    assert(
        completedOutcome == ESPressio::Threading::Detail::TaskInvocationOutcome::Completed
    );

    facility.PublishInvocationOutcome(
        completedClaim.Binding()->RecordIndex,
        completedClaim.Binding()->Phase,
        completedOutcome
    );

    assert(
        facility.PublicState(
            completedClaim.Binding()->RecordIndex,
            completedClaim.Binding()->Phase
        ) == ESPressio::Threading::TaskState::Completed
    );

    alignas(int) std::byte facilityResultStorage[sizeof(int)];

    assert(
        facility.TakeResult(
            completedClaim.Binding()->RecordIndex,
            completedClaim.Binding()->Phase,
            facilityResultStorage
        ) == ESPressio::Threading::TaskTakeStatus::Succeeded
    );

    assert(
        *reinterpret_cast<int*>(facilityResultStorage) == 42
    );

    assert(
        facility.Reclaim(
            completedClaim.Binding()->RecordIndex,
            completedClaim.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::Reclaimed
    );

    assert(
        facility.RecordsInUse() == 0U
    );


    auto queuedCancellationAdmission = facility.Admit(
        Test::ReturningCallable{}
    );

    const auto queuedCancellation = facility.Cancel(
        queuedCancellationAdmission.Binding()->RecordIndex,
        queuedCancellationAdmission.Binding()->Phase
    );

    assert(
        queuedCancellation.Result() == ESPressio::Threading::TaskCancelResult::Accepted
    );

    assert(
        queuedCancellation.IsTerminalPublished()
    );

    assert(
        facility.QueuedTasks() == 0U
    );

    assert(
        facility.Reclaim(
            queuedCancellationAdmission.Binding()->RecordIndex,
            queuedCancellationAdmission.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::NotEligible
    );

    assert(
        facility.ReleaseOwner(
            queuedCancellationAdmission.Binding()->RecordIndex,
            queuedCancellationAdmission.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReleaseEffect::OwnershipReleased
    );

    assert(
        facility.Reclaim(
            queuedCancellationAdmission.Binding()->RecordIndex,
            queuedCancellationAdmission.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::Reclaimed
    );


    auto cooperativeAdmission = facility.Admit(
        Test::CancellationAwareCallable{}
    );

    const auto cooperativeClaim = facility.ClaimNextForWorker(
        1U
    );

    assert(
        cooperativeClaim.IsClaimed()
    );

    const auto runningCancellation = facility.Cancel(
        cooperativeClaim.Binding()->RecordIndex,
        cooperativeClaim.Binding()->Phase
    );

    assert(
        runningCancellation.Result() == ESPressio::Threading::TaskCancelResult::Accepted
    );

    assert(
        !runningCancellation.IsTerminalPublished()
    );

    const auto cooperativeOutcome = facility.Invoke(
        cooperativeClaim.Binding()->RecordIndex,
        cooperativeClaim.Binding()->Phase
    );

    assert(
        cooperativeOutcome == ESPressio::Threading::Detail::TaskInvocationOutcome::Cancelled
    );

    facility.PublishInvocationOutcome(
        cooperativeClaim.Binding()->RecordIndex,
        cooperativeClaim.Binding()->Phase,
        cooperativeOutcome
    );

    assert(
        facility.PublicState(
            cooperativeClaim.Binding()->RecordIndex,
            cooperativeClaim.Binding()->Phase
        ) == ESPressio::Threading::TaskState::Cancelled
    );

    facility.ReleaseOwner(
        cooperativeClaim.Binding()->RecordIndex,
        cooperativeClaim.Binding()->Phase
    );

    assert(
        facility.Reclaim(
            cooperativeClaim.Binding()->RecordIndex,
            cooperativeClaim.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::Reclaimed
    );


    auto abandonedRunningAdmission = facility.Admit(
        Test::ReturningCallable{}
    );

    const auto abandonedRunningClaim = facility.ClaimNextForWorker(
        2U
    );

    assert(
        abandonedRunningClaim.IsClaimed()
    );

    assert(
        facility.ReleaseOwner(
            abandonedRunningClaim.Binding()->RecordIndex,
            abandonedRunningClaim.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReleaseEffect::OwnershipReleased
    );

    const auto abandonedRunningOutcome = facility.Invoke(
        abandonedRunningClaim.Binding()->RecordIndex,
        abandonedRunningClaim.Binding()->Phase
    );

    facility.PublishInvocationOutcome(
        abandonedRunningClaim.Binding()->RecordIndex,
        abandonedRunningClaim.Binding()->Phase,
        abandonedRunningOutcome
    );

    assert(
        facility.Reclaim(
            abandonedRunningClaim.Binding()->RecordIndex,
            abandonedRunningClaim.Binding()->Phase
        ) == ESPressio::Threading::Detail::TaskReclaimResult::Reclaimed
    );


    Test::TestWorkerScheduler workerScheduler;

    assert(
        workerScheduler.AvailableCount() == 0U
    );

    assert(
        workerScheduler.MarkAvailable(
            2U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::Available
    );

    assert(
        workerScheduler.MarkAvailable(
            4U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::Available
    );

    assert(
        workerScheduler.MarkAvailable(
            1U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::OutsideFacilityRange
    );

    assert(
        workerScheduler.AvailableCount() == 2U
    );

    const auto firstWorkerLease = workerScheduler.TryClaimAvailable();

    assert(
        firstWorkerLease.has_value()
    );

    assert(
        firstWorkerLease.value() == 2U
    );

    const auto secondWorkerLease = workerScheduler.TryClaimAvailable();

    assert(
        secondWorkerLease.has_value()
    );

    assert(
        secondWorkerLease.value() == 4U
    );

    assert(
        !workerScheduler.TryClaimAvailable().has_value()
    );


    using RegistrationSet = ESPressio::Threading::Detail::RegistrationSet<
        Test::TaskRegistration,
        2U
    >;

    RegistrationSet registrations;
    std::size_t registrationIndex = 0U;

    Test::TaskRegistration firstRegistration;
    firstRegistration.RecordIndex = 3U;
    firstRegistration.Phase = true;
    firstRegistration.WaitingContextIndex = 1U;

    assert(
        registrations.Register(
            firstRegistration,
            registrationIndex
        ) == ESPressio::Threading::Detail::WaitRegistrationStatus::Registered
    );

    assert(
        registrationIndex == 0U
    );

    Test::TaskRegistration secondRegistration;
    secondRegistration.RecordIndex = 3U;
    secondRegistration.Phase = true;
    secondRegistration.WaitingContextIndex = 2U;

    assert(
        registrations.Register(
            secondRegistration,
            registrationIndex
        ) == ESPressio::Threading::Detail::WaitRegistrationStatus::Registered
    );

    assert(
        registrationIndex == 1U
    );

    assert(
        registrations.ActiveCount() == 2U
    );

    Test::TaskRegistration thirdRegistration;
    thirdRegistration.RecordIndex = 4U;
    thirdRegistration.WaitingContextIndex = 3U;

    assert(
        registrations.Register(
            thirdRegistration,
            registrationIndex
        ) == ESPressio::Threading::Detail::WaitRegistrationStatus::CapacityUnavailable
    );

    std::size_t matchingRegistrationCount = 0U;

    registrations.VisitActive(
        [&matchingRegistrationCount](
            const Test::TaskRegistration& registration
        ) {
            if (
                registration.RecordIndex == 3U &&
                registration.Phase
            ) {
                ++matchingRegistrationCount;
            }
        }
    );

    assert(
        matchingRegistrationCount == 2U
    );

    registrations.Unregister(
        0U
    );

    assert(
        registrations.ActiveCount() == 1U
    );

    assert(
        registrations.Register(
            thirdRegistration,
            registrationIndex
        ) == ESPressio::Threading::Detail::WaitRegistrationStatus::Registered
    );

    assert(
        registrationIndex == 0U
    );



    ESPressio::Threading::Detail::ManagedContextWakeSet<
        3U,
        Test::SignalProvider
    > structuralWakeSet;

    const auto currentContextResolver = [](
        const void*
    ) noexcept -> std::optional<std::uint8_t> {
        return static_cast<std::uint8_t>(0U);
    };

    const auto interruptionResolver = [](
        const void*,
        std::uint8_t
    ) noexcept {
        return false;
    };

    ESPressio::Threading::Detail::ManagedContextRouter<
        3U,
        Test::SignalProvider
    > unboundStructuralRouter(
        structuralWakeSet
    );

    assert(
        !unboundStructuralRouter.IsTopologyBound()
    );

    assert(
        !unboundStructuralRouter.CurrentContextIndex().has_value()
    );

    assert(
        unboundStructuralRouter.IsInterrupted(
            0U
        )
    );

    unboundStructuralRouter.BindTopology(
        nullptr,
        currentContextResolver,
        interruptionResolver
    );

    assert(
        !unboundStructuralRouter.IsTopologyBound()
    );


    ESPressio::Threading::Detail::ManagedContextRouter<
        3U,
        Test::SignalProvider
    > structuralRouter(
        structuralWakeSet,
        nullptr,
        currentContextResolver,
        interruptionResolver
    );

    assert(
        structuralRouter.CurrentContextIndex().value() == 0U
    );

    assert(
        structuralRouter.Wake(
            2U
        ) == ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled
    );

    assert(
        structuralRouter.Wait(
            2U,
            ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
        ) == ESPressio::Platform::Synchronization::SignalWaitResult::Signaled
    );


    Test::ManagedContextRouter runtimeRouter;
    assert((
        ESPressio::Threading::Detail::ExecutionControl<
            Test::ExecutionContextProvider,
            Test::ManagedContextRouter
        >::SleepFor(
            runtimeRouter,
            ESPressio::Clock::Duration::FromNanoseconds(
                0
            )
        ) == ESPressio::Threading::SleepResult::Completed
    ));

    ESPressio::Threading::Detail::ExecutionControl<
        Test::ExecutionContextProvider,
        Test::ManagedContextRouter
    >::Yield();


    Test::TestFacilityRuntime runtime(
        runtimeRouter
    );

    assert(
        runtime.ValidateSynchronization() ==
        ESPressio::Threading::Detail::TaskFacilitySynchronizationResult::Ready
    );

    const auto unavailableDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        unavailableDispatch.Status() ==
        ESPressio::Threading::TaskDispatchStatus::Unavailable
    );

    assert(
        runtime.WorkerBecameAvailable(
            1U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::Available
    );

    auto immediateDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        immediateDispatch.IsSucceeded()
    );

    auto immediateTask = immediateDispatch.TakeTask();

    assert(
        immediateTask.State() == ESPressio::Threading::TaskState::Running
    );

    const auto immediateBinding = runtime.AssignedTaskForContext(
        1U
    );

    assert(
        immediateBinding.has_value()
    );

    const auto immediateOutcome = runtime.Invoke(
        immediateBinding->RecordIndex,
        immediateBinding->Phase
    );

    runtime.CompleteWorkerTask(
        1U,
        immediateBinding->RecordIndex,
        immediateBinding->Phase,
        immediateOutcome
    );

    assert(
        immediateTask.State() == ESPressio::Threading::TaskState::Completed
    );

    {
        auto result = immediateTask.TakeResult();

        assert(
            result.IsSucceeded()
        );
    }

    assert(
        runtime.WorkerBecameAvailable(
            1U
        ) == ESPressio::Threading::Detail::WorkerAvailabilityResult::Available
    );

    auto firstQueuedDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::Queue
    );

    auto secondQueuedDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::Queue
    );

    assert(
        firstQueuedDispatch.IsSucceeded() &&
        secondQueuedDispatch.IsSucceeded()
    );

    auto firstQueuedTask = firstQueuedDispatch.TakeTask();
    auto secondQueuedTask = secondQueuedDispatch.TakeTask();

    assert(
        firstQueuedTask.State() == ESPressio::Threading::TaskState::Running
    );

    assert(
        secondQueuedTask.State() == ESPressio::Threading::TaskState::Queued
    );

    const auto firstQueuedBinding = runtime.AssignedTaskForContext(
        1U
    );

    assert(
        firstQueuedBinding.has_value()
    );

    const auto firstQueuedOutcome = runtime.Invoke(
        firstQueuedBinding->RecordIndex,
        firstQueuedBinding->Phase
    );

    runtime.CompleteWorkerTask(
        1U,
        firstQueuedBinding->RecordIndex,
        firstQueuedBinding->Phase,
        firstQueuedOutcome
    );

    assert(
        secondQueuedTask.State() == ESPressio::Threading::TaskState::Running
    );

    const auto secondQueuedBinding = runtime.AssignedTaskForContext(
        1U
    );

    assert(
        secondQueuedBinding.has_value()
    );

    const auto secondQueuedOutcome = runtime.Invoke(
        secondQueuedBinding->RecordIndex,
        secondQueuedBinding->Phase
    );

    runtime.CompleteWorkerTask(
        1U,
        secondQueuedBinding->RecordIndex,
        secondQueuedBinding->Phase,
        secondQueuedOutcome
    );

    {
        auto result = firstQueuedTask.TakeResult();

        assert(
            result.IsSucceeded()
        );
    }

    {
        auto result = secondQueuedTask.TakeResult();

        assert(
            result.IsSucceeded()
        );
    }

    Test::LifetimeCallable::DestructionCount = 0U;
    Test::LifetimeResult::DestructionCount = 0U;

    {
        auto lifetimeDispatch = runtime.Dispatch(
            Test::LifetimeCallable{},
            ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
        );

        assert(
            lifetimeDispatch.IsSucceeded()
        );

        auto lifetimeTask = lifetimeDispatch.TakeTask();
        const auto lifetimeBinding = runtime.AssignedTaskForContext(
            1U
        );

        assert(
            lifetimeBinding.has_value()
        );

        const auto lifetimeOutcome = runtime.Invoke(
            lifetimeBinding->RecordIndex,
            lifetimeBinding->Phase
        );

        assert(
            Test::LifetimeCallable::DestructionCount == 1U
        );

        runtime.CompleteWorkerTask(
            1U,
            lifetimeBinding->RecordIndex,
            lifetimeBinding->Phase,
            lifetimeOutcome
        );

        assert(
            Test::LifetimeResult::DestructionCount == 0U
        );
    }

    assert(
        Test::LifetimeResult::DestructionCount == 1U
    );



    auto occupyingDispatch = runtime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        occupyingDispatch.IsSucceeded()
    );

    auto occupyingTask = occupyingDispatch.TakeTask();

    Test::LifetimeCallable::DestructionCount = 0U;

    auto cancelledQueuedDispatch = runtime.Dispatch(
        Test::LifetimeCallable{},
        ESPressio::Threading::TaskDispatchPolicy::Queue
    );

    assert(
        cancelledQueuedDispatch.IsSucceeded()
    );

    auto cancelledQueuedTask = cancelledQueuedDispatch.TakeTask();

    assert(
        cancelledQueuedTask.State() == ESPressio::Threading::TaskState::Queued
    );

    assert(
        cancelledQueuedTask.Cancel() ==
        ESPressio::Threading::TaskCancelResult::Accepted
    );

    assert(
        cancelledQueuedTask.State() == ESPressio::Threading::TaskState::Cancelled
    );

    assert(
        Test::LifetimeCallable::DestructionCount == 1U
    );

    const auto occupyingBinding = runtime.AssignedTaskForContext(
        1U
    );

    assert(
        occupyingBinding.has_value()
    );

    const auto occupyingOutcome = runtime.Invoke(
        occupyingBinding->RecordIndex,
        occupyingBinding->Phase
    );

    runtime.CompleteWorkerTask(
        1U,
        occupyingBinding->RecordIndex,
        occupyingBinding->Phase,
        occupyingOutcome
    );

    {
        auto result = occupyingTask.TakeResult();

        assert(
            result.IsSucceeded()
        );
    }

    ESPressio::Threading::Detail::InfrastructureLifecycle<
        Test::AtomicByteProvider
    > lifecycle;

    assert(
        lifecycle.CommitInitialization() ==
        ESPressio::Threading::ThreadingInitializationResult::Succeeded
    );

    Test::LifecycleResource firstLifecycleResource;
    Test::LifecycleResource failingLifecycleResource(true);
    Test::LifecycleResource laterLifecycleResource;

    assert(
        lifecycle.Start(
            firstLifecycleResource,
            failingLifecycleResource,
            laterLifecycleResource
        ) == ESPressio::Threading::ThreadingStartResult::ProviderFailure
    );

    assert(
        firstLifecycleResource.StartCount == 1U &&
        firstLifecycleResource.TerminationWakeCount == 1U &&
        firstLifecycleResource.JoinCount == 1U &&
        firstLifecycleResource.DestroyCount == 1U
    );

    assert(
        failingLifecycleResource.StartCount == 1U &&
        failingLifecycleResource.DestroyCount == 1U
    );

    assert(
        laterLifecycleResource.StartCount == 0U &&
        laterLifecycleResource.DestroyCount == 1U
    );

    assert(
        lifecycle.ShouldTerminate()
    );

    ESPressio::Threading::Detail::InfrastructureLifecycle<
        Test::AtomicByteProvider
    > successfulLifecycle;

    assert(
        successfulLifecycle.CommitInitialization() ==
        ESPressio::Threading::ThreadingInitializationResult::Succeeded
    );

    Test::LifecycleResource successfulResource;

    assert(
        successfulLifecycle.Start(
            successfulResource
        ) == ESPressio::Threading::ThreadingStartResult::Succeeded
    );

    assert(
        successfulLifecycle.CanActivate()
    );

    assert(
        successfulLifecycle.BeginShutdown() ==
        ESPressio::Threading::ThreadingShutdownResult::Accepted
    );

    assert(
        successfulLifecycle.ShouldTerminate()
    );

    successfulResource.RequestInfrastructureTermination();

    assert(
        successfulResource.JoinInfrastructure(
            ESPressio::Platform::Synchronization::WaitTimeout::Forever()
        ) == ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded
    );

    assert(
        successfulResource.DestroyInfrastructure() ==
        ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded
    );

    successfulLifecycle.PublishShutdownComplete();

    assert(
        successfulLifecycle.BeginShutdown() ==
        ESPressio::Threading::ThreadingShutdownResult::AlreadyCompleted
    );


    ESPressio::Threading::Detail::ThreadingBootstrap<
        Test::AtomicByteProvider
    > bootstrap;

    auto gatedBeforeStart = bootstrap.Dispatch(
        runtime,
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        gatedBeforeStart.Status() ==
        ESPressio::Threading::TaskDispatchStatus::ShuttingDown
    );

    assert(
        bootstrap.CommitInitialization() ==
        ESPressio::Threading::ThreadingInitializationResult::Succeeded
    );

    Test::LifecycleResource bootstrapInfrastructure;

    assert(
        bootstrap.Start(
            bootstrapInfrastructure
        ) == ESPressio::Threading::ThreadingStartResult::Succeeded
    );

    auto gatedOperationalDispatch = bootstrap.Dispatch(
        runtime,
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        gatedOperationalDispatch.Status() !=
        ESPressio::Threading::TaskDispatchStatus::ShuttingDown
    );


    ESPressio::Threading::Detail::InfrastructureLifecycle<
        Test::AtomicByteProvider
    > coordinatedLifecycle;

    assert(
        coordinatedLifecycle.CommitInitialization() ==
        ESPressio::Threading::ThreadingInitializationResult::Succeeded
    );

    Test::LifecycleResource coordinatedInfrastructureResource;

    assert(
        coordinatedLifecycle.Start(
            coordinatedInfrastructureResource
        ) == ESPressio::Threading::ThreadingStartResult::Succeeded
    );

    Test::ShutdownTaskResource shutdownTaskResource;
    Test::ShutdownThreadResource shutdownThreadResource;

    auto shutdownTaskResources = std::tie(
        shutdownTaskResource
    );

    auto shutdownThreadResources = std::tie(
        shutdownThreadResource
    );

    assert(
        ESPressio::Threading::Detail::ShutdownCoordinator::Begin(
            coordinatedLifecycle,
            shutdownTaskResources,
            shutdownThreadResources
        ) == ESPressio::Threading::ThreadingShutdownResult::Accepted
    );

    assert(
        shutdownTaskResource.CancellationStarted &&
        shutdownThreadResource.StopRequested
    );

    assert(
        ESPressio::Threading::Detail::ShutdownCoordinator::IsExecutionQuiescent(
            shutdownTaskResources,
            shutdownThreadResources
        )
    );


    bool dedicatedWorkerShutdown = false;

    const auto dedicatedWorkerShutdownPredicate = [](
        const void* context
    ) noexcept {
        return *static_cast<const bool*>(
            context
        );
    };

    Test::TestDedicatedWorkerLeaseRuntime dedicatedWorkerRuntime(
        runtimeRouter,
        &dedicatedWorkerShutdown,
        dedicatedWorkerShutdownPredicate
    );

    assert(
        dedicatedWorkerRuntime.Initialize(
            ESPressio::Platform::Execution::ExecutionPriority::Critical,
            ESPressio::Platform::Execution::ProcessorAffinity::Any(),
            "test-critical-worker"
        ) == ESPressio::Threading::Detail::WorkerExecutionInitializationResult::Succeeded
    );

    const auto dedicatedUnavailable = dedicatedWorkerRuntime.Dispatch(
        Test::ReturningCallable{},
        ESPressio::Threading::TaskDispatchPolicy::AbandonImmediately
    );

    assert(
        dedicatedUnavailable.Status() ==
        ESPressio::Threading::TaskDispatchStatus::Unavailable
    );

    assert(
        dedicatedWorkerRuntime.DestroyInfrastructure() ==
        ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded
    );


    bool dedicatedCanActivate = false;
    bool dedicatedShouldTerminate = false;

    const auto canActivatePredicate = [](
        const void* context
    ) noexcept {
        return *static_cast<const bool*>(
            context
        );
    };

    const auto shouldTerminatePredicate = [](
        const void* context
    ) noexcept {
        return *static_cast<const bool*>(
            context
        );
    };

    Test::TestDedicatedThreadRuntime dedicatedThreadRuntime(
        Test::DedicatedCallable{},
        runtimeRouter,
        2U,
        &dedicatedCanActivate,
        canActivatePredicate,
        shouldTerminatePredicate
    );

    // The test predicates share one context pointer in production. For this compile/lifecycle
    // surface test both are false, so semantic activation remains unavailable and destruction safe.
    static_cast<void>(
        dedicatedShouldTerminate
    );

    assert(
        dedicatedThreadRuntime.Initialize(
            ESPressio::Platform::Execution::ExecutionPriority::High,
            ESPressio::Platform::Execution::ProcessorAffinity::Any(),
            "test-dedicated"
        ) == ESPressio::Threading::Detail::WorkerExecutionInitializationResult::Succeeded
    );

    auto dedicatedThreadHandle = dedicatedThreadRuntime.Handle();

    assert(
        dedicatedThreadHandle.State() ==
        ESPressio::Threading::ThreadState::NeverStarted
    );

    assert(
        dedicatedThreadHandle.Start() ==
        ESPressio::Threading::ThreadStartResult::ShuttingDown
    );

    assert(
        dedicatedThreadHandle.RequestStop() ==
        ESPressio::Threading::ThreadStopRequestResult::NotRunning
    );

    assert(
        dedicatedThreadHandle.Join() ==
        ESPressio::Threading::ThreadJoinResult::NeverStarted
    );

    assert(
        dedicatedThreadRuntime.DestroyInfrastructure() ==
        ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded
    );


    bool shutdownRequested = false;

    const auto shutdownPredicate = [](
        const void* context
    ) noexcept {
        return *static_cast<const bool*>(
            context
        );
    };

    Test::TestWorkerExecutionContext workerExecutionContext(
        runtime,
        runtimeRouter,
        1U,
        &shutdownRequested,
        shutdownPredicate
    );

    assert(
        workerExecutionContext.Initialize(
            ESPressio::Platform::Execution::ExecutionPriority::Normal,
            ESPressio::Platform::Execution::ProcessorAffinity::Any(),
            "test-worker"
        ) == ESPressio::Threading::Detail::WorkerExecutionInitializationResult::Succeeded
    );

    assert(
        workerExecutionContext.Start() ==
        ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
    );

    assert(
        workerExecutionContext.Join(
            ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
        ) == ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded
    );

    assert(
        workerExecutionContext.Destroy() ==
        ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded
    );

    return 0;
}
