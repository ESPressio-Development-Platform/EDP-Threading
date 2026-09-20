#pragma once

#include <optional>
#include <utility>

namespace ESPressio::Threading {

    /// Defines the compile-time contract for `TaskCompletion`.
    /// @tparam TResult Result Type produced or carried by the Task.
    template<class TResult>
    class TaskCompletion final {

        private:

            // Completion payload.

            /// Optional successfully completed result.
            std::optional<TResult> _result;


            // Internal construction.

            /// Creates an acknowledged-cancellation completion.
            TaskCompletion() = default;

            /// Creates a successfully completed result.
            explicit TaskCompletion(
                TResult result
            ) :
                _result(
                    std::move(
                        result
                    )
                ) {}

        public:

            // Construction.

            /// Creates an acknowledged-cancellation completion.
            static TaskCompletion Cancelled() {
                return TaskCompletion();
            }

            /// Creates a normal completion carrying the callable result.
            static TaskCompletion Completed(
                TResult result
            ) {
                return TaskCompletion(
                    std::move(result)
                );
            }


            // Inspection.

            /// Indicates whether callable execution acknowledged cancellation.
            bool IsCancelled() const noexcept {
                return !_result.has_value();
            }


            // Result extraction.

            /// Moves the successfully completed result from this completion object.
            TResult TakeResult() {
                return std::move(
                    _result.value()
                );
            }

    };


    template<>
    class TaskCompletion<void> final {

        private:

            // Completion state.

            /// Indicates whether callable execution acknowledged cancellation.
            bool _cancelled;


            // Internal construction.

            /// Creates a void completion with the supplied cancellation state.
            explicit TaskCompletion(
                bool cancelled
            ) noexcept :
                _cancelled(cancelled) {}

        public:

            // Construction.

            /// Creates an acknowledged-cancellation completion.
            static TaskCompletion Cancelled() noexcept {
                return TaskCompletion(true);
            }

            /// Creates a normal void completion.
            static TaskCompletion Completed() noexcept {
                return TaskCompletion(false);
            }


            // Inspection.

            /// Indicates whether callable execution acknowledged cancellation.
            bool IsCancelled() const noexcept {
                return _cancelled;
            }

    };

} // ESPressio::Threading
