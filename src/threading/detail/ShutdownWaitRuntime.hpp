#pragma once

#include <cstddef>

#include "../ThreadingTypes.hpp"
#include "InfrastructureLifecycle.hpp"
#include "TaskFacilityRuntime.hpp"
#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

    /// Result of validating the synchronization provider used by shutdown waiting.
    enum class ShutdownWaitSynchronizationResult : std::uint8_t {
        Ready = 0,
        ProviderFailure = 1
    };


    /// Owns bounded terminal-shutdown wait registration and targeted wake behavior.
    ///
    /// @tparam TInfrastructureLifecycle Authoritative Threading infrastructure lifecycle Type.
    /// @tparam TExecutionContextCapacity Number of managed execution contexts eligible to wait.
    /// @tparam TMutexProvider Concrete Platform Mutex provider protecting waiter registration state.
    /// @tparam TManagedContextRouter Topology router used for context identity and targeted wake delivery.
    template<class TInfrastructureLifecycle, std::size_t TExecutionContextCapacity, class TMutexProvider, class TManagedContextRouter>
    class ShutdownWaitRuntime;


    /// Empty-topology shutdown wait specialization retaining only the lifecycle reference.
    ///
    /// @tparam TInfrastructureLifecycle Authoritative Threading infrastructure lifecycle Type.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type selected by Bootstrap.
    /// @tparam TManagedContextRouter Empty-topology router Type selected by Bootstrap.
    template<class TInfrastructureLifecycle, class TMutexProvider, class TManagedContextRouter>
    class ShutdownWaitRuntime<
        TInfrastructureLifecycle,
        0U,
        TMutexProvider,
        TManagedContextRouter
    > final {

        private:

            // Authoritative terminal lifecycle.

            /// Non-owning reference to the application-wide Threading lifecycle.
            TInfrastructureLifecycle* _lifecycle;


            // Completion inspection.

            /// Indicates whether the authoritative Threading lifecycle reached terminal shutdown completion.
            bool IsComplete() const noexcept {
                return _lifecycle->State() == InfrastructureState::ShutdownComplete;
            }

        public:

            // Construction.

            /// Binds terminal-shutdown waiting to the authoritative lifecycle and managed-context router.
            explicit ShutdownWaitRuntime(
                TInfrastructureLifecycle& lifecycle,
                TManagedContextRouter&
            ) noexcept :
                _lifecycle(&lifecycle) {}


            // Synchronization validation.

            /// Reports that the empty topology requires no synchronization provider validation.
            ShutdownWaitSynchronizationResult ValidateSynchronization() noexcept {
                return ShutdownWaitSynchronizationResult::Ready;
            }


            // Wait operations.

            /// Observes terminal shutdown completion without blocking for an empty topology.
            ShutdownWaitResult Wait() {
                return IsComplete()
                    ? ShutdownWaitResult::Completed
                    : ShutdownWaitResult::Interrupted;
            }

            /// Observes terminal shutdown completion for the empty topology; duration does not require blocking.
            ShutdownWaitResult WaitFor(
                Duration
            ) {
                return Wait();
            }

            /// Observes terminal shutdown completion for the empty topology; deadline does not require blocking.
            ShutdownWaitResult WaitUntil(
                MonotonicTimestamp
            ) {
                return Wait();
            }

            // Terminal publication wake.

            /// Performs no wake work because an empty topology owns no managed contexts.
            void WakeCompleted() noexcept {}

    };


    /// Defines the compile-time contract for `ShutdownWaitRuntime`.
    /// @tparam TInfrastructureLifecycle Authoritative Threading infrastructure lifecycle Type.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<class TInfrastructureLifecycle, std::size_t TExecutionContextCapacity, class TMutexProvider, class TManagedContextRouter>
    class ShutdownWaitRuntime final {

        private:

            /// Compact Type used to identify one managed execution context.
            using ContextIndex =
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type;

            /// Concrete bounded registration Type stored by this wait surface.
            using Registration = ShutdownWaitRegistration<ContextIndex>;

            /// Authoritative global lifecycle.
            TInfrastructureLifecycle* _lifecycle;

            /// Non-owning topology targeted-wake router.
            TManagedContextRouter* _router;

            /// Target-owned bounded terminal-shutdown wait registrations.
            RegistrationSet<
                Registration,
                TExecutionContextCapacity
            > _waiters;

            /// Serializes registration publication/removal with terminal wake discovery.
            TMutexProvider _mutex;


            // Synchronization and completion helpers.

            /// Acquires the shutdown-wait registration mutex indefinitely.
            ESPressio::Platform::Synchronization::LockAcquireResult AcquireLock() noexcept {
                return _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                );
            }

            /// Releases the shutdown-wait registration mutex.
            ESPressio::Platform::Synchronization::LockReleaseResult ReleaseLock() noexcept {
                return _mutex.Release();
            }

            /// Indicates whether the authoritative Threading lifecycle reached terminal shutdown completion.
            bool IsComplete() const noexcept {
                return _lifecycle->State() == InfrastructureState::ShutdownComplete;
            }

            /// Removes one previously published terminal-shutdown wait registration.
            void Unregister(
                std::size_t registrationIndex
            ) noexcept {
                if (
                    AcquireLock() !=
                    ESPressio::Platform::Synchronization::LockAcquireResult::Acquired
                ) {
                    return;
                }

                _waiters.Unregister(
                    registrationIndex
                );

                static_cast<void>(
                    ReleaseLock()
                );
            }

            /// Waits for terminal shutdown completion using one canonical monotonic wait budget.
            ShutdownWaitResult WaitWithBudget(
                const MonotonicWaitBudget& budget
            ) {
                if (IsComplete()) {
                    return ShutdownWaitResult::Completed;
                }

                const auto contextIndex = _router->CurrentContextIndex();

                if (!contextIndex.has_value()) {
                    return ShutdownWaitResult::Interrupted;
                }

                if (
                    AcquireLock() !=
                    ESPressio::Platform::Synchronization::LockAcquireResult::Acquired
                ) {
                    return ShutdownWaitResult::Interrupted;
                }

                if (IsComplete()) {
                    static_cast<void>(
                        ReleaseLock()
                    );
                    return ShutdownWaitResult::Completed;
                }

                Registration registration;
                registration.WaitingContextIndex = contextIndex.value();

                std::size_t registrationIndex = 0U;

                if (
                    _waiters.Register(
                        registration,
                        registrationIndex
                    ) != WaitRegistrationStatus::Registered
                ) {
                    static_cast<void>(
                        ReleaseLock()
                    );
                    return ShutdownWaitResult::Interrupted;
                }

                // Re-observe after publication of the complete registration. Completion either
                // happened before this check or must discover this waiter during terminal wake.
                if (IsComplete()) {
                    _waiters.Unregister(
                        registrationIndex
                    );
                    static_cast<void>(
                        ReleaseLock()
                    );
                    return ShutdownWaitResult::Completed;
                }

                static_cast<void>(
                    ReleaseLock()
                );

                for (;;) {
                    const auto remaining = budget.Remaining();

                    if (remaining.IsNoWait()) {
                        const auto complete = IsComplete();

                        Unregister(
                            registrationIndex
                        );

                        return complete
                            ? ShutdownWaitResult::Completed
                            : ShutdownWaitResult::TimedOut;
                    }

                    const auto waitResult = _router->Wait(
                        contextIndex.value(),
                        remaining
                    );

                    if (IsComplete()) {
                        Unregister(
                            registrationIndex
                        );
                        return ShutdownWaitResult::Completed;
                    }

                    if (_router->IsInterrupted(
                        contextIndex.value()
                    )) {
                        Unregister(
                            registrationIndex
                        );
                        return ShutdownWaitResult::Interrupted;
                    }

                    if (
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::ProviderFailure
                    ) {
                        Unregister(
                            registrationIndex
                        );
                        return ShutdownWaitResult::Interrupted;
                    }

                    if (
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut &&
                        budget.Remaining().IsNoWait()
                    ) {
                        const auto complete = IsComplete();

                        Unregister(
                            registrationIndex
                        );

                        return complete
                            ? ShutdownWaitResult::Completed
                            : ShutdownWaitResult::TimedOut;
                    }
                }
            }

        public:

            // Construction.

            /// Binds terminal-shutdown waiting to the authoritative lifecycle and managed-context router.
            explicit ShutdownWaitRuntime(
                TInfrastructureLifecycle& lifecycle,
                TManagedContextRouter& router
            ) noexcept :
                _lifecycle(&lifecycle),
                _router(&router) {}


            /// Validates that the configured Mutex provider can be acquired and released.
            ShutdownWaitSynchronizationResult ValidateSynchronization() noexcept {
                const auto acquireResult = _mutex.Acquire(
                    ESPressio::Platform::Synchronization::WaitTimeout::NoWait()
                );

                if (
                    acquireResult !=
                    ESPressio::Platform::Synchronization::LockAcquireResult::Acquired
                ) {
                    return ShutdownWaitSynchronizationResult::ProviderFailure;
                }

                return _mutex.Release() ==
                    ESPressio::Platform::Synchronization::LockReleaseResult::Released
                    ? ShutdownWaitSynchronizationResult::Ready
                    : ShutdownWaitSynchronizationResult::ProviderFailure;
            }


            // Wait operations.

            /// Waits indefinitely for terminal shutdown completion.
            ShutdownWaitResult Wait() {
                return WaitWithBudget(
                    MonotonicWaitBudget::Forever()
                );
            }

            /// Waits for terminal shutdown completion for at most the supplied duration.
            ShutdownWaitResult WaitFor(
                Duration duration
            ) {
                return WaitWithBudget(
                    MonotonicWaitBudget::For(
                        duration
                    )
                );
            }

            /// Waits for terminal shutdown completion until the supplied monotonic deadline.
            ShutdownWaitResult WaitUntil(
                MonotonicTimestamp deadline
            ) {
                return WaitWithBudget(
                    MonotonicWaitBudget::Until(
                        deadline
                    )
                );
            }


            // Terminal publication wake.

            /// Wakes every context registered against the non-restartable terminal shutdown predicate.
            void WakeCompleted() {
                if (
                    AcquireLock() !=
                    ESPressio::Platform::Synchronization::LockAcquireResult::Acquired
                ) {
                    return;
                }

                _waiters.VisitActive(
                    [this](
                        Registration& registration
                    ) {
                        static_cast<void>(
                            _router->Wake(
                                registration.WaitingContextIndex
                            )
                        );
                    }
                );

                static_cast<void>(
                    ReleaseLock()
                );
            }

    };

} // ESPressio::Threading::Detail
