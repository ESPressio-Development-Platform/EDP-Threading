#pragma once

#include <cstdint>

namespace ESPressio::Threading::Detail {

    enum class DedicatedThreadOperationalState : std::uint8_t {
        NeverStarted = 0,
        Running = 1,
        RunningStopRequested = 2,
        Stopped = 3
    };


    enum class DedicatedThreadControlStartResult : std::uint8_t {
        Started = 0,
        AlreadyRunning = 1
    };


    enum class DedicatedThreadControlStopRequestResult : std::uint8_t {
        Accepted = 0,
        NotRunning = 1
    };


    enum class DedicatedThreadControlPublicationResult : std::uint8_t {
        Published = 0,
        PhaseMismatch = 1,
        NotRunning = 2
    };


    class DedicatedThreadControl final {

        private:

            // Packed control byte.

            /// Packed operational state and activation Phase.
            std::uint8_t _value;

            /// Bit mask containing the encoded Dedicated Thread operational state.
            static constexpr std::uint8_t StateMask = 0x03U;
            /// Bit mask containing the one-bit Dedicated Thread activation Phase.
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

            /// Starts a new activation and toggles the activation Phase under the owning runtime mutex.
            DedicatedThreadControlStartResult TryStart(
                bool& activationPhase
            ) noexcept {
                const auto state = State();

                if (
                    state == DedicatedThreadOperationalState::Running ||
                    state == DedicatedThreadOperationalState::RunningStopRequested
                ) {
                    return DedicatedThreadControlStartResult::AlreadyRunning;
                }

                const auto nextPhase = static_cast<std::uint8_t>(
                    (_value ^ PhaseMask) & PhaseMask
                );

                _value = static_cast<std::uint8_t>(
                    nextPhase |
                    static_cast<std::uint8_t>(DedicatedThreadOperationalState::Running)
                );

                activationPhase = nextPhase != 0U;
                return DedicatedThreadControlStartResult::Started;
            }

            /// Requests cooperative stop for the current activation.
            ///
            /// The owning Dedicated Thread runtime serializes every mutation through its mutex.
            DedicatedThreadControlStopRequestResult TryRequestStop() noexcept {
                if (State() != DedicatedThreadOperationalState::Running) {
                    return DedicatedThreadControlStopRequestResult::NotRunning;
                }

                _value = static_cast<std::uint8_t>(
                    (_value & PhaseMask) |
                    static_cast<std::uint8_t>(DedicatedThreadOperationalState::RunningStopRequested)
                );

                return DedicatedThreadControlStopRequestResult::Accepted;
            }

            /// Publishes completion only for the activation Phase that actually returned.
            ///
            /// The owning Dedicated Thread runtime serializes every mutation through its mutex.
            DedicatedThreadControlPublicationResult TryPublishStopped(
                bool activationPhase
            ) noexcept {
                if (((_value & PhaseMask) != 0U) != activationPhase) {
                    return DedicatedThreadControlPublicationResult::PhaseMismatch;
                }

                const auto state = State();

                if (
                    state != DedicatedThreadOperationalState::Running &&
                    state != DedicatedThreadOperationalState::RunningStopRequested
                ) {
                    return DedicatedThreadControlPublicationResult::NotRunning;
                }

                _value = static_cast<std::uint8_t>(
                    (_value & PhaseMask) |
                    static_cast<std::uint8_t>(DedicatedThreadOperationalState::Stopped)
                );

                return DedicatedThreadControlPublicationResult::Published;
            }

    };

} // ESPressio::Threading::Detail
