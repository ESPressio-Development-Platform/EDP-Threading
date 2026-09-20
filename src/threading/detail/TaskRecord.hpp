#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "../ThreadingTypes.hpp"

namespace ESPressio::Threading::Detail {

    enum class TaskOperationalState : std::uint8_t {
        Queued = 0,
        Running = 1,
        RunningCancelRequested = 2,
        Completed = 3,
        Cancelled = 4
    };


    enum class TaskInvocationOutcome : std::uint8_t {
        Completed = 0,
        Cancelled = 1
    };


    /// Outcome of an internal Task control-state transition.
    enum class TaskControlTransitionResult : std::uint8_t {
        Applied = 0,
        StateMismatch = 1
    };


    /// Defines the compile-time contract for `SmallestIndex`.
    /// @tparam TCapacity Compile-time bounded capacity represented by this Type.
    template<std::size_t TCapacity>
    struct SmallestIndex {

        static_assert(
            TCapacity > 0U,
            "A bounded index requires a positive capacity"
        );

        static_assert(
            TCapacity <= static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()),
            "A bounded index capacity must fit within the largest supported 32-bit index plus its sentinel"
        );

        /// Smallest unsigned index type able to represent every record plus an invalid sentinel.
        using Type = std::conditional_t<
            (TCapacity <= static_cast<std::size_t>(std::numeric_limits<std::uint8_t>::max())),
            std::uint8_t,
            std::conditional_t<
                (TCapacity <= static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max())),
                std::uint16_t,
                std::uint32_t
            >
        >;

        /// Sentinel value which cannot identify a valid record for the configured capacity.
        static constexpr Type Invalid = std::numeric_limits<Type>::max();

    };


    class TaskControl final {

        private:

            // Packed control byte.

            /// Packed operational state, public ownership and incarnation Phase.
            std::uint8_t _value;

            /// Bit mask selecting the internal Task operational state.
            static constexpr std::uint8_t StateMask = 0x07U;

            /// Bit mask selecting the sole public Task ownership interest.
            static constexpr std::uint8_t OwnerMask = 0x08U;

            /// Bit mask selecting the record-incarnation Phase.
            static constexpr std::uint8_t PhaseMask = 0x10U;

        public:

            TaskControl() noexcept :
                _value(0U) {}


            // State inspection.

            /// Returns the internal operational state.
            TaskOperationalState State() const noexcept {
                return static_cast<TaskOperationalState>(
                    _value & StateMask
                );
            }

            /// Indicates whether the sole public Task ownership interest exists.
            bool HasOwner() const noexcept {
                return (_value & OwnerMask) != 0U;
            }

            /// Returns the current one-bit record-incarnation Phase.
            bool Phase() const noexcept {
                return (_value & PhaseMask) != 0U;
            }

            /// Indicates whether cooperative cancellation has been requested while Running.
            bool IsCancellationRequested() const noexcept {
                return State() == TaskOperationalState::RunningCancelRequested;
            }


            // State mutation.

            /// Initializes a newly claimed record and toggles its incarnation Phase.
            void InitializeQueued() noexcept {
                const auto current = _value;
                const auto nextPhase = static_cast<std::uint8_t>(
                    (current ^ PhaseMask) & PhaseMask
                );

                _value = static_cast<std::uint8_t>(
                    nextPhase |
                    OwnerMask |
                    static_cast<std::uint8_t>(TaskOperationalState::Queued)
                );
            }

            /// Replaces only the operational state while preserving owner and Phase bits.
            void SetState(
                TaskOperationalState state
            ) noexcept {
                _value = static_cast<std::uint8_t>(
                    (_value & static_cast<std::uint8_t>(~StateMask)) |
                    static_cast<std::uint8_t>(state)
                );
            }

            /// Changes the operational state only when the expected state still owns the transition.
            ///
            /// The owning facility runtime serializes every mutation through its mutex.
            TaskControlTransitionResult CompareExchangeState(
                TaskOperationalState expectedState,
                TaskOperationalState desiredState
            ) noexcept {
                if (State() != expectedState) {
                    return TaskControlTransitionResult::StateMismatch;
                }

                SetState(
                    desiredState
                );

                return TaskControlTransitionResult::Applied;
            }

            /// Releases the sole public ownership interest.
            ///
            /// The owning facility runtime serializes every mutation through its mutex.
            void ReleaseOwner() noexcept {
                _value = static_cast<std::uint8_t>(
                    _value & static_cast<std::uint8_t>(~OwnerMask)
                );
            }

    };


    /// Defines the compile-time contract for `TaskPayloadOperations`.
    /// @tparam TTaskRecord Task-record Type inspected by the helper.
    template<class TTaskRecord>
    struct TaskPayloadOperations final {

        // Payload lifecycle.

        /// Executes the callable and establishes any result payload without publishing terminal lifecycle state.
        TaskInvocationOutcome (*Invoke)(TTaskRecord&, TaskContext&);

        /// Destroys the callable payload before execution has consumed it.
        void (*DestroyCallable)(TTaskRecord&) noexcept;

        /// Destroys a completed result payload when one exists.
        void (*DestroyResult)(TTaskRecord&) noexcept;

        /// Moves a completed result into caller-provided typed storage.
        void (*MoveResult)(
            TTaskRecord&,
            void*
        );

    };


    /// Defines the compile-time contract for `TaskRecord`.
    /// @tparam TCallableCapacity Callable-storage capacity declaration Type.
    /// @tparam TResultCapacity Result-storage capacity declaration Type.
    /// @tparam TRecordCapacity Task-record capacity declaration Type.
    /// @tparam TExecutionContextCapacity Total managed execution-context capacity of the topology.
    template<std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TRecordCapacity, std::size_t TExecutionContextCapacity>
    struct TaskRecord final {

        static_assert(
            TCallableCapacity > 0U,
            "Task callable capacity must be positive"
        );

        static_assert(
            TExecutionContextCapacity > 0U,
            "Task records require positive managed execution-context capacity"
        );

        static_assert(
            sizeof(TaskControl) == 1U,
            "TaskControl must remain a one-byte intrinsic Task control representation"
        );

        /// Largest index capacity needed by the mutually exclusive queue/Worker scratch field.
        static constexpr std::size_t ScratchCapacity =
            TRecordCapacity > TExecutionContextCapacity
                ? TRecordCapacity
                : TExecutionContextCapacity;

        /// Payload bytes shared by callable storage and result storage.
        static constexpr std::size_t PayloadCapacity =
            TCallableCapacity > TResultCapacity
                ? TCallableCapacity
                : TResultCapacity;

        /// Smallest record-index type satisfying the configured record capacity.
        using Index = typename SmallestIndex<TRecordCapacity>::Type;

        /// Smallest managed execution-context index Type satisfying the complete topology capacity.
        using ExecutionContextIndex = typename SmallestIndex<TExecutionContextCapacity>::Type;

        /// Sentinel which cannot identify a valid managed execution context.
        static constexpr ExecutionContextIndex InvalidExecutionContextIndex =
            SmallestIndex<TExecutionContextCapacity>::Invalid;

        /// Smallest scratch Type able to hold either a queue link or an execution-context index.
        using ScratchIndex = typename SmallestIndex<ScratchCapacity>::Type;

        // Reusable callable/result payload.

        /// Storage reused first by the callable and then by its result.
        alignas(std::max_align_t) std::byte Payload[PayloadCapacity];


        // Mutually exclusive transient linkage.

        /// Queued: next record index. Running: managed execution-context index owning execution.
        ScratchIndex QueueOrExecutionContext =
            static_cast<ScratchIndex>(
                SmallestIndex<TRecordCapacity>::Invalid
            );


        // Compact intrinsic control.

        /// One-byte Task lifecycle/ownership/Phase representation.
        TaskControl Control;


        // Type-erased payload lifecycle.

        /// One pointer to immutable Type-specific payload operations shared by all records of that callable/result pairing.
        const TaskPayloadOperations<TaskRecord>* PayloadOperations = nullptr;


        // Scratch interpretation.

        /// Stores the next queued record index in the mutually exclusive scratch field.
        void SetQueueNext(
            Index recordIndex
        ) noexcept {
            QueueOrExecutionContext = static_cast<ScratchIndex>(
                recordIndex
            );
        }

        /// Returns the next queued record index from the mutually exclusive scratch field.
        Index QueueNext() const noexcept {
            return static_cast<Index>(
                QueueOrExecutionContext
            );
        }

        /// Stores the managed execution-context index after this record is granted to a Worker.
        void SetExecutionContextIndex(
            ExecutionContextIndex contextIndex
        ) noexcept {
            QueueOrExecutionContext = static_cast<ScratchIndex>(
                contextIndex
            );
        }

        /// Returns the managed execution-context index owning current Worker execution.
        ExecutionContextIndex CurrentExecutionContextIndex() const noexcept {
            return static_cast<ExecutionContextIndex>(
                QueueOrExecutionContext
            );
        }

    };

} // ESPressio::Threading::Detail
