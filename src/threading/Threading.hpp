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
