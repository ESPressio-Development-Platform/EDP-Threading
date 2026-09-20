#pragma once

#include "Task.hpp"
#include "TaskCompletion.hpp"
#include "TaskResults.hpp"
#include "Thread.hpp"
#include "ThreadingComposition.hpp"
#include "detail/StaticTopologyOwner.hpp"
#include "ThreadingTypes.hpp"


namespace ESPressio::Threading {

    /// Concrete statically owned runtime for one declared Threading topology.
    /// @tparam TTopology Compile-time Threading topology being realized or inspected.
    /// @tparam TBindings Tuple Type containing application Dedicated Thread callable bindings.
    /// @tparam TSignalProvider Concrete Platform Signal provider Type used for targeted wakes.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed contexts.
    /// @tparam TSpinLockProvider Concrete Platform SpinLock provider Type protecting topology lifecycle publication.
    /// @tparam TMutexProvider Concrete Platform Mutex provider Type protecting resource-local state.
    template<class TTopology, class TBindings, class TSignalProvider, class TExecutionContextProvider, class TSpinLockProvider, class TMutexProvider>
    using StaticThreadingRuntime = Detail::StaticTopologyOwner<
        TTopology,
        TBindings,
        TSignalProvider,
        TExecutionContextProvider,
        TSpinLockProvider,
        TMutexProvider
    >;

} // ESPressio::Threading
