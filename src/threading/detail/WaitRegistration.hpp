#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include "TaskRecord.hpp"
#include "TopologyIndex.hpp"

namespace ESPressio::Threading::Detail {

    enum class WaitRegistrationStatus : std::uint8_t {
        Registered = 0,
        CapacityUnavailable = 1
    };


    /// Defines the compile-time contract for `ExecutionContextIndexTraits`.
    /// @tparam TContextCapacity Number of managed execution contexts represented by the topology.
    template<std::size_t TContextCapacity>
    struct ExecutionContextIndexTraits final {

        static_assert(
            TContextCapacity > 0U,
            "ExecutionContextIndex requires positive managed-context capacity"
        );

        /// Smallest index Type able to address every managed execution context plus an invalid sentinel.
        using Type = typename TopologyIndexTraits<
            ManagedContextIndexSpace,
            TContextCapacity
        >::Storage;

        /// Sentinel which cannot identify a valid managed execution context.
        static constexpr Type Invalid = TopologyIndexTraits<
            ManagedContextIndexSpace,
            TContextCapacity
        >::Invalid;

    };


    /// Defines the compile-time contract for `TaskWaitRegistration`.
    /// @tparam TRecordIndex Compact Task-record index Type stored by a wait registration.
    /// @tparam TContextIndex Dense managed execution-context index Type stored by a registration.
    template<class TRecordIndex, class TContextIndex>
    struct TaskWaitRegistration final {

        /// Managed execution-context index Type used by this registration.
        using ContextIndexType = TContextIndex;

        // Wait target.

        /// Target Task record index.
        TRecordIndex RecordIndex{};

        /// Target Task record incarnation Phase.
        bool Phase = false;


        // Wake routing.

        /// Waiting managed execution context, or the invalid sentinel while inactive.
        TContextIndex WaitingContextIndex = std::numeric_limits<TContextIndex>::max();


        // Registration state.

        /// Indicates whether this registration currently participates in target wake discovery.
        bool IsActive() const noexcept {
            return WaitingContextIndex != std::numeric_limits<TContextIndex>::max();
        }

        /// Returns this registration to its structurally inactive state.
        void Clear() noexcept {
            WaitingContextIndex = std::numeric_limits<TContextIndex>::max();
        }

    };


    /// Defines the compile-time contract for `ThreadJoinRegistration`.
    /// @tparam TContextIndex Dense managed execution-context index Type stored by a registration.
    template<class TContextIndex>
    struct ThreadJoinRegistration final {

        /// Managed execution-context index Type used by this registration.
        using ContextIndexType = TContextIndex;

        // Wait target.

        /// Target Dedicated Thread activation Phase.
        bool Phase = false;


        // Wake routing.

        /// Waiting managed execution context, or the invalid sentinel while inactive.
        TContextIndex WaitingContextIndex = std::numeric_limits<TContextIndex>::max();


        // Registration state.

        /// Indicates whether this registration currently participates in target wake discovery.
        bool IsActive() const noexcept {
            return WaitingContextIndex != std::numeric_limits<TContextIndex>::max();
        }

        /// Returns this registration to its structurally inactive state.
        void Clear() noexcept {
            WaitingContextIndex = std::numeric_limits<TContextIndex>::max();
        }

    };


    /// Defines the compile-time contract for `AdmissionWaitRegistration`.
    /// @tparam TContextIndex Dense managed execution-context index Type stored by a registration.
    template<class TContextIndex>
    struct AdmissionWaitRegistration final {

        /// Managed execution-context index Type used by this registration.
        using ContextIndexType = TContextIndex;

        // Wake routing.

        /// Waiting managed execution context, or the invalid sentinel while inactive.
        TContextIndex WaitingContextIndex = std::numeric_limits<TContextIndex>::max();


        // Registration state.

        /// Indicates whether this registration currently participates in admission-capacity wake discovery.
        bool IsActive() const noexcept {
            return WaitingContextIndex != std::numeric_limits<TContextIndex>::max();
        }

