#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace ESPressio::Threading::Detail {

    enum class TaskOperationalState : std::uint8_t {
        Queued = 0,
        Running = 1,
        RunningCancelRequested = 2,
        Completed = 3,
        Cancelled = 4
    };


    template<std::size_t TCapacity>
    struct SmallestIndex {

        static_assert(
            TCapacity > 0U,
            "A bounded index requires a positive capacity"
        );

        using Type = std::conditional_t<
            (TCapacity < static_cast<std::size_t>(std::numeric_limits<std::uint8_t>::max())),
            std::uint8_t,
            std::conditional_t<
                (TCapacity < static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max())),
                std::uint16_t,
                std::uint32_t
            >
        >;

        static constexpr Type Invalid = std::numeric_limits<Type>::max();

    };


    class TaskControl final {

        private:

            // Packed control byte.

            /// Packed operational state, public ownership and incarnation Phase.
            std::uint8_t _value = 0U;

            static constexpr std::uint8_t StateMask = 0x07U;
            static constexpr std::uint8_t OwnerMask = 0x08U;
            static constexpr std::uint8_t PhaseMask = 0x10U;

        public:

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
                const auto nextPhase = static_cast<std::uint8_t>(
                    (_value ^ PhaseMask) & PhaseMask
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

            /// Releases the sole public ownership interest.
            void ReleaseOwner() noexcept {
                _value = static_cast<std::uint8_t>(
                    _value & static_cast<std::uint8_t>(~OwnerMask)
                );
            }

    };


    static_assert(
        sizeof(TaskControl) == 1U,
        "TaskControl must remain a one-byte intrinsic Task control representation"
    );


    template<class TTaskRecord>
    struct TaskPayloadOperations final {

        // Payload lifecycle.

        /// Executes the callable and publishes its result into the record payload.
        void (*Invoke)(TTaskRecord&);

        /// Destroys the currently live callable/result payload.
        void (*Destroy)(TTaskRecord&) noexcept;

        /// Moves a completed result into caller-provided typed storage.
        void (*MoveResult)(
            TTaskRecord&,
            void*
        );

    };


    template<std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TRecordCapacity>
    struct TaskRecord final {

        static_assert(
            TCallableCapacity > 0U,
            "Task callable capacity must be positive"
        );

        static constexpr std::size_t PayloadCapacity =
            TCallableCapacity > TResultCapacity
                ? TCallableCapacity
                : TResultCapacity;

        using Index = typename SmallestIndex<TRecordCapacity>::Type;

        // Reusable callable/result payload.

        /// Storage reused first by the callable and then by its result.
        alignas(std::max_align_t) std::byte Payload[PayloadCapacity];


        // Transient queue linkage.

        /// Next record index while this record is queued.
        Index QueueNext = SmallestIndex<TRecordCapacity>::Invalid;


        // Compact intrinsic control.

        /// One-byte Task lifecycle/ownership/Phase representation.
        TaskControl Control;


        // Type-erased payload lifecycle.

        /// One pointer to immutable Type-specific payload operations shared by all records of that callable/result pairing.
        const TaskPayloadOperations<TaskRecord>* PayloadOperations = nullptr;

    };

} // ESPressio::Threading::Detail
