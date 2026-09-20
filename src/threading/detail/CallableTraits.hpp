#pragma once

#include <type_traits>

#include "../TaskCompletion.hpp"
#include "../ThreadingTypes.hpp"

namespace ESPressio::Threading::Detail {

    /// Defines the compile-time contract for `TaskCompletionResult`.
    /// @tparam TCompletion Task-completion Type being classified.
    template<class TCompletion>
    struct TaskCompletionResult;


    /// Defines the compile-time contract for `TaskCompletionResult`.
    /// @tparam TResult Result Type produced or carried by the Task.
    template<class TResult>
    struct TaskCompletionResult<TaskCompletion<TResult>> {

        using Type = TResult;

    };


    /// Defines the compile-time contract for `CallableResultSelector`.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    /// @tparam TAcceptsContext Whether the callable accepts a TaskContext parameter.
    template<class TCallable, bool TAcceptsContext>
    struct CallableResultSelector;


    /// Defines the compile-time contract for `CallableResultSelector`.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    template<class TCallable>
    struct CallableResultSelector<TCallable, false> {

        using Type = std::invoke_result_t<TCallable&>;

    };


    /// Defines the compile-time contract for `CallableResultSelector`.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    template<class TCallable>
    struct CallableResultSelector<TCallable, true> {

        using Completion = std::invoke_result_t<TCallable&, TaskContext&>;
        using Type = typename TaskCompletionResult<Completion>::Type;

    };


    /// Defines the compile-time contract for `CallableResult`.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    template<class TCallable>
    struct CallableResult final {

        static constexpr bool AcceptsContext =
            std::is_invocable_v<TCallable&, TaskContext&>;

        using Type = typename CallableResultSelector<
            TCallable,
            AcceptsContext
        >::Type;

    };


    /// Defines the compile-time contract for `CallableResultT`.
    /// @tparam TCallable Callable Type whose invocation/storage contract is being adapted.
    template<class TCallable>
    using CallableResultT = typename CallableResult<TCallable>::Type;

} // ESPressio::Threading::Detail
