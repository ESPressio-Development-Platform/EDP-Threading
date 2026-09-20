#pragma once

#include <optional>
#include <utility>

#include "ThreadingTypes.hpp"

namespace ESPressio::Threading {

    /// Defines the compile-time contract for `TaskDispatchResult`.
    /// @tparam TTask Concrete Task Type being inspected.
    template<class TTask>
    class TaskDispatchResult final {

        private:

            // Dispatch outcome.

            /// Operational dispatch status.
            TaskDispatchStatus _status;

            /// Task handle present only after successful dispatch.
            std::optional<TTask> _task;

        public:

            // Construction.

            /// Creates a failed dispatch result.
            explicit TaskDispatchResult(
                TaskDispatchStatus status
            ) noexcept :
                _status(status) {}

            /// Creates a successful dispatch result carrying the new Task handle.
            explicit TaskDispatchResult(
                TTask task
            ) :
                _status(TaskDispatchStatus::Succeeded),
                _task(std::move(task)) {}


            // Inspection.

            /// Returns the operational dispatch status.
            TaskDispatchStatus Status() const noexcept {
                return _status;
            }

            /// Indicates whether dispatch succeeded and a Task handle is available.
            bool IsSucceeded() const noexcept {
                return _status == TaskDispatchStatus::Succeeded &&
                    _task.has_value();
            }


            // Ownership extraction.

            /// Moves the successfully dispatched Task handle to the caller.
            TTask TakeTask() {
                return std::move(
                    _task.value()
                );
            }

    };


    /// Defines the compile-time contract for `TaskTakeResult`.
    /// @tparam TResult Result Type produced or carried by the Task.
    template<class TResult>
    class TaskTakeResult final {

        private:

            // Extraction outcome.

            /// Operational extraction status.
            TaskTakeStatus _status;

            /// Result payload present only after successful extraction.
            std::optional<TResult> _result;

        public:

            // Construction.

            /// Creates a failed extraction result.
            explicit TaskTakeResult(
                TaskTakeStatus status
            ) noexcept :
                _status(status) {}

            /// Creates a successful extraction result carrying the Task result.
            explicit TaskTakeResult(
                TResult result
            ) :
                _status(TaskTakeStatus::Succeeded),
                _result(std::move(result)) {}


            // Inspection.

            /// Returns the operational extraction status.
            TaskTakeStatus Status() const noexcept {
                return _status;
            }

            /// Indicates whether extraction succeeded and a result is available.
            bool IsSucceeded() const noexcept {
                return _status == TaskTakeStatus::Succeeded &&
                    _result.has_value();
            }


            // Result extraction.

            /// Moves the extracted Task result to the caller.
            TResult TakeResult() {
                return std::move(
                    _result.value()
                );
            }

    };

} // ESPressio::Threading
