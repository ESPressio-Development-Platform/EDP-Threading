#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace ESPressio::Threading::Detail {

    template<class TRecordIndex, class TContextIndex>
    struct TaskWaitRegistration final {

        // Wait target.

        /// Target Task record index or the invalid sentinel when inactive.
        TRecordIndex RecordIndex;

        /// Target Task record incarnation Phase.
        bool Phase;

        /// Waiting managed execution context.
        TContextIndex ContextIndex;

    };


    template<class TContextIndex>
    struct ThreadJoinRegistration final {

        // Wait target.

        /// Target Dedicated Thread activation Phase.
        bool Phase;

        /// Waiting managed execution context or the invalid sentinel when inactive.
        TContextIndex ContextIndex;

    };


    template<class TContextIndex>
    struct ShutdownWaitRegistration final {

        // Waiting context.

        /// Waiting managed execution context or the invalid sentinel when inactive.
        TContextIndex ContextIndex;

    };


    template<class TRegistration, class TInactivePredicate>
    class RegistrationSet final {

        private:

            // Bounded registration storage.

            /// Caller-defined registration records; inactivity is encoded structurally by the registration itself.
            TRegistration* _registrations;

            /// Number of statically provisioned registration slots.
            std::size_t _capacity;

            /// Predicate that identifies an inactive registration slot.
            TInactivePredicate _isInactive;

        public:

            // Construction.

            /// Binds this bounded registration view to statically owned target-resource storage.
            RegistrationSet(
                TRegistration* registrations,
                std::size_t capacity,
                TInactivePredicate isInactive
            ) noexcept :
                _registrations(registrations),
                _capacity(capacity),
                _isInactive(isInactive) {}


            // Mutation.

            /// Attempts to reserve one inactive registration slot.
            TRegistration* TryAcquire() noexcept {
                for (std::size_t index = 0U; index < _capacity; ++index) {
                    if (_isInactive(_registrations[index])) {
                        return &_registrations[index];
                    }
                }

                return nullptr;
            }


            // Iteration.

            /// Visits every active registration in bounded storage order.
            template<class TVisitor>
            void VisitActive(
                TVisitor&& visitor
            ) {
                for (std::size_t index = 0U; index < _capacity; ++index) {
                    if (_isInactive(_registrations[index])) { continue; }

                    visitor(
                        _registrations[index]
                    );
                }
            }

    };

} // ESPressio::Threading::Detail
