#pragma once

#include <cstddef>
#include <new>
#include <utility>

#include "TaskResults.hpp"
#include "ThreadingTypes.hpp"

namespace ESPressio::Threading {

    namespace Detail {

        /// Type-erased operations bound to one concrete Task facility without dynamic allocation.
        struct TaskHandleOperations final {

            // Observation.

            /// Reads the stable public lifecycle state for one Task record.
            TaskState (*State)(const void*, const void*) noexcept;

            /// Waits indefinitely for one Task record to become terminal.
            TaskWaitResult (*Wait)(void*, void*);

            /// Waits for one Task record to become terminal within a relative duration.
            TaskWaitResult (*WaitFor)(
                void*,
                void*,
                Duration
            );

            /// Waits for one Task record to become terminal by a canonical monotonic deadline.
            TaskWaitResult (*WaitUntil)(
                void*,
                void*,
                MonotonicTimestamp
            );

            /// Requests cooperative cancellation of one Task record.
            TaskCancelResult (*Cancel)(void*, void*) noexcept;

            /// Releases the sole public Task ownership interest.
            void (*Release)(void*, void*) noexcept;

            /// Moves a completed result into caller-provided typed storage.
            TaskTakeStatus (*TakeResult)(
                void*,
                void*,
                void*
            );

        };

    } // ESPressio::Threading::Detail


    template<class TResult>
    class Task final {

        private:

            // Handle binding.

            /// Opaque address of the concrete facility owning this Task.
            void* _owner;

            /// Opaque address of the concrete Task record.
            void* _record;

            /// Static operations for the concrete facility owning the record.
            const Detail::TaskHandleOperations* _operations;


            // Internal lifecycle.

            /// Releases this handle's public ownership interest when one remains.
            void Release() noexcept {
                if (_record == nullptr) { return; }

                _operations->Release(
                    _owner,
                    _record
                );

                _owner = nullptr;
                _record = nullptr;
                _operations = nullptr;
            }

        public:

            // Construction and lifetime.

            /// Creates a valid Task handle for one successfully admitted record.
            Task(
                void* owner,
                void* record,
                const Detail::TaskHandleOperations& operations
            ) noexcept :
                _owner(owner),
                _record(record),
                _operations(&operations) {}

            /// Prevents shared public ownership through copying.
            Task(const Task&) = delete;

            /// Prevents shared public ownership through copy assignment.
            Task& operator =(const Task&) = delete;

            /// Transfers the sole public Task ownership interest.
            Task(
                Task&& other
            ) noexcept :
                _owner(other._owner),
                _record(other._record),
                _operations(other._operations) {
                other._owner = nullptr;
                other._record = nullptr;
                other._operations = nullptr;
            }

            /// Transfers the sole public Task ownership interest after releasing the current one.
            Task& operator =(
                Task&& other
            ) noexcept {
                if (this == &other) { return *this; }

                Release();

                _owner = other._owner;
                _record = other._record;
                _operations = other._operations;
                other._owner = nullptr;
                other._record = nullptr;
                other._operations = nullptr;

                return *this;
            }

            /// Releases public ownership without cancelling or waiting.
            ~Task() {
                Release();
            }


            // Observation.

            /// Indicates whether this handle still owns a valid Task observation/result interest.
            bool IsValid() const noexcept {
                return _record != nullptr;
            }

            /// Returns the stable public Task lifecycle state.
            TaskState State() const noexcept {
                return _operations->State(
                    _owner,
                    _record
                );
            }

            /// Indicates whether the Task completed normally.
            bool IsCompleted() const noexcept {
                return State() == TaskState::Completed;
            }

            /// Indicates whether the Task completed through cooperative cancellation.
            bool IsCancelled() const noexcept {
                return State() == TaskState::Cancelled;
            }

            /// Indicates whether the Task has reached either terminal lifecycle state.
            bool IsFinished() const noexcept {
                const auto state = State();

                return state == TaskState::Completed ||
                    state == TaskState::Cancelled;
            }


            // Waiting.

            /// Waits indefinitely until this Task becomes terminal or the waiting context is interrupted.
            TaskWaitResult Wait() {
                return _operations->Wait(
                    _owner,
                    _record
                );
            }

            /// Waits for this Task using one relative physical-time budget.
            TaskWaitResult WaitFor(
                Duration duration
            ) {
                return _operations->WaitFor(
                    _owner,
                    _record,
                    duration
                );
            }

            /// Waits for this Task until one canonical EDP monotonic deadline.
            TaskWaitResult WaitUntil(
                MonotonicTimestamp deadline
            ) {
                return _operations->WaitUntil(
                    _owner,
                    _record,
                    deadline
                );
            }


