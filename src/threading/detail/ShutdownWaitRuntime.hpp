#pragma once

#include <cstddef>

#include "../ThreadingTypes.hpp"
#include "InfrastructureLifecycle.hpp"
#include "TaskFacilityRuntime.hpp"
#include "WaitRegistration.hpp"

namespace ESPressio::Threading::Detail {

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

            TInfrastructureLifecycle* _lifecycle;

            bool IsComplete() const noexcept {
                return _lifecycle->State() == InfrastructureState::ShutdownComplete;
            }

        public:

            explicit ShutdownWaitRuntime(
                TInfrastructureLifecycle& lifecycle,
                TManagedContextRouter&
            ) noexcept :
                _lifecycle(&lifecycle) {}


            bool ValidateSynchronization() noexcept {
                return true;
            }


            ShutdownWaitResult Wait() {
                return IsComplete()
                    ? ShutdownWaitResult::Completed
                    : ShutdownWaitResult::Interrupted;
            }

            ShutdownWaitResult WaitFor(
                Duration
            ) {
                return Wait();
            }

            ShutdownWaitResult WaitUntil(
                MonotonicTimestamp
            ) {
                return Wait();
            }

            void WakeCompleted() noexcept {}

    };


    template<class TInfrastructureLifecycle, std::size_t TExecutionContextCapacity, class TMutexProvider, class TManagedContextRouter>
    class ShutdownWaitRuntime final {

        private:

            using ContextIndex =
                typename ExecutionContextIndexTraits<TExecutionContextCapacity>::Type;

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

            bool IsComplete() const noexcept {
                return _lifecycle->State() == InfrastructureState::ShutdownComplete;
            }

            void Unregister(
                std::size_t registrationIndex
            ) noexcept {
                if (!AcquireLock()) {
                    return;
                }

                _waiters.Unregister(
                    registrationIndex
                );

                ReleaseLock();
            }

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

                if (!AcquireLock()) {
                    return ShutdownWaitResult::Interrupted;
                }

                if (IsComplete()) {
                    ReleaseLock();
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
                    ReleaseLock();
                    return ShutdownWaitResult::Interrupted;
                }

                // Re-observe after publication of the complete registration. Completion either
                // happened before this check or must discover this waiter during terminal wake.
                if (IsComplete()) {
                    _waiters.Unregister(
                        registrationIndex
                    );
                    ReleaseLock();
                    return ShutdownWaitResult::Completed;
                }

                ReleaseLock();

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

            explicit ShutdownWaitRuntime(
                TInfrastructureLifecycle& lifecycle,
                TManagedContextRouter& router
            ) noexcept :
                _lifecycle(&lifecycle),
                _router(&router) {}


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


            // Wait operations.

            ShutdownWaitResult Wait() {
                return WaitWithBudget(
                    MonotonicWaitBudget::Forever()
                );
            }

            ShutdownWaitResult WaitFor(
                Duration duration
            ) {
                return WaitWithBudget(
                    MonotonicWaitBudget::For(
                        duration
                    )
                );
            }

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
                if (!AcquireLock()) {
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

                ReleaseLock();
            }

    };

} // ESPressio::Threading::Detail
