#pragma once

#include <cstddef>
#include <cstdint>

#include "TaskRecord.hpp"

namespace ESPressio::Threading::Detail {

    template<std::size_t TRecordCapacity, class TAtomicWord32Provider>
    class TaskAvailabilityBitmap final {

        static_assert(
            TRecordCapacity > 0U,
            "Task availability requires positive record capacity"
        );

        private:

            static constexpr std::size_t BitsPerWord = 32U;
            static constexpr std::size_t WordCount =
                (TRecordCapacity + BitsPerWord - 1U) / BitsPerWord;

            // Availability words.

            /// One bit per Task record; one means available for admission.
            typename TAtomicWord32Provider::Word _words[WordCount];


            // Bit helpers.

            /// Returns the valid-bit mask for one bitmap word.
            static constexpr std::uint32_t ValidMask(
                std::size_t wordIndex
            ) noexcept {
                const auto firstRecord = wordIndex * BitsPerWord;
                const auto remaining = TRecordCapacity - firstRecord;

                if (remaining >= BitsPerWord) {
                    return 0xFFFFFFFFU;
                }

                return static_cast<std::uint32_t>(
                    (static_cast<std::uint64_t>(1U) << remaining) - 1U
                );
            }

        public:

            // Construction.

            /// Creates an availability bitmap with every configured Task record free.
            TaskAvailabilityBitmap() noexcept {
                for (std::size_t wordIndex = 0U; wordIndex < WordCount; ++wordIndex) {
                    _words[wordIndex].StoreRelaxed(
                        ValidMask(wordIndex)
                    );
                }
            }


            // Allocation.

            /// Atomically claims the first available record index.
            bool TryClaim(
                std::size_t& recordIndex
            ) noexcept {
                for (std::size_t wordIndex = 0U; wordIndex < WordCount; ++wordIndex) {
                    auto expected = _words[wordIndex].LoadAcquire();

                    while (expected != 0U) {
                        const auto lowestBit = static_cast<std::uint32_t>(
                            expected & static_cast<std::uint32_t>(0U - expected)
                        );
                        const auto desired = static_cast<std::uint32_t>(
                            expected & ~lowestBit
                        );

                        if (_words[wordIndex].CompareExchangeAcqRel(
                            expected,
                            desired
                        )) {
                            std::size_t bitIndex = 0U;
                            auto bit = lowestBit;

                            while ((bit & 1U) == 0U) {
                                ++bitIndex;
                                bit >>= 1U;
                            }

                            recordIndex = wordIndex * BitsPerWord + bitIndex;
                            return true;
                        }
                    }
                }

                return false;
            }

            /// Republishes one fully reclaimed Task record as available.
            void Release(
                std::size_t recordIndex
            ) noexcept {
                const auto wordIndex = recordIndex / BitsPerWord;
                const auto bitIndex = recordIndex % BitsPerWord;
                const auto releaseBit = static_cast<std::uint32_t>(
                    1U << bitIndex
                );

                auto expected = _words[wordIndex].LoadAcquire();

                for (;;) {
                    const auto desired = static_cast<std::uint32_t>(
                        expected | releaseBit
                    );

                    if (_words[wordIndex].CompareExchangeAcqRel(
                        expected,
                        desired
                    )) { return; }
                }
            }

    };


    template<class TRecord, std::size_t TRecordCapacity>
    class TaskFifo final {

        public:

            using Index = typename SmallestIndex<TRecordCapacity>::Type;

        private:

            // Queue endpoints.

            /// First queued record or the invalid-index sentinel.
            Index _head = SmallestIndex<TRecordCapacity>::Invalid;

            /// Last queued record or the invalid-index sentinel.
            Index _tail = SmallestIndex<TRecordCapacity>::Invalid;

        public:

            // Queue inspection.

            /// Indicates whether no Task record is queued.
            bool IsEmpty() const noexcept {
                return _head == SmallestIndex<TRecordCapacity>::Invalid;
            }


            // Queue mutation.

            /// Appends one record to the FIFO tail.
            void Push(
                TRecord* records,
                Index index
            ) noexcept {
                records[index].QueueNext = SmallestIndex<TRecordCapacity>::Invalid;

                if (_tail == SmallestIndex<TRecordCapacity>::Invalid) {
                    _head = index;
                    _tail = index;
                    return;
                }

                records[_tail].QueueNext = index;
                _tail = index;
            }

            /// Removes and returns the FIFO head.
            Index Pop(
                TRecord* records
            ) noexcept {
                if (IsEmpty()) {
                    return SmallestIndex<TRecordCapacity>::Invalid;
                }

                const auto index = _head;
                _head = records[index].QueueNext;
                records[index].QueueNext = SmallestIndex<TRecordCapacity>::Invalid;

                if (_head == SmallestIndex<TRecordCapacity>::Invalid) {
                    _tail = SmallestIndex<TRecordCapacity>::Invalid;
                }

                return index;
            }

            /// Removes one queued record while preserving surviving FIFO order.
            bool Remove(
                TRecord* records,
                Index index
            ) noexcept {
                Index previous = SmallestIndex<TRecordCapacity>::Invalid;
                auto current = _head;

                while (current != SmallestIndex<TRecordCapacity>::Invalid) {
                    if (current == index) {
                        const auto next = records[current].QueueNext;

                        if (previous == SmallestIndex<TRecordCapacity>::Invalid) {
                            _head = next;
                        } else {
                            records[previous].QueueNext = next;
                        }

                        if (_tail == current) {
                            _tail = previous;
                        }

                        records[current].QueueNext = SmallestIndex<TRecordCapacity>::Invalid;
                        return true;
                    }

                    previous = current;
                    current = records[current].QueueNext;
                }

                return false;
            }

    };

} // ESPressio::Threading::Detail
