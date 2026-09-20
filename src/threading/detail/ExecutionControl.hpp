#pragma once

#include "../ThreadingTypes.hpp"
#include "TaskFacilityRuntime.hpp"

namespace ESPressio::Threading::Detail {

    template<class TExecutionContextProvider, class TManagedContextRouter>
    class ExecutionControl final {

        private:

            // Shared sleep implementation.

            static SleepResult SleepWithBudget(
                TManagedContextRouter& router,
                const MonotonicWaitBudget& budget
            ) {
                const auto contextIndex = router.CurrentContextIndex();

                if (!contextIndex.has_value()) {
                    return SleepResult::Interrupted;
                }

                for (;;) {
                    if (router.IsInterrupted(
                        contextIndex.value()
                    )) {
                        return SleepResult::Interrupted;
                    }

                    const auto remaining = budget.Remaining();

                    if (remaining.IsNoWait()) {
                        return SleepResult::Completed;
                    }

                    const auto waitResult = router.Wait(
                        contextIndex.value(),
                        remaining
                    );

                    if (router.IsInterrupted(
                        contextIndex.value()
                    )) {
                        return SleepResult::Interrupted;
                    }

                    if (
                        waitResult == ESPressio::Platform::Synchronization::SignalWaitResult::ProviderFailure
                    ) {
                        return SleepResult::Interrupted;
                    }

                    // Signal wake is only a prompt to re-evaluate interruption/deadline.
                    // TimedOut is likewise re-evaluated against the original Clock budget.
                }
            }

        public:

            // Scheduler cooperation.

            /// Gives the concrete Platform scheduler an opportunity to run another runnable context.
            static void Yield() noexcept {
                TExecutionContextProvider::Yield();
            }


            // Canonical monotonic sleep.

            /// Suspends the current managed context for one relative physical-time budget.
            static SleepResult SleepFor(
                TManagedContextRouter& router,
                Duration duration
            ) {
                return SleepWithBudget(
                    router,
                    MonotonicWaitBudget::For(
                        duration
                    )
                );
            }

            /// Suspends the current managed context until one canonical monotonic deadline.
            static SleepResult SleepUntil(
                TManagedContextRouter& router,
                MonotonicTimestamp deadline
            ) {
                return SleepWithBudget(
                    router,
                    MonotonicWaitBudget::Until(
                        deadline
                    )
                );
            }

    };

} // ESPressio::Threading::Detail
