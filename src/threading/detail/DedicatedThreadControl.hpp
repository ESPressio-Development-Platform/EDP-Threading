#pragma once

#include <cstdint>

namespace ESPressio::Threading::Detail {

    enum class DedicatedThreadOperationalState : std::uint8_t {
        NeverStarted = 0,
        Running = 1,
        RunningStopRequested = 2,
        Stopped = 3
    };


    template<class TAtomicWord8Provider>
    class DedicatedThreadControl final {

        private:

            // Packed control byte.

            /// Packed operational state and activation Phase.
            typename TAtomicWord8Provider::Word _value;

            static constexpr std::uint8_t StateMask = 0x03U;
            static constexpr std::uint8_t PhaseMask = 0x04U;

        public:

            // Observation.

            /// Returns the internal Dedicated Thread operational state.
            DedicatedThreadOperationalState State() const noexcept {
                return static_cast<DedicatedThreadOperationalState>(
                    _value.LoadAcquire() & StateMask
                );
            }

            /// Returns the current activation Phase.
            bool Phase() const noexcept {
                return (_value.LoadAcquire() & PhaseMask) != 0U;
            }

            /// Indicates whether cooperative stop has been requested for the active activation.
            bool IsStopRequested() const noexcept {
                return State() == DedicatedThreadOperationalState::RunningStopRequested;
            }


            // Lifecycle transitions.

            /// Atomically starts a new activation and toggles the activation Phase.
            bool TryStart(
                bool& activationPhase
            ) noexcept {
                auto expected = _value.LoadAcquire();

                for (;;) {
                    const auto state = static_cast<DedicatedThreadOperationalState>(
                        expected & StateMask
                    );

                    if (
                        state == DedicatedThreadOperationalState::Running ||
                        state == DedicatedThreadOperationalState::RunningStopRequested
                    ) {
                        return false;
                    }

                    const auto nextPhase = static_cast<std::uint8_t>(
                        (expected ^ PhaseMask) & PhaseMask
                    );
                    const auto desired = static_cast<std::uint8_t>(
                        nextPhase |
                        static_cast<std::uint8_t>(DedicatedThreadOperationalState::Running)
                    );

                    if (_value.CompareExchangeAcqRel(
                        expected,
                        desired
                    )) {
                        activationPhase = nextPhase != 0U;
                        return true;
                    }
                }
            }

            /// Atomically requests cooperative stop for the current activation.
            bool TryRequestStop() noexcept {
                auto expected = _value.LoadAcquire();

                for (;;) {
                    if (
                        static_cast<DedicatedThreadOperationalState>(
                            expected & StateMask
                        ) != DedicatedThreadOperationalState::Running
                    ) {
                        return false;
                    }

                    const auto desired = static_cast<std::uint8_t>(
                        (expected & PhaseMask) |
                        static_cast<std::uint8_t>(DedicatedThreadOperationalState::RunningStopRequested)
                    );

                    if (_value.CompareExchangeAcqRel(
                        expected,
                        desired
                    )) { return true; }
                }
            }

            /// Publishes completion only for the activation Phase that actually returned.
            bool TryPublishStopped(
                bool activationPhase
            ) noexcept {
                auto expected = _value.LoadAcquire();

                for (;;) {
                    if (((expected & PhaseMask) != 0U) != activationPhase) {
                        return false;
                    }

                    const auto state = static_cast<DedicatedThreadOperationalState>(
                        expected & StateMask
                    );

                    if (
                        state != DedicatedThreadOperationalState::Running &&
                        state != DedicatedThreadOperationalState::RunningStopRequested
                    ) {
                        return false;
                    }

                    const auto desired = static_cast<std::uint8_t>(
                        (expected & PhaseMask) |
                        static_cast<std::uint8_t>(DedicatedThreadOperationalState::Stopped)
                    );

                    if (_value.CompareExchangeAcqRel(
                        expected,
                        desired
                    )) { return true; }
                }
            }

    };

} // ESPressio::Threading::Detail