            // Cancellation.

            /// Requests cooperative cancellation of this Task.
            TaskCancelResult Cancel() noexcept {
                return _operations->Cancel(
                    _owner,
                    _record
                );
            }


            // Result extraction.

            /// Moves the completed result from the Task record and consumes this handle on success.
            TaskTakeResult<TResult> TakeResult() {
                alignas(TResult) std::byte resultStorage[sizeof(TResult)];

                const auto status = _operations->TakeResult(
                    _owner,
                    _record,
                    resultStorage
                );

                if (status != TaskTakeStatus::Succeeded) {
                    return TaskTakeResult<TResult>(status);
                }

                auto* result = reinterpret_cast<TResult*>(resultStorage);
                TaskTakeResult<TResult> taken(
                    std::move(*result)
                );
                result->~TResult();

                _owner = nullptr;
                _record = nullptr;
                _operations = nullptr;

                return taken;
            }

    };


    template<>
    class Task<void> final {

        private:

            // Handle binding.

            /// Opaque address of the concrete facility owning this Task.
            void* _owner;

            /// Opaque address of the concrete Task record.
            void* _record;

            /// Static operations for the concrete facility owning the record.
            const Detail::TaskHandleOperations* _operations;


            // Internal lifecycle.

            /// Releases this handle's public ownership interest when one remains.
            void Release() noexcept {
                if (_record == nullptr) { return; }

                _operations->Release(
                    _owner,
                    _record
                );

                _owner = nullptr;
                _record = nullptr;
                _operations = nullptr;
            }

        public:

            // Construction and lifetime.

            /// Creates a valid void Task handle for one successfully admitted record.
            Task(
                void* owner,
                void* record,
                const Detail::TaskHandleOperations& operations
            ) noexcept :
                _owner(owner),
                _record(record),
                _operations(&operations) {}

            /// Prevents shared public ownership through copying.
            Task(const Task&) = delete;

            /// Prevents shared public ownership through copy assignment.
            Task& operator =(const Task&) = delete;

            /// Transfers the sole public Task ownership interest.
            Task(
                Task&& other
            ) noexcept :
                _owner(other._owner),
                _record(other._record),
                _operations(other._operations) {
                other._owner = nullptr;
                other._record = nullptr;
                other._operations = nullptr;
            }

            /// Transfers the sole public Task ownership interest after releasing the current one.
            Task& operator =(
                Task&& other
            ) noexcept {
                if (this == &other) { return *this; }

                Release();

                _owner = other._owner;
                _record = other._record;
                _operations = other._operations;
                other._owner = nullptr;
                other._record = nullptr;
                other._operations = nullptr;

                return *this;
            }

            /// Releases public ownership without cancelling or waiting.
            ~Task() {
                Release();
            }


            // Observation.

            /// Indicates whether this handle still owns a valid Task observation interest.
            bool IsValid() const noexcept {
                return _record != nullptr;
            }

            /// Returns the stable public Task lifecycle state.
            TaskState State() const noexcept {
                return _operations->State(
                    _owner,
                    _record
                );
            }

            /// Indicates whether the Task completed normally.
            bool IsCompleted() const noexcept {
                return State() == TaskState::Completed;
            }

            /// Indicates whether the Task completed through cooperative cancellation.
            bool IsCancelled() const noexcept {
                return State() == TaskState::Cancelled;
            }

            /// Indicates whether the Task has reached either terminal lifecycle state.
            bool IsFinished() const noexcept {
                const auto state = State();

                return state == TaskState::Completed ||
                    state == TaskState::Cancelled;
            }


            // Waiting.

            /// Waits indefinitely until this Task becomes terminal or the waiting context is interrupted.
            TaskWaitResult Wait() {
                return _operations->Wait(
                    _owner,
                    _record
                );
            }

            /// Waits for this Task using one relative physical-time budget.
            TaskWaitResult WaitFor(
                Duration duration
            ) {
                return _operations->WaitFor(
                    _owner,
                    _record,
                    duration
                );
            }

            /// Waits for this Task until one canonical EDP monotonic deadline.
            TaskWaitResult WaitUntil(
                MonotonicTimestamp deadline
            ) {
                return _operations->WaitUntil(
                    _owner,
                    _record,
                    deadline
                );
            }


            // Cancellation.

            /// Requests cooperative cancellation of this Task.
            TaskCancelResult Cancel() noexcept {
                return _operations->Cancel(
                    _owner,
                    _record
                );
            }

    };

} // ESPressio::Threading