        /// Returns this registration to its structurally inactive state.
        void Clear() noexcept {
            WaitingContextIndex = std::numeric_limits<TContextIndex>::max();
        }

    };


    /// Defines the compile-time contract for `ShutdownWaitRegistration`.
    /// @tparam TContextIndex Dense managed execution-context index Type stored by a registration.
    template<class TContextIndex>
    struct ShutdownWaitRegistration final {

        /// Managed execution-context index Type used by this registration.
        using ContextIndexType = TContextIndex;

        // Wake routing.

        /// Waiting managed execution context, or the invalid sentinel while inactive.
        TContextIndex WaitingContextIndex = std::numeric_limits<TContextIndex>::max();


        // Registration state.

        /// Indicates whether this registration currently participates in shutdown wake discovery.
        bool IsActive() const noexcept {
            return WaitingContextIndex != std::numeric_limits<TContextIndex>::max();
        }

        /// Returns this registration to its structurally inactive state.
        void Clear() noexcept {
            WaitingContextIndex = std::numeric_limits<TContextIndex>::max();
        }

    };


    /// Defines the compile-time contract for `RegistrationSet`.
    /// @tparam TRegistration Registration record Type stored in the bounded registration set.
    /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
    template<class TRegistration, std::size_t TCapacity>
    class RegistrationSet final {

        static_assert(
            TCapacity > 0U,
            "A bounded waiter-registration set requires positive capacity"
        );

        private:

            // Target-owned bounded storage.

            /// Specialized waiter registrations owned directly by the target resource.
            TRegistration _registrations[TCapacity];

        public:

            // Registration lifecycle.

            /// Publishes one complete active registration into the first inactive slot.
            ///
            /// The target resource must serialize this operation with target predicate checks,
            /// terminal/phase publication and registration removal.
            WaitRegistrationStatus Register(
                const TRegistration& registration,
                std::size_t& registrationIndex
            ) noexcept {
                for (std::size_t index = 0U; index < TCapacity; ++index) {
                    if (_registrations[index].IsActive()) { continue; }

                    _registrations[index] = registration;
                    registrationIndex = index;
                    return WaitRegistrationStatus::Registered;
                }

                return WaitRegistrationStatus::CapacityUnavailable;
            }

            /// Removes one previously published registration.
            ///
            /// The caller must supply an index obtained from a successful Register operation and
            /// must serialize removal with the owning target resource's publication protocol.
            void Unregister(
                std::size_t registrationIndex
            ) noexcept {
                _registrations[registrationIndex].Clear();
            }


            // Inspection.

            /// Returns the number of currently active registrations.
            std::size_t ActiveCount() const noexcept {
                std::size_t activeCount = 0U;

                for (std::size_t index = 0U; index < TCapacity; ++index) {
                    if (_registrations[index].IsActive()) {
                        ++activeCount;
                    }
                }

                return activeCount;
            }

            /// Returns the number of active registrations satisfying one bounded predicate.
            /// @tparam TPredicate Predicate callable Type used to locate a registration.
            template<class TPredicate>
            std::size_t MatchingCount(
                TPredicate&& predicate
            ) const {
                std::size_t matchingCount = 0U;

                for (std::size_t index = 0U; index < TCapacity; ++index) {
                    if (
                        _registrations[index].IsActive() &&
                        predicate(
                            _registrations[index]
                        )
                    ) {
                        ++matchingCount;
                    }
                }

                return matchingCount;
            }


            // Iteration.

            /// Visits every active registration in bounded storage order.
            /// @tparam TVisitor Visitor callable Type invoked for active registrations.
            template<class TVisitor>
            void VisitActive(
                TVisitor&& visitor
            ) {
                for (std::size_t index = 0U; index < TCapacity; ++index) {
                    if (!_registrations[index].IsActive()) { continue; }

                    visitor(
                        _registrations[index]
                    );
                }
            }

    };

} // ESPressio::Threading::Detail
