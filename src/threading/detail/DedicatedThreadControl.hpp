#pragma once

#include <cstdint>

namespace ESPressio::Threading::Detail {

    enum class DedicatedThreadOperationalState : std::uint8_t {
        NeverStarted = 0,
        Running = 1,
        RunningStopRequested = 2,
        Stopped = 3
    };


    class DedicatedThreadControl final {

        private:

            // Packed control byte.

            /// Packed operational state and activation Phase.
            std::uint8_t _value;

            static constexpr std::uint8_t StateMask = 0x03U;
            static constexpr std::uint8_t PhaseMask = 0x04U;

        public:

            DedicatedThreadControl() noexcept :
                _value(
                    static_cast<std::uint8_t>(
                        DedicatedThreadOperationalState::NeverStarted
                    )
                ) {}


            // Observation.

            /// Returns the internal Dedicated Thread operational state.
            DedicatedThreadOperationalState State() const noexcept {
                return static_cast<DedicatedThreadOperationalState>(
                    _value & StateMask
                );
            }

            /// Returns the current activation Phase.
            bool Phase() const noexcept {
                return (_value & PhaseMask) != 0U;
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
                const auto state = State();

                if (
                    state == DedicatedThreadOperationalState::Running ||
                    state == DedicatedThreadOperationalState::RunningStopRequested
                ) {
                    return false;
                }

                const auto nextPhase = static_cast<std::uint8_t>(
                    (_value ^ PhaseMask) & PhaseMask
                );

                _value = static_cast<std::uint8_t>(
                    nextPhase |
                    static_cast<std::uint8_t>(DedicatedThreadOperationalState::Running)
                );

                activationPhase = nextPhase != 0U;
                return true;
            }

            /// Requests cooperative stop for the current activation.
            ///
            /// The owning Dedicated Thread runtime serializes every mutation through its mutex.
            bool TryRequestStop() noexcept {
                if (State() != DedicatedThreadOperationalState::Running) {
                    return false;
                }

                _value = static_cast<std::uint8_t>(
                    (_value & PhaseMask) |
                    static_cast<std::uint8_t>(DedicatedThreadOperationalState::RunningStopRequested)
                );

                return true;
            }

            /// Publishes completion only for the activation Phase that actually returned.
            ///
            /// The owning Dedicated Thread runtime serializes every mutation through its mutex.
            bool TryPublishStopped(
                bool activationPhase
            ) noexcept {
                if (((_value & PhaseMask) != 0U) != activationPhase) {
                    return false;
                }

                const auto state = State();

                if (
                    state != DedicatedThreadOperationalState::Running &&
                    state != DedicatedThreadOperationalState::RunningStopRequested
                ) {
                    return false;
                }

                _value = static_cast<std::uint8_t>(
                    (_value & PhaseMask) |
                    static_cast<std::uint8_t>(DedicatedThreadOperationalState::Stopped)
                );

                return true;
            }

    };

} // ESPressio::Threading::Detail
