#pragma once

#include <new>
#include <type_traits>
#include <utility>

#include "../TaskCompletion.hpp"
#include "../ThreadingTypes.hpp"
#include "TaskRecord.hpp"

namespace ESPressio::Threading::Detail {

    template<class TRecord, class TCallable, class TResult>
    struct TaskPayloadAdapter final {

        static_assert(
            sizeof(TCallable) <= TRecord::PayloadCapacity,
            "Task callable exceeds the configured facility callable/result payload"
        );

        static_assert(
            sizeof(TResult) <= TRecord::PayloadCapacity,
            "Task result exceeds the configured facility callable/result payload"
        );

        static_assert(
            alignof(TCallable) <= alignof(std::max_align_t) &&
            alignof(TResult) <= alignof(std::max_align_t),
            "Over-aligned Task callable/result Types are not supported by the v1 bounded payload"
        );

        // Cancellation view.

        /// Reads the authoritative cancellation-request state from one Task record.
        static bool IsCancellationRequested(
            const void* context
        ) noexcept {
            return static_cast<const TRecord*>(context)->Control.IsCancellationRequested();
        }


        // Payload execution.

        /// Invokes the stored callable and establishes any result payload without publishing terminal lifecycle state.
        static TaskInvocationOutcome Invoke(
            TRecord& record
        ) {
            auto* callable = reinterpret_cast<TCallable*>(record.Payload);
            TaskContext context(
                &record,
                &IsCancellationRequested
            );

            if constexpr (
                std::is_invocable_r_v<
                    TaskCompletion<TResult>,
                    TCallable&,
                    TaskContext&
                >
            ) {
                auto completion = (*callable)(
                    context
                );

                callable->~TCallable();

                if (completion.IsCancelled()) {
                    return TaskInvocationOutcome::Cancelled;
                }

                new (record.Payload) TResult(
                    completion.TakeResult()
                );

                return TaskInvocationOutcome::Completed;
            } else {
                static_assert(
                    std::is_invocable_r_v<TResult, TCallable&>,
                    "Task callable must return TResult or TaskCompletion<TResult>"
                );

                TResult result = (*callable)();
                callable->~TCallable();

                new (record.Payload) TResult(
                    std::move(result)
                );

                return TaskInvocationOutcome::Completed;
            }
        }

        /// Destroys the currently published callable/result payload.
        ///
        /// This operation must not be called during the short Worker-owned interval after Invoke
        /// has replaced the callable payload but before the facility publishes terminal state.
        static void Destroy(
            TRecord& record
        ) noexcept {
            if (record.Control.State() == TaskOperationalState::Completed) {
                reinterpret_cast<TResult*>(record.Payload)->~TResult();
            } else if (
                record.Control.State() == TaskOperationalState::Queued ||
                record.Control.State() == TaskOperationalState::Running ||
                record.Control.State() == TaskOperationalState::RunningCancelRequested
            ) {
                reinterpret_cast<TCallable*>(record.Payload)->~TCallable();
            }
        }

        /// Moves the completed result into caller-provided typed storage and destroys the in-record result.
        static void MoveResult(
            TRecord& record,
            void* destination
        ) {
            auto* result = reinterpret_cast<TResult*>(record.Payload);

            new (destination) TResult(
                std::move(*result)
            );

            result->~TResult();
        }

        /// Shared immutable operation table for this callable/result pairing.
        inline static const TaskPayloadOperations<TRecord> Operations {
            &Invoke,
            &Destroy,
            &MoveResult
        };

    };


    template<class TRecord, class TCallable>
    struct TaskPayloadAdapter<TRecord, TCallable, void> final {

        static_assert(
            sizeof(TCallable) <= TRecord::PayloadCapacity,
            "Task callable exceeds the configured facility callable payload"
        );

        static_assert(
            alignof(TCallable) <= alignof(std::max_align_t),
            "Over-aligned Task callable Types are not supported by the v1 bounded payload"
        );

        // Cancellation view.

        /// Reads the authoritative cancellation-request state from one Task record.
        static bool IsCancellationRequested(
            const void* context
        ) noexcept {
            return static_cast<const TRecord*>(context)->Control.IsCancellationRequested();
        }


        // Payload execution.

        /// Invokes the stored void callable without publishing terminal lifecycle state.
        static TaskInvocationOutcome Invoke(
            TRecord& record
        ) {
            auto* callable = reinterpret_cast<TCallable*>(record.Payload);
            TaskContext context(
                &record,
                &IsCancellationRequested
            );

            if constexpr (
                std::is_invocable_r_v<
                    TaskCompletion<void>,
                    TCallable&,
                    TaskContext&
                >
            ) {
                const auto completion = (*callable)(
                    context
                );

                callable->~TCallable();

                return completion.IsCancelled()
                    ? TaskInvocationOutcome::Cancelled
                    : TaskInvocationOutcome::Completed;
            } else {
                static_assert(
                    std::is_invocable_r_v<void, TCallable&>,
                    "Void Task callable must return void or TaskCompletion<void>"
                );

                (*callable)();
                callable->~TCallable();

                return TaskInvocationOutcome::Completed;
            }
        }

        /// Destroys a live callable payload when execution has not consumed it.
        ///
        /// This operation must not be called during the short Worker-owned interval after Invoke
        /// has consumed the callable but before the facility publishes terminal state.
        static void Destroy(
            TRecord& record
        ) noexcept {
            if (
                record.Control.State() == TaskOperationalState::Queued ||
                record.Control.State() == TaskOperationalState::Running ||
                record.Control.State() == TaskOperationalState::RunningCancelRequested
            ) {
                reinterpret_cast<TCallable*>(record.Payload)->~TCallable();
            }
        }

        /// Void Tasks have no result payload to move.
        static void MoveResult(
            TRecord&,
            void*
        ) {}

        /// Shared immutable operation table for this callable/void pairing.
        inline static const TaskPayloadOperations<TRecord> Operations {
            &Invoke,
            &Destroy,
            &MoveResult
        };

    };

} // ESPressio::Threading::Detail
