#pragma once

#include <cstddef>
#include <cstdint>

#include "TaskRecord.hpp"

namespace ESPressio::Threading::Detail {

    template<std::size_t TCapacity>
    class AvailabilityBitmap final {

        static_assert(
            TCapacity > 0U,
            "A bounded availability bitmap requires positive capacity"
        );

        private:

            // Bitmap storage.

            /// Number of availability bits stored in each byte.
            static constexpr std::size_t ByteBits = 8U;

            /// Number of bytes required to represent every bounded record.
            static constexpr std::size_t ByteCount =
                (TCapacity + ByteBits - 1U) / ByteBits;

            /// One bit per Task record; one means available.
            std::uint8_t _bytes[ByteCount] = {};


            // Internal helpers.

            /// Returns the mask of valid record bits in one bitmap byte.
            static constexpr std::uint8_t ValidMask(
                std::size_t byteIndex
            ) noexcept {
                const auto firstRecord = byteIndex * ByteBits;
                const auto remaining = TCapacity - firstRecord;

                if (remaining >= ByteBits) {
                    return 0xFFU;
                }

                return static_cast<std::uint8_t>(
                    (
                        static_cast<std::uint16_t>(1U) << remaining
                    ) - 1U
                );
            }

        public:

            // Construction.

            /// Marks every bounded Task record available.
            AvailabilityBitmap() noexcept {
                for (std::size_t byteIndex = 0U; byteIndex < ByteCount; ++byteIndex) {
                    _bytes[byteIndex] = ValidMask(
                        byteIndex
                    );
                }
            }


            // Availability operations.

            /// Attempts to claim the lowest-index available record.
            ///
            /// The owning Task facility must serialize this operation with all other bitmap
            /// and queue mutation. The bitmap deliberately carries no duplicated lock state.
            bool TryClaim(
                std::size_t& recordIndex
            ) noexcept {
                for (std::size_t byteIndex = 0U; byteIndex < ByteCount; ++byteIndex) {
                    auto byte = _bytes[byteIndex];

                    if (byte == 0U) { continue; }

                    std::size_t bitIndex = 0U;

                    while ((byte & 1U) == 0U) {
                        byte = static_cast<std::uint8_t>(
                            byte >> 1U
                        );
                        ++bitIndex;
                    }

                    const auto mask = static_cast<std::uint8_t>(
                        1U << bitIndex
                    );

                    _bytes[byteIndex] = static_cast<std::uint8_t>(
                        _bytes[byteIndex] &
                        static_cast<std::uint8_t>(~mask)
                    );

                    recordIndex = byteIndex * ByteBits + bitIndex;
                    return true;
                }

                return false;
            }

            /// Republishes one fully reclaimed record as available.
            ///
            /// The owning Task facility must serialize this operation with admission and reclamation.
            void Release(
                std::size_t recordIndex
            ) noexcept {
                const auto byteIndex = recordIndex / ByteBits;
                const auto bitIndex = recordIndex % ByteBits;

                _bytes[byteIndex] = static_cast<std::uint8_t>(
                    _bytes[byteIndex] |
                    static_cast<std::uint8_t>(
                        1U << bitIndex
                    )
                );
            }

            /// Indicates whether one record is currently published as available.
            bool IsAvailable(
                std::size_t recordIndex
            ) const noexcept {
                const auto byteIndex = recordIndex / ByteBits;
                const auto bitIndex = recordIndex % ByteBits;

                return (
                    _bytes[byteIndex] &
                    static_cast<std::uint8_t>(
                        1U << bitIndex
                    )
                ) != 0U;
            }

    };


    template<std::size_t TCapacity>
    class IntrusiveTaskQueue final {

        static_assert(
            TCapacity > 0U,
            "A bounded Task queue requires positive capacity"
        );

        public:

            // Queue index vocabulary.

            /// Smallest index Type satisfying the configured Task-record capacity.
            using Index = typename SmallestIndex<TCapacity>::Type;

            /// Sentinel which cannot identify a valid record.
            static constexpr Index InvalidIndex = SmallestIndex<TCapacity>::Invalid;

        private:

            // Queue endpoints.

            /// First queued record.
            Index _head = InvalidIndex;

            /// Last queued record.
            Index _tail = InvalidIndex;

        public:

            // Inspection.

            /// Indicates whether the queue contains no Task records.
            bool IsEmpty() const noexcept {
                return _head == InvalidIndex;
            }

            /// Returns the first queued record index or the invalid sentinel.
            Index Head() const noexcept {
                return _head;
            }


            // Mutation.

            /// Appends one record whose queue-link field is supplied by the caller.
            template<class TRecords>
            void Push(
                TRecords& records,
                Index recordIndex
            ) noexcept {
                records[recordIndex].QueueNext = InvalidIndex;

                if (_tail == InvalidIndex) {
                    _head = recordIndex;
                    _tail = recordIndex;
                    return;
                }

                records[_tail].QueueNext = recordIndex;
                _tail = recordIndex;
            }

            /// Removes and returns the first queued record or the invalid sentinel.
            template<class TRecords>
            Index Pop(
                TRecords& records
            ) noexcept {
                if (_head == InvalidIndex) {
                    return InvalidIndex;
                }

                const auto recordIndex = _head;
                _head = records[recordIndex].QueueNext;

                if (_head == InvalidIndex) {
                    _tail = InvalidIndex;
                }

                records[recordIndex].QueueNext = InvalidIndex;
                return recordIndex;
            }

            /// Removes one specific queued record while preserving FIFO order of all others.
            template<class TRecords>
            bool Remove(
                TRecords& records,
                Index recordIndex
            ) noexcept {
                Index previous = InvalidIndex;
                auto current = _head;

                while (current != InvalidIndex) {
                    if (current == recordIndex) {
                        const auto next = records[current].QueueNext;

                        if (previous == InvalidIndex) {
                            _head = next;
                        } else {
                            records[previous].QueueNext = next;
                        }

                        if (_tail == current) {
                            _tail = previous;
                        }

                        records[current].QueueNext = InvalidIndex;
                        return true;
                    }

                    previous = current;
                    current = records[current].QueueNext;
                }

                return false;
            }

    };

} // ESPressio::Threading::Detail
