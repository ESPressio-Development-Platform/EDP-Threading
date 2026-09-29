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


    enum class ThreadWakeResult : std::uint8_t {
        Woken = 0,
        ProviderFailure = 1
    };


    enum class ThreadWaitResult : std::uint8_t {
        Woken = 0,
        TimedOut = 1,
        ProviderFailure = 2
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


    enum class ThreadingFinalizationResult : std::uint8_t {
        Completed = 0,
        NotShuttingDown = 1,
        ExecutionNotQuiescent = 2,
        ProviderFailure = 3
    };


    enum class TaskDispatchPolicy : std::uint8_t {
        Queue = 0,
        QueueWithTimeout = 1,
        AbandonImmediately = 2
    };


    /// Logical execution-priority vocabulary shared with the Platform execution contract.
    using ThreadPriority = ESPressio::Platform::Execution::ExecutionPriority;
    /// Logical processor-affinity vocabulary shared with the Platform execution contract.
    using ProcessorAffinity = ESPressio::Platform::Execution::ProcessorAffinity;
    /// Canonical physical-duration Type supplied by EDP-Clock.
    using Duration = ESPressio::Clock::Duration;
    /// Canonical monotonic-coordinate Type supplied by EDP-Clock.
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


    namespace Detail {

        /// Type-erased operations bound to one active Dedicated Thread context.
        struct ThreadContextOperations final {
            bool (*IsStopRequested)(const void*) noexcept;
            ThreadWaitResult (*Wait)(void*) noexcept;
            ThreadWaitResult (*WaitFor)(void*, Duration) noexcept;
            ThreadWaitResult (*WaitUntil)(void*, MonotonicTimestamp) noexcept;
        };

    } // ESPressio::Threading::Detail


    class ThreadContext final {

        private:

            // Stop observation.

            /// Opaque Dedicated Thread resource supplied by the runtime.
            void* _context;

            /// Static operation table for this concrete Dedicated Thread runtime Type.
            const Detail::ThreadContextOperations* _operations;

        public:

            // Construction.

            /// Creates a lightweight view over one activation's authoritative stop state.
            ThreadContext(
                void* context,
                const Detail::ThreadContextOperations& operations
            ) noexcept :
                _context(context),
                _operations(&operations) {}


            // Stop inspection.

            /// Indicates whether cooperative Dedicated Thread stop has been requested.
            bool IsStopRequested() const noexcept {
                return _operations->IsStopRequested(
                    _context
                );
            }


            // Managed-context waiting.

            /// Waits indefinitely until this Dedicated Thread's reusable wake signal is published.
            ThreadWaitResult Wait() noexcept {
                return _operations->Wait(
                    _context
                );
            }

            /// Waits for a wake using one relative canonical monotonic-time budget.
            ThreadWaitResult WaitFor(
                Duration duration
            ) noexcept {
                return _operations->WaitFor(
                    _context,
                    duration
                );
            }

            /// Waits for a wake until one canonical monotonic deadline.
            ThreadWaitResult WaitUntil(
                MonotonicTimestamp deadline
            ) noexcept {
                return _operations->WaitUntil(
                    _context,
                    deadline
                );
            }

    };

} // ESPressio::Threading
