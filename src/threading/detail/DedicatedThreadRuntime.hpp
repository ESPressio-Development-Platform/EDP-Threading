#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>

#include "../Thread.hpp"
#include "DedicatedThreadControl.hpp"
#include "TaskFacilityRuntime.hpp"
#include "TaskWorkerExecutionContext.hpp"
#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

    /// Outcome of validating Dedicated Thread synchronization resources.
    enum class DedicatedThreadSynchronizationResult : std::uint8_t {
        Ready = 0,
        ProviderFailure = 1
    };


    /// Outcome of publishing terminal state for one Dedicated Thread activation phase.
    enum class DedicatedThreadStoppedPublicationResult : std::uint8_t {
        Published = 0,
        StaleActivation = 1,
        ProviderFailure = 2
    };


    /// Detects whether a Dedicated Thread callable returns void when invoked without ThreadContext.
    /// @tparam TCallable Callable Type being inspected.
    /// @tparam TEnable SFINAE helper Type used to select the invocable specialization.
    template<class TCallable, class TEnable = void>
    struct IsVoidThreadCallableWithoutContext final {

        /// Compile-time result produced by this trait specialization.
        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsVoidThreadCallableWithoutContext`.
    /// @tparam TCallable Callable Type being invoked, stored, or adapted.
    template<class TCallable>
    struct IsVoidThreadCallableWithoutContext<
        TCallable,
        std::void_t<
            std::invoke_result_t<TCallable&>
        >
    > final {

        /// Compile-time result produced by this trait specialization.
        static constexpr bool Value = std::is_same_v<
            std::invoke_result_t<TCallable&>,
            void
        >;

    };


    /// Detects whether a Dedicated Thread callable returns void when invoked with ThreadContext.
    /// @tparam TCallable Callable Type being inspected.
    /// @tparam TEnable SFINAE helper Type used to select the invocable specialization.
    template<class TCallable, class TEnable = void>
    struct IsVoidThreadCallableWithContext final {

        /// Compile-time result produced by this trait specialization.
        static constexpr bool Value = false;

    };


    /// Defines the compile-time contract for `IsVoidThreadCallableWithContext`.
    /// @tparam TCallable Callable Type being invoked, stored, or adapted.
    template<class TCallable>
    struct IsVoidThreadCallableWithContext<
        TCallable,
        std::void_t<
            std::invoke_result_t<
                TCallable&,
                ThreadContext&
            >
        >
    > final {

        /// Compile-time result produced by this trait specialization.
        static constexpr bool Value = std::is_same_v<
            std::invoke_result_t<
                TCallable&,
                ThreadContext&
            >,
            void
        >;

    };


    /// Defines the compile-time contract for `DedicatedThreadRuntime`.
    /// @tparam TThreadIdentity Semantic identity Type of the Dedicated Thread.
    /// @tparam TCallable Callable Type being invoked, stored, or adapted.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TStackCapacity Semantic stack-capacity value requested by the topology.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TThreadIdentity, class TCallable, std::size_t TStackCapacity, std::size_t TExecutionContextCapacity, class TMutexProvider, class TExecutionContextProvider, class TManagedContextRouter>
    class DedicatedThreadRuntime final {

        static_assert(
            IsVoidThreadCallableWithContext<TCallable>::Value ||
            IsVoidThreadCallableWithoutContext<TCallable>::Value,
            "Dedicated Thread callable must return exactly void and accept either ThreadContext& or no arguments"
        );

        private:

            // Compact semantic state.

            /// One-byte authoritative Dedicated Thread lifecycle and activation Phase.
            DedicatedThreadControl _control;

            /// Target-owned Join registrations, bounded by managed execution-context count.
            RegistrationSet<
                ThreadJoinRegistration<
                    typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type
                >,
                TExecutionContextCapacity
            > _joinWaiters;

            /// Dedicated Thread lifecycle serialization.
            TMutexProvider _mutex;


            // Bound behaviour and Platform context.

            /// Concrete callable retained for the full topology lifetime.
            TCallable _callable;

            /// Concrete Platform execution-context provider.
            TExecutionContextProvider _provider;

            /// Caller-owned native control and stack backing.
            ExecutionContextBacking<
                TExecutionContextProvider,
                TStackCapacity
            > _backing;


            // Topology bindings.

            /// Compact Type used to identify one managed execution context.
            using ContextIndex =
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type;

            /// Typed Platform outcome returned when the runtime acquires its serialization mutex.
            using LockAcquireResult =
                ESPressio::Platform::Synchronization::LockAcquireResult;

            /// Typed Platform outcome returned when the runtime releases its serialization mutex.
            using LockReleaseResult =
                ESPressio::Platform::Synchronization::LockReleaseResult;

            /// Non-owning topology wake/interruption router.
            TManagedContextRouter* _router;

            /// Dense topology-wide index of this Dedicated Thread context.
            ContextIndex _contextIndex;

            /// Non-owning global lifecycle context.
            const void* _lifecycleContext;

            /// Predicate permitting semantic activation only after successful infrastructure Start.
            bool (*_canActivate)(const void*) noexcept;

            /// Predicate requesting terminal trampoline exit during rollback/shutdown.
            bool (*_shouldTerminate)(const void*) noexcept;


            // Synchronization.

            /// Acquires the Dedicated Thread serialization mutex indefinitely.
            LockAcquireResult AcquireLock() noexcept {
                return _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                );
            }

            /// Releases the Dedicated Thread serialization mutex.
            LockReleaseResult ReleaseLock() noexcept {
                return _mutex.Release();
            }

            /// Projects the compact internal lifecycle state onto the stable public Dedicated Thread state.
            static ThreadState PublicStateFor(
                DedicatedThreadOperationalState state
            ) noexcept {
                switch (state) {
                    case DedicatedThreadOperationalState::NeverStarted:
                        return ThreadState::NeverStarted;

                    case DedicatedThreadOperationalState::Running:
                    case DedicatedThreadOperationalState::RunningStopRequested:
                        return ThreadState::Running;

                    case DedicatedThreadOperationalState::Stopped:
                    default:
                        return ThreadState::Stopped;
                }
            }

            /// Wakes every managed context registered for the completed activation Phase.
            ESPressio::Platform::Synchronization::SignalNotifyResult WakeJoiners(
                bool phase
            ) {
                auto result =
                    ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled;

                _joinWaiters.VisitActive(
                    [this, phase, &result](
                        auto& registration
                    ) {
                        if (registration.Phase != phase) {
                            return;
                        }

                        const auto wakeResult = _router->Wake(
                            registration.WaitingContextIndex
                        );

                        if (
                            result == ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled &&
                            wakeResult != ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled
                        ) {
                            result = wakeResult;
                        }
                    }
                );

                return result;
            }

            /// Indicates whether the captured activation Phase has reached a stopped state.
            bool IsActivationStopped(
                bool capturedPhase
            ) const noexcept {
                const auto currentPhase = _control.Phase();

                if (currentPhase != capturedPhase) {
                    return true;
                }

                const auto state = _control.State();

                return state == DedicatedThreadOperationalState::Stopped;
            }

            /// Bridges ThreadContext stop observation back to this runtime without exposing its concrete Type.
            static bool IsStopRequestedThunk(
                const void* resource
            ) noexcept {
                return const_cast<DedicatedThreadRuntime*>(
                    static_cast<const DedicatedThreadRuntime*>(
                        resource
                    )
                )->IsStopRequested();
            }

            /// Invokes the bound application callable using the supported Dedicated Thread signature.
            void InvokeCallable() {
                if constexpr (
                    std::is_invocable_v<TCallable&, ThreadContext&>
                ) {
                    ThreadContext context(
                        this,
                        &IsStopRequestedThunk
                    );

                    _callable(
                        context
                    );
                } else {
                    _callable();
                }
            }

            /// Publishes terminal state for the completed activation and wakes matching joiners.
            DedicatedThreadStoppedPublicationResult PublishStopped(
                bool activationPhase
            ) noexcept {
                if (AcquireLock() != LockAcquireResult::Acquired) {
                    return DedicatedThreadStoppedPublicationResult::ProviderFailure;
                }

                const auto publicationResult = _control.TryPublishStopped(
                    activationPhase
                );

                if (
                    publicationResult ==
                        DedicatedThreadControlPublicationResult::Published &&
                    WakeJoiners(
                        activationPhase
                    ) != ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled
                ) {
                    static_cast<void>(
                        ReleaseLock()
                    );
                    return DedicatedThreadStoppedPublicationResult::ProviderFailure;
                }

                static_cast<void>(
                    ReleaseLock()
                );

                return publicationResult == DedicatedThreadControlPublicationResult::Published
                    ? DedicatedThreadStoppedPublicationResult::Published
                    : DedicatedThreadStoppedPublicationResult::StaleActivation;
            }


            /// Validates that the configured Mutex provider can support Dedicated Thread lifecycle serialization.
            DedicatedThreadSynchronizationResult ValidateSynchronization() noexcept {
                const auto acquireResult = _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
                );

                if (
                    acquireResult !=
                    ESPressio::Platform::Synchronization::LockAcquireResult::Acquired
                ) {
                    return DedicatedThreadSynchronizationResult::ProviderFailure;
                }

                return _mutex.Release() ==
                    ESPressio::Platform::Synchronization::LockReleaseResult::Released
                    ? DedicatedThreadSynchronizationResult::Ready
                    : DedicatedThreadSynchronizationResult::ProviderFailure;
            }


            // Persistent trampoline.

            /// Runs the persistent Platform trampoline that waits for semantic activations or infrastructure termination.
            static void Entry(
                void* parameter
            ) noexcept {
                auto* self = static_cast<DedicatedThreadRuntime*>(
                    parameter
                );

                if (self == nullptr) {
                    return;
                }

                for (;;) {
                    if (
                        self->_shouldTerminate(
                            self->_lifecycleContext
                        )
                    ) {
                        return;
                    }

                    bool activationPhase = false;
                    bool shouldRun = false;

                    if (self->AcquireLock() == LockAcquireResult::Acquired) {
                        const auto state = self->_control.State();

                        if (
                            state == DedicatedThreadOperationalState::Running ||
                            state == DedicatedThreadOperationalState::RunningStopRequested
                        ) {
                            activationPhase = self->_control.Phase();
                            shouldRun = true;
                        }

                        static_cast<void>(
                            self->ReleaseLock()
                        );
                    }

                    if (shouldRun) {
                        self->InvokeCallable();
                        static_cast<void>(
                            self->PublishStopped(
                                activationPhase
                            )
                        );
                        continue;
                    }

                    static_cast<void>(
                        self->_router->Wait(
                            self->_contextIndex,
                            ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                        )
                    );
                }
            }


            // Join implementation.

            /// Joins the activation captured at entry using one non-restarting canonical wait budget.
            ThreadJoinResult JoinWithBudget(
                const MonotonicWaitBudget& budget
            ) {
                const auto waitingContext = _router->CurrentContextIndex();

                if (!waitingContext.has_value()) {
                    return ThreadJoinResult::Interrupted;
                }

                if (AcquireLock() != LockAcquireResult::Acquired) {
                    return ThreadJoinResult::Interrupted;
                }

                const auto state = _control.State();

                if (state == DedicatedThreadOperationalState::NeverStarted) {
                    ReleaseLock();
                    return ThreadJoinResult::NeverStarted;
                }

                const auto capturedPhase = _control.Phase();

                if (state == DedicatedThreadOperationalState::Stopped) {
                    ReleaseLock();
                    return ThreadJoinResult::Joined;
                }

                ThreadJoinRegistration<ContextIndex> registration;
                registration.Phase = capturedPhase;
                registration.WaitingContextIndex = waitingContext.value();

                std::size_t registrationIndex = 0U;

                if (
                    _joinWaiters.Register(
                        registration,
                        registrationIndex
                    ) != WaitRegistrationStatus::Registered
                ) {
                    ReleaseLock();
                    return ThreadJoinResult::Interrupted;
                }

                if (IsActivationStopped(
                    capturedPhase
                )) {
                    _joinWaiters.Unregister(
                        registrationIndex
                    );
                    ReleaseLock();
                    return ThreadJoinResult::Joined;
                }

                static_cast<void>(
                    ReleaseLock()
                );

                for (;;) {
                    const auto remaining = budget.Remaining();

                    if (remaining.IsNoWait()) {
                        if (AcquireLock() != LockAcquireResult::Acquired) {
                            return ThreadJoinResult::Interrupted;
                        }

                        const auto stopped = IsActivationStopped(
                            capturedPhase
                        );

                        _joinWaiters.Unregister(
                            registrationIndex
                        );
                        ReleaseLock();

                        return stopped
                            ? ThreadJoinResult::Joined
                            : ThreadJoinResult::TimedOut;
                    }

                    const auto waitResult = _router->Wait(
                        waitingContext.value(),
                        remaining
                    );

                    if (AcquireLock() != LockAcquireResult::Acquired) {
                        return ThreadJoinResult::Interrupted;
                    }

                    if (IsActivationStopped(
                        capturedPhase
                    )) {
                        _joinWaiters.Unregister(
                            registrationIndex
                        );
                        ReleaseLock();
                        return ThreadJoinResult::Joined;
                    }

                    ReleaseLock();

                    if (_router->IsInterrupted(
                        waitingContext.value()
                    )) {
                        if (AcquireLock() == LockAcquireResult::Acquired) {
                            _joinWaiters.Unregister(
                                registrationIndex
                            );
                            ReleaseLock();
                        }

                        return ThreadJoinResult::Interrupted;
                    }

                    if (
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::ProviderFailure
                    ) {
                        if (AcquireLock() == LockAcquireResult::Acquired) {
                            _joinWaiters.Unregister(
                                registrationIndex
                            );
                            ReleaseLock();
                        }

                        return ThreadJoinResult::Interrupted;
                    }

                    if (
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut &&
                        budget.Remaining().IsNoWait()
                    ) {
                        if (AcquireLock() != LockAcquireResult::Acquired) {
                            return ThreadJoinResult::Interrupted;
                        }

                        const auto stopped = IsActivationStopped(
                            capturedPhase
                        );

                        _joinWaiters.Unregister(
                            registrationIndex
                        );
                        ReleaseLock();

                        return stopped
                            ? ThreadJoinResult::Joined
                            : ThreadJoinResult::TimedOut;
                    }
                }
            }


            // Handle thunks.

            /// Type-erased Thread handle bridge for lifecycle-state observation.
            static ThreadState StateThunk(
                const void* resource
            ) noexcept {
                return const_cast<DedicatedThreadRuntime*>(
                    static_cast<const DedicatedThreadRuntime*>(
                        resource
                    )
                )->State();
            }

            /// Type-erased Thread handle bridge for semantic activation.
            static ThreadStartResult StartThunk(
                void* resource
            ) noexcept {
                return static_cast<DedicatedThreadRuntime*>(
                    resource
                )->StartActivation();
            }

            /// Type-erased Thread handle bridge for cooperative stop requests.
            static ThreadStopRequestResult RequestStopThunk(
                void* resource
            ) noexcept {
                return static_cast<DedicatedThreadRuntime*>(
                    resource
                )->RequestStop();
            }

            /// Type-erased Thread handle bridge for indefinite Join.
            static ThreadJoinResult JoinThunk(
                void* resource
            ) {
                return static_cast<DedicatedThreadRuntime*>(
                    resource
                )->Join();
            }

            /// Type-erased Thread handle bridge for relative-duration Join.
            static ThreadJoinResult JoinForThunk(
                void* resource,
                Duration duration
            ) {
                return static_cast<DedicatedThreadRuntime*>(
                    resource
                )->JoinFor(
                    duration
                );
            }

            /// Type-erased Thread handle bridge for monotonic-deadline Join.
            static ThreadJoinResult JoinUntilThunk(
                void* resource,
                MonotonicTimestamp deadline
            ) {
                return static_cast<DedicatedThreadRuntime*>(
                    resource
                )->JoinUntil(
                    deadline
                );
            }

        public:

            // Provider contract.

            /// Validated Platform execution-context provider contract traits.
            using ProviderTraits =
                ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<
                    TExecutionContextProvider
                >;

            static_assert(
                sizeof(ProviderTraits) > 0U,
                "Dedicated Thread requires a valid Platform ExecutionContext provider"
            );


            // Construction.

            /// Constructs one topology-owned Dedicated Thread runtime without starting its Platform context.
            DedicatedThreadRuntime(
                TCallable callable,
                TManagedContextRouter& router,
                ContextIndex contextIndex,
                const void* lifecycleContext,
                bool (*canActivate)(const void*) noexcept,
                bool (*shouldTerminate)(const void*) noexcept
            ) noexcept(
                std::is_nothrow_move_constructible_v<TCallable>
            ) :
                _callable(
                    std::move(
                        callable
                    )
                ),
                _router(&router),
                _contextIndex(contextIndex),
                _lifecycleContext(lifecycleContext),
                _canActivate(canActivate),
                _shouldTerminate(shouldTerminate) {}


            // Infrastructure lifecycle.

            /// Initializes synchronization and the persistent Platform execution context without starting execution.
            WorkerExecutionInitializationResult Initialize(
                ESPressio::Platform::Execution::ExecutionPriority priority,
                ESPressio::Platform::Execution::ProcessorAffinity affinity,
                const char* name = nullptr
            ) noexcept {
                if (
                    ValidateSynchronization() !=
                    DedicatedThreadSynchronizationResult::Ready
                ) {
                    return WorkerExecutionInitializationResult::ProviderFailure;
                }

                ESPressio::Platform::Execution::ExecutionConfiguration configuration;
                configuration.Priority = priority;
                configuration.Affinity = affinity;
                configuration.Name = name;

                const auto result = _provider.Initialize(
                    _backing.Storage(),
                    configuration,
                    &Entry,
                    this
                );

                if (
                    result !=
                    ESPressio::Platform::Execution::ExecutionInitializationResult::Succeeded
                ) {
                    static_cast<void>(
                        _provider.Destroy()
                    );

                    return WorkerExecutionInitializationResult::ProviderFailure;
                }

                return WorkerExecutionInitializationResult::Succeeded;
            }

            /// Starts the persistent Platform execution context after the topology initialization barrier.
            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                return _provider.Start();
            }

            /// Wakes the persistent trampoline so rollback/shutdown termination is re-evaluated.
            ESPressio::Platform::Synchronization::SignalNotifyResult RequestInfrastructureTermination() noexcept {
                return _router->Wake(
                    _contextIndex
                );
            }

            /// Joins the persistent Platform execution context using the supplied native wait budget.
            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _provider.Join(
                    timeout
                );
            }

            /// Destroys the initialized Platform execution context and releases its native provider state.
            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                return _provider.Destroy();
            }


            // Public lifecycle.

            /// Returns the stable public lifecycle state under the Dedicated Thread serialization boundary.
            ThreadState State() noexcept {
                if (AcquireLock() != LockAcquireResult::Acquired) {
                    return ThreadState::Stopped;
                }

                const auto result = PublicStateFor(
                    _control.State()
                );

                static_cast<void>(
                    ReleaseLock()
                );
                return result;
            }

            /// Starts one semantic activation when global lifecycle and local state permit it.
            ThreadStartResult StartActivation() noexcept {
                if (!_canActivate(
                    _lifecycleContext
                )) {
                    return ThreadStartResult::ShuttingDown;
                }

                if (AcquireLock() != LockAcquireResult::Acquired) {
                    return ThreadStartResult::ActivationFailed;
                }

                if (!_canActivate(
                    _lifecycleContext
                )) {
                    ReleaseLock();
                    return ThreadStartResult::ShuttingDown;
                }

                const auto state = _control.State();

                if (
                    state == DedicatedThreadOperationalState::Running ||
                    state == DedicatedThreadOperationalState::RunningStopRequested
                ) {
                    static_cast<void>(
                        ReleaseLock()
                    );
                    return ThreadStartResult::AlreadyRunning;
                }

                if (
                    _router->Wake(
                        _contextIndex
                    ) != ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled
                ) {
                    static_cast<void>(
                        ReleaseLock()
                    );
                    return ThreadStartResult::ActivationFailed;
                }

                bool activationPhase = false;

                if (
                    _control.TryStart(
                        activationPhase
                    ) != DedicatedThreadControlStartResult::Started
                ) {
                    static_cast<void>(
                        ReleaseLock()
                    );
                    return ThreadStartResult::ActivationFailed;
                }

                static_cast<void>(
                    ReleaseLock()
                );
                return ThreadStartResult::Started;
            }

            /// Requests cooperative stop of the current activation and wakes its persistent context.
            ThreadStopRequestResult RequestStop() noexcept {
                if (AcquireLock() != LockAcquireResult::Acquired) {
                    return ThreadStopRequestResult::NotRunning;
                }

                if (
                    _control.TryRequestStop() !=
                    DedicatedThreadControlStopRequestResult::Accepted
                ) {
                    ReleaseLock();
                    return ThreadStopRequestResult::NotRunning;
                }

                // Stop-request acceptance is the authoritative semantic transition. The targeted
                // wake is only a mechanism for promptly interrupting blocked execution; wake failure
                // does not retract the already-published cooperative stop request.
                static_cast<void>(
                    _router->Wake(
                        _contextIndex
                    )
                );

                static_cast<void>(
                    ReleaseLock()
                );
                return ThreadStopRequestResult::Accepted;
            }

            /// Indicates whether cooperative stop has been requested for the current activation.
            bool IsStopRequested() noexcept {
                if (AcquireLock() != LockAcquireResult::Acquired) {
                    return true;
                }

                const auto result = _control.IsStopRequested();

                static_cast<void>(
                    ReleaseLock()
                );
                return result;
            }

            /// Joins the activation captured when this call begins.
            ThreadJoinResult Join() {
                return JoinWithBudget(
                    MonotonicWaitBudget::Forever()
                );
            }

            /// Joins the captured activation within one relative physical-time budget.
            ThreadJoinResult JoinFor(
                Duration duration
            ) {
                return JoinWithBudget(
                    MonotonicWaitBudget::For(
                        duration
                    )
                );
            }

            /// Joins the captured activation until one canonical monotonic deadline.
            ThreadJoinResult JoinUntil(
                MonotonicTimestamp deadline
            ) {
                return JoinWithBudget(
                    MonotonicWaitBudget::Until(
                        deadline
                    )
                );
            }


            // Shutdown cooperation.

            /// Indicates whether no semantic Dedicated Thread activation remains active.
            bool IsExecutionQuiescent() noexcept {
                if (AcquireLock() != LockAcquireResult::Acquired) {
                    return false;
                }

                const auto state = _control.State();
                const auto result =
                    state == DedicatedThreadOperationalState::NeverStarted ||
                    state == DedicatedThreadOperationalState::Stopped;

                static_cast<void>(
                    ReleaseLock()
                );
                return result;
            }


            // Context identity.

            /// Returns this Dedicated Thread's dense context index when it owns the current Platform context.
            std::optional<ContextIndex> CurrentContextIndex() const noexcept {
                if (!_provider.IsCurrentContext()) {
                    return std::nullopt;
                }

                return _contextIndex;
            }

            /// Indicates whether the addressed Dedicated Thread context currently has a cooperative interruption request.
            bool IsContextInterrupted(
                ContextIndex contextIndex
            ) noexcept {
                return
                    contextIndex == _contextIndex &&
                    (
                        IsStopRequested() ||
                        _shouldTerminate(
                            _lifecycleContext
                        )
                    );
            }


            // Handle creation.

            /// Creates a non-owning public control handle for this topology-owned Dedicated Thread.
            Thread<TThreadIdentity> Handle() noexcept {
                return Thread<TThreadIdentity>(
                    this,
                    HandleOperations()
                );
            }

            /// Returns the static type-erased operation table shared by handles for this concrete runtime.
            static const ThreadHandleOperations& HandleOperations() noexcept {
                static const ThreadHandleOperations operations{
                    &StateThunk,
                    &StartThunk,
                    &RequestStopThunk,
                    &JoinThunk,
                    &JoinForThunk,
                    &JoinUntilThunk
                };

                return operations;
            }


            // Resource inspection.

            /// Deterministic byte count reported for `ProviderObjectBytes`.
            static constexpr std::size_t ProviderObjectBytes() noexcept {
                return sizeof(TExecutionContextProvider);
            }

            /// Deterministic byte count reported for `ControlBackingBytes`.
            static constexpr std::size_t ControlBackingBytes() noexcept {
                return ExecutionContextBacking<
                    TExecutionContextProvider,
                    TStackCapacity
                >::PhysicalControlCapacity();
            }

            /// Deterministic byte count reported for `StackBackingBytes`.
            static constexpr std::size_t StackBackingBytes() noexcept {
                return ExecutionContextBacking<
                    TExecutionContextProvider,
                    TStackCapacity
                >::PhysicalStackCapacity();
            }

    };

} // ESPressio::Threading::Detail
