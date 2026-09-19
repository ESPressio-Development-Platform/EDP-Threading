#pragma once

#include <cstddef>
#include <cstdint>

#include <ESPressio_Clock.hpp>
#include <ESPressio_Platform.hpp>

namespace ESPressio::Threading {

    enum class TaskState : std::uint8_t {
        Queued = 0,
        Running = 1,
        Completed = 2,
        Cancelled = 3
    };


    enum class TaskDispatchStatus : std::uint8_t {
        Succeeded = 0,
        Unavailable = 1,
        TimedOut = 2,
        Interrupted = 3,
        ShuttingDown = 4
    };


    enum class TaskWaitResult : std::uint8_t {
        Finished = 0,
        TimedOut = 1,
        Interrupted = 2
    };


    enum class TaskCancelResult : std::uint8_t {
        Accepted = 0,
        AlreadyFinished = 1
    };


    enum class TaskTakeStatus : std::uint8_t {
        Succeeded = 0,
        NotCompleted = 1,
        Cancelled = 2
    };


    enum class ThreadState : std::uint8_t {
        NeverStarted = 0,
        Running = 1,
        Stopped = 2
    };


    enum class ThreadStartResult : std::uint8_t {
        Started = 0,
        AlreadyRunning = 1,
        ActivationFailed = 2,
        ShuttingDown = 3
    };


    enum class ThreadStopRequestResult : std::uint8_t {
        Accepted = 0,
        NotRunning = 1
    };


    enum class ThreadJoinResult : std::uint8_t {
        Joined = 0,
        TimedOut = 1,
        Interrupted = 2,
        NeverStarted = 3
    };


    enum class SleepResult : std::uint8_t {
        Completed = 0,
        Interrupted = 1
    };


    enum class ShutdownWaitResult : std::uint8_t {
        Completed = 0,
        TimedOut = 1,
        Interrupted = 2
    };


    enum class ThreadingInitializationResult : std::uint8_t {
        Succeeded = 0,
        AlreadyInitialized = 1,
        InvalidTopology = 2,
        ProviderFailure = 3
    };


    enum class ThreadingStartResult : std::uint8_t {
        Succeeded = 0,
        NotInitialized = 1,
        AlreadyStarted = 2,
        ProviderFailure = 3
    };


    enum class ThreadingShutdownResult : std::uint8_t {
        Accepted = 0,
        AlreadyShuttingDown = 1,
        AlreadyCompleted = 2,
        NotStarted = 3
    };


    enum class TaskDispatchPolicy : std::uint8_t {
        Queue = 0,
        QueueWithTimeout = 1,
        AbandonImmediately = 2
    };


    using ThreadPriority = ESPressio::Platform::Execution::ExecutionPriority;
    using ProcessorAffinity = ESPressio::Platform::Execution::ProcessorAffinity;
    using Duration = ESPressio::Clock::Duration;
    using MonotonicTimestamp = ESPressio::Clock::MonotonicTimestamp;


    class TaskContext final {

        private:

            // Cancellation observation.

            /// Opaque active Task record supplied by the facility.
            const void* _context;

            /// Predicate used to inspect cancellation without duplicating control state.
            bool (*_isCancellationRequested)(const void*) noexcept;

        public:

            // Construction.

            /// Creates a lightweight view over one active Task's authoritative cancellation state.
            TaskContext(
                const void* context,
                bool (*isCancellationRequested)(const void*) noexcept
            ) noexcept :
                _context(context),
                _isCancellationRequested(isCancellationRequested) {}


            // Cancellation inspection.

            /// Indicates whether cooperative Task cancellation has been requested.
            bool IsCancellationRequested() const noexcept {
                return _isCancellationRequested(
                    _context
                );
            }

    };


    class ThreadContext final {

        private:

            // Stop observation.

            /// Opaque Dedicated Thread resource supplied by the runtime.
            const void* _context;

            /// Predicate used to inspect stop state without duplicating control state.
            bool (*_isStopRequested)(const void*) noexcept;

        public:

            // Construction.

            /// Creates a lightweight view over one activation's authoritative stop state.
            ThreadContext(
                const void* context,
                bool (*isStopRequested)(const void*) noexcept
            ) noexcept :
                _context(context),
                _isStopRequested(isStopRequested) {}


            // Stop inspection.

            /// Indicates whether cooperative Dedicated Thread stop has been requested.
            bool IsStopRequested() const noexcept {
                return _isStopRequested(
                    _context
                );
            }

    };

} // ESPressio::Threading
