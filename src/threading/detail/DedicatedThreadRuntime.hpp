#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "../Thread.hpp"
#include "DedicatedThreadControl.hpp"
#include "TaskFacilityRuntime.hpp"
#include "TaskWorkerExecutionContext.hpp"
#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

    template<class TCallable, class = void>
    struct IsVoidThreadCallableWithoutContext final {

        static constexpr bool Value = false;

    };


    template<class TCallable>
    struct IsVoidThreadCallableWithoutContext<
        TCallable,
        std::void_t<
            std::invoke_result_t<TCallable&>
        >
    > final {

        static constexpr bool Value = std::is_same_v<
            std::invoke_result_t<TCallable&>,
            void
        >;

    };


    template<class TCallable, class = void>
    struct IsVoidThreadCallableWithContext final {

        static constexpr bool Value = false;

    };


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

        static constexpr bool Value = std::is_same_v<
            std::invoke_result_t<
                TCallable&,
                ThreadContext&
            >,
            void
        >;

    };


    template<class TThreadIdentity, class TCallable, std::size_t TStackCapacity, std::size_t TExecutionContextCapacity, class TAtomicWord8Provider, class TMutexProvider, class TExecutionContextProvider, class TManagedContextRouter>
    class DedicatedThreadRuntime final {

        static_assert(
            IsVoidThreadCallableWithContext<TCallable>::Value ||
            IsVoidThreadCallableWithoutContext<TCallable>::Value,
            "Dedicated Thread callable must return exactly void and accept either ThreadContext& or no arguments"
        );

        private:

            // Compact semantic state.

            /// One-byte authoritative Dedicated Thread lifecycle and activation Phase.
            DedicatedThreadControl<TAtomicWord8Provider> _control;

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

            using ContextIndex =
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type;

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

            bool AcquireLock() noexcept {
                return _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                ) == ESPressio::Platform::Synchronization::LockAcquireResult::Acquired;
            }

            void ReleaseLock() noexcept {
                static_cast<void>(
                    _mutex.Release()
                );
            }

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

            void WakeJoiners(
                bool phase
            ) {
                _joinWaiters.VisitActive(
                    [this, phase](
                        auto& registration
                    ) {
                        if (registration.Phase != phase) {
                            return;
                        }

                        static_cast<void>(
                            _router->Wake(
                                registration.WaitingContextIndex
                            )
                        );
                    }
                );
            }

            bool ActivationHasStopped(
                bool capturedPhase
            ) const noexcept {
                const auto currentPhase = _control.Phase();

                if (currentPhase != capturedPhase) {
                    return true;
                }

                const auto state = _control.State();

                return state == DedicatedThreadOperationalState::Stopped;
            }

            static bool IsStopRequestedThunk(
                const void* resource
            ) noexcept {
                return static_cast<const DedicatedThreadRuntime*>(
                    resource
                )->IsStopRequested();
            }

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

            void PublishStopped(
                bool activationPhase
            ) noexcept {
                if (!AcquireLock()) {
                    return;
                }

                if (_control.TryPublishStopped(
                    activationPhase
                )) {
                    WakeJoiners(
                        activationPhase
                    );
                }

                ReleaseLock();
            }


            bool ValidateSynchronization() noexcept {
                const auto acquireResult = _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
                );

                if (
                    acquireResult !=
                    ESPressio::Platform::Synchronization::LockAcquireResult::Acquired
                ) {
                    return false;
                }

                return _mutex.Release() ==
                    ESPressio::Platform::Synchronization::LockReleaseResult::Released;
            }


            // Persistent trampoline.

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

                    if (self->AcquireLock()) {
                        const auto state = self->_control.State();

                        if (
                            state == DedicatedThreadOperationalState::Running ||
                            state == DedicatedThreadOperationalState::RunningStopRequested
                        ) {
                            activationPhase = self->_control.Phase();
                            shouldRun = true;
                        }

                        self->ReleaseLock();
                    }

                    if (shouldRun) {
                        self->InvokeCallable();
                        self->PublishStopped(
                            activationPhase
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

            ThreadJoinResult JoinWithBudget(
                const MonotonicWaitBudget& budget
            ) {
                const auto waitingContext = _router->CurrentContextIndex();

                if (!waitingContext.has_value()) {
                    return ThreadJoinResult::Interrupted;
                }

                if (!AcquireLock()) {
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

                if (ActivationHasStopped(
                    capturedPhase
                )) {
                    _joinWaiters.Unregister(
                        registrationIndex
                    );
                    ReleaseLock();
                    return ThreadJoinResult::Joined;
                }

                ReleaseLock();

                for (;;) {
                    const auto remaining = budget.Remaining();

                    if (remaining.IsNoWait()) {
                        if (!AcquireLock()) {
                            return ThreadJoinResult::Interrupted;
                        }

                        const auto stopped = ActivationHasStopped(
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

                    if (!AcquireLock()) {
                        return ThreadJoinResult::Interrupted;
                    }

                    if (ActivationHasStopped(
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
                        if (AcquireLock()) {
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
                        if (AcquireLock()) {
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
                        if (!AcquireLock()) {
                            return ThreadJoinResult::Interrupted;
                        }

                        const auto stopped = ActivationHasStopped(
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

            static ThreadState StateThunk(
                const void* resource
            ) noexcept {
                return const_cast<DedicatedThreadRuntime*>(
                    static_cast<const DedicatedThreadRuntime*>(
                        resource
                    )
                )->State();
            }

            static ThreadStartResult StartThunk(
                void* resource
            ) noexcept {
                return static_cast<DedicatedThreadRuntime*>(
                    resource
                )->StartActivation();
            }

            static ThreadStopRequestResult RequestStopThunk(
                void* resource
            ) noexcept {
                return static_cast<DedicatedThreadRuntime*>(
                    resource
                )->RequestStop();
            }

            static ThreadJoinResult JoinThunk(
                void* resource
            ) {
                return static_cast<DedicatedThreadRuntime*>(
                    resource
                )->Join();
            }

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

            using ProviderTraits =
                ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<
                    TExecutionContextProvider
                >;

            static_assert(
                sizeof(ProviderTraits) > 0U,
                "Dedicated Thread requires a valid Platform ExecutionContext provider"
            );


            // Construction.

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

            WorkerExecutionInitializationResult Initialize(
                ESPressio::Platform::Execution::ExecutionPriority priority,
                ESPressio::Platform::Execution::ProcessorAffinity affinity,
                const char* name = nullptr
            ) noexcept {
                if (!ValidateSynchronization()) {
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

                return result == ESPressio::Platform::Execution::ExecutionInitializationResult::Succeeded
                    ? WorkerExecutionInitializationResult::Succeeded
                    : WorkerExecutionInitializationResult::ProviderFailure;
            }

            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                return _provider.Start();
            }

            /// Wakes the persistent trampoline so rollback/shutdown termination is re-evaluated.
            void RequestInfrastructureTermination() noexcept {
                static_cast<void>(
                    _router->Wake(
                        _contextIndex
                    )
                );
            }

            ESPressio::Platform::Execution::ExecutionJoinResult JoinInfrastructure(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _provider.Join(
                    timeout
                );
            }

            ESPressio::Platform::Execution::ExecutionDestroyResult DestroyInfrastructure() noexcept {
                return _provider.Destroy();
            }


            // Public lifecycle.

            ThreadState State() noexcept {
                if (!AcquireLock()) {
                    return ThreadState::Stopped;
                }

                const auto result = PublicStateFor(
                    _control.State()
                );

                ReleaseLock();
                return result;
            }

            ThreadStartResult StartActivation() noexcept {
                if (!_canActivate(
                    _lifecycleContext
                )) {
                    return ThreadStartResult::ShuttingDown;
                }

                if (!AcquireLock()) {
                    return ThreadStartResult::ActivationFailed;
                }

                if (!_canActivate(
                    _lifecycleContext
                )) {
                    ReleaseLock();
                    return ThreadStartResult::ShuttingDown;
                }

                bool activationPhase = false;

                if (!_control.TryStart(
                    activationPhase
                )) {
                    ReleaseLock();
                    return ThreadStartResult::AlreadyRunning;
                }

                static_cast<void>(
                    _router->Wake(
                        _contextIndex
                    )
                );

                ReleaseLock();
                return ThreadStartResult::Started;
            }

            ThreadStopRequestResult RequestStop() noexcept {
                if (!AcquireLock()) {
                    return ThreadStopRequestResult::NotRunning;
                }

                if (!_control.TryRequestStop()) {
                    ReleaseLock();
                    return ThreadStopRequestResult::NotRunning;
                }

                static_cast<void>(
                    _router->Wake(
                        _contextIndex
                    )
                );

                ReleaseLock();
                return ThreadStopRequestResult::Accepted;
            }

            bool IsStopRequested() const noexcept {
                return _control.IsStopRequested();
            }

            ThreadJoinResult Join() {
                return JoinWithBudget(
                    MonotonicWaitBudget::Forever()
                );
            }

            ThreadJoinResult JoinFor(
                Duration duration
            ) {
                return JoinWithBudget(
                    MonotonicWaitBudget::For(
                        duration
                    )
                );
            }

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
                if (!AcquireLock()) {
                    return false;
                }

                const auto state = _control.State();
                const auto result =
                    state == DedicatedThreadOperationalState::NeverStarted ||
                    state == DedicatedThreadOperationalState::Stopped;

                ReleaseLock();
                return result;
            }


            // Context identity.

            /// Indicates whether this Dedicated Thread owns the current Platform execution context.
            bool TryResolveCurrentContext(
                ContextIndex& contextIndex
            ) const noexcept {
                if (!_provider.IsCurrentContext()) {
                    return false;
                }

                contextIndex = _contextIndex;
                return true;
            }

            /// Indicates whether the addressed Dedicated Thread context currently has a cooperative interruption request.
            bool IsContextInterrupted(
                ContextIndex contextIndex
            ) const noexcept {
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

            Thread<TThreadIdentity> Handle() noexcept {
                return Thread<TThreadIdentity>(
                    this,
                    HandleOperations()
                );
            }

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

            static constexpr std::size_t ProviderObjectBytes() noexcept {
                return sizeof(TExecutionContextProvider);
            }

            static constexpr std::size_t ControlBackingBytes() noexcept {
                return ExecutionContextBacking<
                    TExecutionContextProvider,
                    TStackCapacity
                >::PhysicalControlCapacity();
            }

            static constexpr std::size_t StackBackingBytes() noexcept {
                return ExecutionContextBacking<
                    TExecutionContextProvider,
                    TStackCapacity
                >::PhysicalStackCapacity();
            }

    };

} // ESPressio::Threading::Detail
