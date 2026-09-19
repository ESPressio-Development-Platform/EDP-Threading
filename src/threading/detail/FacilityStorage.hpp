#pragma once

#include <cstddef>
#include <cstdint>

namespace ESPressio::Threading::Detail {

    template<std::size_t TCapacity>
    class AvailabilityBitmap final {

        private:

            // Bitmap storage.

            static constexpr std::size_t WordBits = 32U;
            static constexpr std::size_t WordCount = (TCapacity + WordBits - 1U) / WordBits;

            /// One bit per Task record; one means available.
            std::uint32_t _words[WordCount] = {};


            // Internal helpers.

            /// Returns the mask of valid record bits in one bitmap word.
            static constexpr std::uint32_t ValidMask(
                std::size_t wordIndex
            ) noexcept {
                const auto firstRecord = wordIndex * WordBits;
                const auto remaining = TCapacity - firstRecord;

                if (remaining >= WordBits) {
                    return 0xFFFFFFFFU;
                }

                return static_cast<std::uint32_t>(
                    (static_cast<std::uint64_t>(1U) << remaining) - 1U
                );
            }

        public:

            // Construction.

            /// Marks every bounded Task record available.
            AvailabilityBitmap() noexcept {
                for (std::size_t wordIndex = 0U; wordIndex < WordCount; ++wordIndex) {
                    _words[wordIndex] = ValidMask(
                        wordIndex
                    );
                }
            }


            // Availability operations.

            /// Attempts to claim the lowest-index available record.
            bool TryClaim(
                std::size_t& recordIndex
            ) noexcept {
                for (std::size_t wordIndex = 0U; wordIndex < WordCount; ++wordIndex) {
                    auto word = _words[wordIndex];

                    if (word == 0U) { continue; }

                    std::size_t bitIndex = 0U;

                    while ((word & 1U) == 0U) {
                        word >>= 1U;
                        ++bitIndex;
                    }

                    const auto mask = static_cast<std::uint32_t>(
                        1U << bitIndex
                    );

                    _words[wordIndex] &= static_cast<std::uint32_t>(~mask);
                    recordIndex = wordIndex * WordBits + bitIndex;

                    return true;
                }

                return false;
            }

            /// Republishes one fully reclaimed record as available.
            void Release(
                std::size_t recordIndex
            ) noexcept {
                const auto wordIndex = recordIndex / WordBits;
                const auto bitIndex = recordIndex % WordBits;

                _words[wordIndex] |= static_cast<std::uint32_t>(
                    1U << bitIndex
                );
            }

            /// Indicates whether one record is currently published as available.
            bool IsAvailable(
                std::size_t recordIndex
            ) const noexcept {
                const auto wordIndex = recordIndex / WordBits;
                const auto bitIndex = recordIndex % WordBits;

                return (
                    _words[wordIndex] &
                    static_cast<std::uint32_t>(1U << bitIndex)
                ) != 0U;
            }

    };


    template<std::size_t TCapacity, class TIndex>
    class IntrusiveTaskQueue final {

        private:

            // Queue endpoints.

            /// First queued record.
            TIndex _head;

            /// Last queued record.
            TIndex _tail;

            /// Sentinel outside the valid bounded record range.
            TIndex _invalid;

        public:

            // Construction.

            /// Creates an empty intrusive Task FIFO.
            explicit IntrusiveTaskQueue(
                TIndex invalid
            ) noexcept :
                _head(invalid),
                _tail(invalid),
                _invalid(invalid) {}


            // Inspection.

            /// Indicates whether the queue contains no Task records.
            bool IsEmpty() const noexcept {
                return _head == _invalid;
            }

            /// Returns the first queued record index or the invalid sentinel.
            TIndex Head() const noexcept {
                return _head;
            }


            // Mutation.

            /// Appends one record whose queue-link field is supplied by the caller.
            template<class TRecords>
            void Push(
                TRecords& records,
                TIndex recordIndex
            ) noexcept {
                records[recordIndex].QueueNext = _invalid;

                if (_tail == _invalid) {
                    _head = recordIndex;
                    _tail = recordIndex;

                    return;
                }

                records[_tail].QueueNext = recordIndex;
                _tail = recordIndex;
            }

            /// Removes and returns the first queued record or the invalid sentinel.
            template<class TRecords>
            TIndex Pop(
                TRecords& records
            ) noexcept {
                if (_head == _invalid) {
                    return _invalid;
                }

                const auto recordIndex = _head;
                _head = records[recordIndex].QueueNext;

                if (_head == _invalid) {
                    _tail = _invalid;
                }

                records[recordIndex].QueueNext = _invalid;

                return recordIndex;
            }

            /// Removes one specific queued record while preserving FIFO order of all others.
            template<class TRecords>
            bool Remove(
                TRecords& records,
                TIndex recordIndex
            ) noexcept {
                TIndex previous = _invalid;
                auto current = _head;

                while (current != _invalid) {
                    if (current == recordIndex) {
                        const auto next = records[current].QueueNext;

                        if (previous == _invalid) {
                            _head = next;
                        } else {
                            records[previous].QueueNext = next;
                        }

                        if (_tail == current) {
                            _tail = previous;
                        }

                        records[current].QueueNext = _invalid;

                        return true;
                    }

                    previous = current;
                    current = records[current].QueueNext;
                }

                return false;
            }

    };

} // ESPressio::Threading::Detail
