#pragma once

#include <cstdint>

#include <ESPressio_Clock.hpp>
#include <ESPressio_Platform.hpp>

namespace ESPressio::Threading::Detail {

    enum class MonotonicWaitMode : std::uint8_t {
        Forever = 0,
        Relative = 1,
        Deadline = 2
    };


    class MonotonicWaitBudget final {

        private:

            // Budget specification.

            /// Selected wait-budget mode.
            MonotonicWaitMode _mode;

            /// Canonical monotonic coordinate captured when a relative budget begins.
            ESPressio::Clock::MonotonicTimestamp _start;

            /// Relative physical-time budget.
            ESPressio::Clock::Duration _duration;

            /// Canonical absolute deadline for deadline-based waits.
            ESPressio::Clock::MonotonicTimestamp _deadline;

        public:

            // Construction.

            /// Creates an indefinite wait budget.
            static MonotonicWaitBudget Forever() noexcept {
                return MonotonicWaitBudget{
                    MonotonicWaitMode::Forever,
                    {},
                    {},
                    {}
                };
            }

            /// Creates one relative wait budget beginning at the current canonical monotonic coordinate.
            static MonotonicWaitBudget For(
                ESPressio::Clock::Duration duration
            ) noexcept {
                return MonotonicWaitBudget{
                    MonotonicWaitMode::Relative,
                    ESPressio::Clock::MonotonicNow(),
                    duration,
                    {}
                };
            }

            /// Creates one absolute canonical monotonic deadline budget.
            static MonotonicWaitBudget Until(
                ESPressio::Clock::MonotonicTimestamp deadline
            ) noexcept {
                return MonotonicWaitBudget{
                    MonotonicWaitMode::Deadline,
                    {},
                    {},
                    deadline
                };
            }


            // Budget inspection.

            /// Returns the selected wait-budget mode.
            MonotonicWaitMode Mode() const noexcept {
                return _mode;
            }

            /// Computes the remaining relative Platform wait without restarting the original budget.
            ESPressio::Platform::Synchronization::WaitTimeout Remaining() const noexcept {
                if (_mode == MonotonicWaitMode::Forever) {
                    return ESPressio::Platform::Synchronization::WaitTimeout::Forever();
                }

                const auto now = ESPressio::Clock::MonotonicNow();

                if (_mode == MonotonicWaitMode::Deadline) {
                    if (now >= _deadline) {
                        return ESPressio::Platform::Synchronization::WaitTimeout::NoWait();
                    }

                    const auto remaining = ESPressio::Clock::Delta(
                        now,
                        _deadline
                    ).Nanoseconds();

                    if (remaining <= 0) {
                        return ESPressio::Platform::Synchronization::WaitTimeout::NoWait();
                    }

                    return ESPressio::Platform::Synchronization::WaitTimeout::ForNanoseconds(
                        static_cast<std::uint64_t>(
                            remaining
                        )
                    );
                }

                const auto budgetNanoseconds = _duration.Nanoseconds();

                if (budgetNanoseconds <= 0) {
                    return ESPressio::Platform::Synchronization::WaitTimeout::NoWait();
                }

                const auto elapsedNanoseconds = ESPressio::Clock::Delta(
                    _start,
                    now
                ).Nanoseconds();

                if (elapsedNanoseconds >= budgetNanoseconds) {
                    return ESPressio::Platform::Synchronization::WaitTimeout::NoWait();
                }

                const auto normalizedElapsed =
                    elapsedNanoseconds > 0
                        ? elapsedNanoseconds
                        : 0;

                return ESPressio::Platform::Synchronization::WaitTimeout::ForNanoseconds(
                    static_cast<std::uint64_t>(
                        budgetNanoseconds - normalizedElapsed
                    )
                );
            }

    };

} // ESPressio::Threading::Detail
