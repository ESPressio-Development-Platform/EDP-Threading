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


    enum class TaskInvocationOutcome : std::uint8_t {
        Completed = 0,
        Cancelled = 1
    };


    template<std::size_t TCapacity>
    struct SmallestIndex {

        static_assert(
            TCapacity > 0U,
            "A bounded index requires a positive capacity"
        );

        /// Smallest unsigned index type able to represent every record plus an invalid sentinel.
        using Type = std::conditional_t<
            (TCapacity < static_cast<std::size_t>(std::numeric_limits<std::uint8_t>::max())),
            std::uint8_t,
            std::conditional_t<
                (TCapacity < static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max())),
                std::uint16_t,
                std::uint32_t
            >
        >;

        /// Sentinel value which cannot identify a valid record for the configured capacity.
        static constexpr Type Invalid = std::numeric_limits<Type>::max();

    };


    template<class TAtomicWord8Provider>
    class TaskControl final {

        private:

            // Packed control byte.

            /// Packed operational state, public ownership and incarnation Phase.
            typename TAtomicWord8Provider::Word _value;

            /// Bit mask selecting the internal Task operational state.
            static constexpr std::uint8_t StateMask = 0x07U;

            /// Bit mask selecting the sole public Task ownership interest.
            static constexpr std::uint8_t OwnerMask = 0x08U;

            /// Bit mask selecting the record-incarnation Phase.
            static constexpr std::uint8_t PhaseMask = 0x10U;

        public:

            // State inspection.

            /// Returns the internal operational state.
            TaskOperationalState State() const noexcept {
                return static_cast<TaskOperationalState>(
                    _value.LoadAcquire() & StateMask
                );
            }

            /// Indicates whether the sole public Task ownership interest exists.
            bool HasOwner() const noexcept {
                return (_value.LoadAcquire() & OwnerMask) != 0U;
            }

            /// Returns the current one-bit record-incarnation Phase.
            bool Phase() const noexcept {
                return (_value.LoadAcquire() & PhaseMask) != 0U;
            }

            /// Indicates whether cooperative cancellation has been requested while Running.
            bool IsCancellationRequested() const noexcept {
                return State() == TaskOperationalState::RunningCancelRequested;
            }


            // State mutation.

            /// Initializes a newly claimed record and toggles its incarnation Phase.
            void InitializeQueued() noexcept {
                const auto current = _value.LoadRelaxed();
                const auto nextPhase = static_cast<std::uint8_t>(
                    (current ^ PhaseMask) & PhaseMask
                );

                _value.StoreRelease(
                    static_cast<std::uint8_t>(
                        nextPhase |
                        OwnerMask |
                        static_cast<std::uint8_t>(TaskOperationalState::Queued)
                    )
                );
            }

            /// Replaces only the operational state while preserving owner and Phase bits.
            void SetState(
                TaskOperationalState state
            ) noexcept {
                auto expected = _value.LoadAcquire();

                for (;;) {
                    const auto desired = static_cast<std::uint8_t>(
                        (expected & static_cast<std::uint8_t>(~StateMask)) |
                        static_cast<std::uint8_t>(state)
                    );

                    if (_value.CompareExchangeAcqRel(
                        expected,
                        desired
                    )) { return; }
                }
            }

            /// Atomically changes the operational state only when the expected state still owns the transition.
            bool CompareExchangeState(
                TaskOperationalState expectedState,
                TaskOperationalState desiredState
            ) noexcept {
                auto expected = _value.LoadAcquire();

                for (;;) {
                    if (
                        static_cast<TaskOperationalState>(
                            expected & StateMask
                        ) != expectedState
                    ) {
                        return false;
                    }

                    const auto desired = static_cast<std::uint8_t>(
                        (expected & static_cast<std::uint8_t>(~StateMask)) |
                        static_cast<std::uint8_t>(desiredState)
                    );

                    if (_value.CompareExchangeAcqRel(
                        expected,
                        desired
                    )) {
                        return true;
                    }
                }
            }

            /// Releases the sole public ownership interest.
            void ReleaseOwner() noexcept {
                auto expected = _value.LoadAcquire();

                for (;;) {
                    const auto desired = static_cast<std::uint8_t>(
                        expected & static_cast<std::uint8_t>(~OwnerMask)
                    );

                    if (_value.CompareExchangeAcqRel(
                        expected,
                        desired
                    )) { return; }
                }
            }

    };


    template<class TTaskRecord>
    struct TaskPayloadOperations final {

        // Payload lifecycle.

        /// Executes the callable and establishes any result payload without publishing terminal lifecycle state.
        TaskInvocationOutcome (*Invoke)(TTaskRecord&);

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


    template<std::size_t TCallableCapacity, std::size_t TResultCapacity, std::size_t TRecordCapacity, class TAtomicWord8Provider>
    struct TaskRecord final {

        static_assert(
            TCallableCapacity > 0U,
            "Task callable capacity must be positive"
        );

        static_assert(
            sizeof(TaskControl<TAtomicWord8Provider>) == 1U,
            "TaskControl must remain a one-byte intrinsic Task control representation"
        );

        /// Payload bytes shared by callable storage and result storage.
        static constexpr std::size_t PayloadCapacity =
            TCallableCapacity > TResultCapacity
                ? TCallableCapacity
                : TResultCapacity;

        /// Smallest record-index type satisfying the configured record capacity.
        using Index = typename SmallestIndex<TRecordCapacity>::Type;

        // Reusable callable/result payload.

        /// Storage reused first by the callable and then by its result.
        alignas(std::max_align_t) std::byte Payload[PayloadCapacity];


        // Transient queue linkage.

        /// Next record index while this record is queued.
        Index QueueNext = SmallestIndex<TRecordCapacity>::Invalid;


        // Compact intrinsic control.

        /// One-byte Task lifecycle/ownership/Phase representation.
        TaskControl<TAtomicWord8Provider> Control;


        // Type-erased payload lifecycle.

        /// One pointer to immutable Type-specific payload operations shared by all records of that callable/result pairing.
        const TaskPayloadOperations<TaskRecord>* PayloadOperations = nullptr;

    };

} // ESPressio::Threading::Detail
