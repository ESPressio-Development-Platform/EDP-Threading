#pragma once

#include <utility>

#include "ThreadingTypes.hpp"

namespace ESPressio::Threading {

    namespace Detail {

        /// Type-erased control operations bound to one topology-owned Dedicated Thread resource.
        struct ThreadHandleOperations final {

            // Lifecycle.

            /// Reads the current public Dedicated Thread state.
            ThreadState (*State)(const void*) noexcept;

            /// Starts a new semantic activation when no activation currently exists.
            ThreadStartResult (*Start)(void*) noexcept;

            /// Requests cooperative stop of the current activation.
            ThreadStopRequestResult (*RequestStop)(void*) noexcept;

            /// Joins the activation captured when this operation begins.
            ThreadJoinResult (*Join)(void*);

            /// Joins the captured activation using one relative physical-time budget.
            ThreadJoinResult (*JoinFor)(
                void*,
                Duration
            );

            /// Joins the captured activation until one canonical monotonic deadline.
            ThreadJoinResult (*JoinUntil)(
                void*,
                MonotonicTimestamp
            );

        };

    } // ESPressio::Threading::Detail


    template<class TThreadIdentity>
    class Thread final {

        private:

            // Handle binding.

            /// Opaque topology-owned Dedicated Thread resource.
            void* _resource;

            /// Static operations for the concrete Dedicated Thread resource.
            const Detail::ThreadHandleOperations* _operations;

        public:

            // Construction and lifetime.

            /// Creates a control handle for one topology-owned Dedicated Thread identity.
            Thread(
                void* resource,
                const Detail::ThreadHandleOperations& operations
            ) noexcept :
                _resource(resource),
                _operations(&operations) {}

            /// Prevents copying of the control handle.
            Thread(const Thread&) = delete;

            /// Prevents copy assignment of the control handle.
            Thread& operator =(const Thread&) = delete;

            /// Transfers this non-owning control handle.
            Thread(
                Thread&& other
            ) noexcept :
                _resource(other._resource),
                _operations(other._operations) {
                other._resource = nullptr;
                other._operations = nullptr;
            }

            /// Transfers this non-owning control handle.
            Thread& operator =(
                Thread&& other
            ) noexcept {
                if (this == &other) { return *this; }

                _resource = other._resource;
                _operations = other._operations;
                other._resource = nullptr;
                other._operations = nullptr;

                return *this;
            }

            /// Destroys only this non-owning control handle.
            ~Thread() = default;


            // Observation.

            /// Indicates whether this handle remains bound to its topology-owned resource.
            bool IsValid() const noexcept {
                return _resource != nullptr;
            }

            /// Returns the current public Dedicated Thread lifecycle state.
            ThreadState State() const noexcept {
                if (!IsValid()) {
                    return ThreadState::NeverStarted;
                }

                return _operations->State(
                    _resource
                );
            }


            // Lifecycle control.

            /// Starts a semantic activation when the Dedicated Thread is not already running.
            ThreadStartResult Start() noexcept {
                if (!IsValid()) {
                    return ThreadStartResult::ShuttingDown;
                }

                return _operations->Start(
                    _resource
                );
            }

            /// Requests cooperative stop of the current semantic activation.
            ThreadStopRequestResult RequestStop() noexcept {
                if (!IsValid()) {
                    return ThreadStopRequestResult::NotRunning;
                }

                return _operations->RequestStop(
                    _resource
                );
            }

            /// Waits indefinitely for the activation captured at Join entry to stop.
            ThreadJoinResult Join() {
                if (!IsValid()) {
                    return ThreadJoinResult::NeverStarted;
                }

                return _operations->Join(
                    _resource
                );
            }

            /// Waits for the captured activation using one relative physical-time budget.
            ThreadJoinResult JoinFor(
                Duration duration
            ) {
                if (!IsValid()) {
                    return ThreadJoinResult::NeverStarted;
                }

                return _operations->JoinFor(
                    _resource,
                    duration
                );
            }

            /// Waits for the captured activation until one canonical monotonic deadline.
            ThreadJoinResult JoinUntil(
                MonotonicTimestamp deadline
            ) {
                if (!IsValid()) {
                    return ThreadJoinResult::NeverStarted;
                }

                return _operations->JoinUntil(
                    _resource,
                    deadline
                );
            }

    };

} // ESPressio::Threading
