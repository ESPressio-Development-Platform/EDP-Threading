#pragma once

#include <cstddef>
#include <type_traits>

#include <ESPressio_Memory.hpp>

#include "../TaskCompletion.hpp"
#include "../ThreadingTypes.hpp"
#include "TaskRecord.hpp"

namespace ESPressio::Threading::Detail {

    template<class TRecord, class TCallable, class TResult>
    struct TaskPayloadAdapter final {

        static_assert(sizeof(TCallable) <= TRecord::PayloadCapacity,
            "Task callable exceeds the configured facility callable/result payload");
        static_assert(sizeof(TResult) <= TRecord::PayloadCapacity,
            "Task result exceeds the configured facility callable/result payload");
        static_assert(alignof(TCallable) <= alignof(std::max_align_t) &&
            alignof(TResult) <= alignof(std::max_align_t),
            "Over-aligned Task callable/result Types are not supported by the v1 bounded payload");
        static_assert(std::is_nothrow_move_constructible_v<TResult>,
            "Task result ownership transfer must be nothrow move constructible");
        static_assert(std::is_nothrow_destructible_v<TCallable> && std::is_nothrow_destructible_v<TResult>,
            "Task payload destruction must be noexcept");

        static TaskInvocationOutcome Invoke(TRecord& record, TaskContext& context) {
            auto* callable = reinterpret_cast<TCallable*>(record.Payload);

            if constexpr (std::is_invocable_r_v<TaskCompletion<TResult>, TCallable&, TaskContext&>) {
                auto completion = (*callable)(context);
                ESPressio::Memory::ObjectLifetime::Destroy(*callable);
                if (completion.IsCancelled()) {
                    return TaskInvocationOutcome::Cancelled;
                }
                auto result = completion.TakeResult();
                static_cast<void>(ESPressio::Memory::ObjectLifetime::MoveConstruct<TResult>(record.Payload, result));
                return TaskInvocationOutcome::Completed;
            } else {
                static_cast<void>(context);
                static_assert(std::is_invocable_r_v<TResult, TCallable&>,
                    "Task callable must return TResult or TaskCompletion<TResult>");
                TResult result = (*callable)();
                ESPressio::Memory::ObjectLifetime::Destroy(*callable);
                static_cast<void>(ESPressio::Memory::ObjectLifetime::MoveConstruct<TResult>(record.Payload, result));
                return TaskInvocationOutcome::Completed;
            }
        }

        static void DestroyCallable(TRecord& record) noexcept {
            ESPressio::Memory::ObjectLifetime::Destroy(
                *reinterpret_cast<TCallable*>(record.Payload));
        }

        static void DestroyResult(TRecord& record) noexcept {
            ESPressio::Memory::ObjectLifetime::Destroy(
                *reinterpret_cast<TResult*>(record.Payload));
        }

        static void MoveResult(TRecord& record, void* destination) {
            auto* result = reinterpret_cast<TResult*>(record.Payload);
            static_cast<void>(ESPressio::Memory::ObjectLifetime::MoveConstruct<TResult>(destination, *result));
            ESPressio::Memory::ObjectLifetime::Destroy(*result);
        }

        inline static const TaskPayloadOperations<TRecord> Operations {
            &Invoke, &DestroyCallable, &DestroyResult, &MoveResult
        };
    };

    template<class TRecord, class TCallable>
    struct TaskPayloadAdapter<TRecord, TCallable, void> final {
        static_assert(sizeof(TCallable) <= TRecord::PayloadCapacity,
            "Task callable exceeds the configured facility callable payload");
        static_assert(alignof(TCallable) <= alignof(std::max_align_t),
            "Over-aligned Task callable Types are not supported by the v1 bounded payload");
        static_assert(std::is_nothrow_destructible_v<TCallable>,
            "Task callable destruction must be noexcept");

        static TaskInvocationOutcome Invoke(TRecord& record, TaskContext& context) {
            auto* callable = reinterpret_cast<TCallable*>(record.Payload);
            if constexpr (std::is_invocable_r_v<TaskCompletion<void>, TCallable&, TaskContext&>) {
                const auto completion = (*callable)(context);
                ESPressio::Memory::ObjectLifetime::Destroy(*callable);
                return completion.IsCancelled() ? TaskInvocationOutcome::Cancelled : TaskInvocationOutcome::Completed;
            } else {
                static_cast<void>(context);
                static_assert(std::is_invocable_r_v<void, TCallable&>,
                    "Void Task callable must return void or TaskCompletion<void>");
                (*callable)();
                ESPressio::Memory::ObjectLifetime::Destroy(*callable);
                return TaskInvocationOutcome::Completed;
            }
        }

        static void DestroyCallable(TRecord& record) noexcept {
            ESPressio::Memory::ObjectLifetime::Destroy(
                *reinterpret_cast<TCallable*>(record.Payload));
        }
        static void DestroyResult(TRecord&) noexcept {}
        static void MoveResult(TRecord&, void*) {}

        inline static const TaskPayloadOperations<TRecord> Operations {
            &Invoke, &DestroyCallable, &DestroyResult, &MoveResult
        };
    };

} // ESPressio::Threading::Detail
