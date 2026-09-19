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

            /// Address of the cancellation predicate owned by the active Task record.
            const bool* _cancellationRequested;

        public:

            // Construction.

            /// Creates a Task execution context over one stable cancellation predicate.
            explicit TaskContext(
                const bool& cancellationRequested
            ) noexcept :
                _cancellationRequested(&cancellationRequested) {}


            // Cancellation inspection.

            /// Indicates whether cooperative Task cancellation has been requested.
            bool IsCancellationRequested() const noexcept {
                return *_cancellationRequested;
            }

    };


    class ThreadContext final {

        private:

            // Stop observation.

            /// Address of the stop predicate owned by the Dedicated Thread resource.
            const bool* _stopRequested;

        public:

            // Construction.

            /// Creates a Thread execution context over one stable stop predicate.
            explicit ThreadContext(
                const bool& stopRequested
            ) noexcept :
                _stopRequested(&stopRequested) {}


            // Stop inspection.

            /// Indicates whether cooperative Dedicated Thread stop has been requested.
            bool IsStopRequested() const noexcept {
                return *_stopRequested;
            }

    };

} // ESPressio::Threading
