#pragma once

#include <type_traits>

#include "../TaskCompletion.hpp"
#include "../ThreadingTypes.hpp"

namespace ESPressio::Threading::Detail {

    template<class TCompletion>
    struct TaskCompletionResult;


    template<class TResult>
    struct TaskCompletionResult<TaskCompletion<TResult>> {

        using Type = TResult;

    };


    template<class TCallable, bool TAcceptsContext>
    struct CallableResultSelector;


    template<class TCallable>
    struct CallableResultSelector<TCallable, false> {

        using Type = std::invoke_result_t<TCallable&>;

    };


    template<class TCallable>
    struct CallableResultSelector<TCallable, true> {

        using Completion = std::invoke_result_t<TCallable&, TaskContext&>;
        using Type = typename TaskCompletionResult<Completion>::Type;

    };


    template<class TCallable>
    struct CallableResult final {

        static constexpr bool AcceptsContext =
            std::is_invocable_v<TCallable&, TaskContext&>;

        using Type = typename CallableResultSelector<
            TCallable,
            AcceptsContext
        >::Type;

    };


    template<class TCallable>
    using CallableResultT = typename CallableResult<TCallable>::Type;

} // ESPressio::Threading::Detail
