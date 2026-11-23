#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

#include <ESPressio_Platform_CurrentContextBinding.hpp>

#include "detail/ContextBindingScope.hpp"
#include "detail/MonotonicWaitBudget.hpp"
#include "ThreadingTypes.hpp"

namespace ESPressio::Threading {

    /// Published registration/retirement fact for one native incarnation.
    enum class ContextBindingPhase : std::uint8_t {
        Pending = 0U,
        Bound = 1U,
        AlreadyBound = 2U,
        UnsupportedContext = 3U,
        Retired = 4U,
        RetirementNotBound = 5U,
        RetirementDifferentBinding = 6U,
        RetirementUnsupportedContext = 7U
    };


    /// Result of the selected startup acknowledgement barrier.
    enum class ContextBindingStartupResult : std::uint8_t {
        NotAttempted = 0U,
        Ready = 1U,
        InvalidMapping = 2U,
        BindingRejected = 3U,
        TimedOut = 4U,
        ProviderFailure = 5U
    };


    /// Native ownership fact, independent of registration and export/durability.
    enum class ContextBindingNativeState : std::uint8_t {
        Uninitialized = 0U,
        Initialized = 1U,
        Started = 2U,
        Joined = 3U,
        Destroyed = 4U
    };


    /// Owner-supplied immutable session identity and address-stable record for one direct topology slot.
    /// @tparam TRecord Record Type borrowed by the selected Platform binding provider.
    template<class TRecord>
    struct ContextBindingRegistration final {

        /// Nonzero identity unique throughout the complete application profiling session.
        std::uint64_t Identity;

        /// Address-stable record; Bootstrap retains it beyond native/capture/export retirement.
        const TRecord* Record;

        /// Borrows an existing record without copying it or extending its lifetime.
        ContextBindingRegistration(
            std::uint64_t identity,
            const TRecord& record
        ) noexcept :
            Identity(identity),
            Record(std::addressof(record)) {}

        /// Prevents borrowing a temporary record.
        ContextBindingRegistration(
            std::uint64_t,
            TRecord&&
        ) = delete;

        /// Prevents borrowing a const temporary record.
        ContextBindingRegistration(
            std::uint64_t,
            const TRecord&&
        ) = delete;

    };


    /// Couples observed native ownership and precise binding/retirement outcomes.
    struct ContextBindingSnapshot final {

        /// Last published registration/retirement fact.
        ContextBindingPhase Phase;

        /// Independently observed native resource lifecycle.
        ContextBindingNativeState NativeState;

        /// Indicates that the native wrapper caught an exceptional inner-body exit.
        bool HasAbnormalExit;

    };


    namespace Detail {

        /// Adds one typed non-owning binding-policy reference only to explicitly selected routers.
        /// @tparam TCapacity Direct topology context capacity.
        /// @tparam TSignalProvider Existing targeted wake provider.
        /// @tparam TPolicy Address-stable Bootstrap-owned selected binding policy.
        template<std::size_t TCapacity, class TSignalProvider, class TPolicy>
        class BindingManagedContextRouter final {

            private:

                /// Existing router, composed without weakening its final ownership boundary.
                ManagedContextRouter<TCapacity, TSignalProvider> _base;

                /// Borrowed policy outliving this router and all native entries.
                TPolicy* _binding;

            public:

                /// Existing router Type, retaining its structural resolver and wake mechanisms.
                using Base = ManagedContextRouter<TCapacity, TSignalProvider>;

                /// Existing targeted wake-set Type.
                using WakeSet = typename Base::WakeSet;

                /// Existing compact direct topology context index.
                using ContextIndex = typename Base::ContextIndex;

                /// Exact topology managed context capacity.
                static constexpr std::size_t ContextCapacity = TCapacity;

                /// Exact Platform binding operation result.
                using BindingOutcome = ESPressio::Platform::Execution::CurrentContextBindResult;

                /// Indicates that native entry must register and retire its direct slot.
                static constexpr bool HasContextBinding = true;

                /// Binds existing wake storage and the explicit owner policy.
                BindingManagedContextRouter(
                    typename Base::WakeSet& wakeSet,
                    TPolicy& policy
                ) noexcept :
                    _base(wakeSet),
                    _binding(&policy) {
                    static_assert(TCapacity == TPolicy::ContextCapacity);
                }

                /// Binds the original structural resolver after complete topology construction.
                /// @tparam TArguments Existing structural resolver binding argument Types.
                template<class... TArguments>
                void BindTopology(TArguments&&... arguments) noexcept {
                    _base.BindTopology(std::forward<TArguments>(arguments)...);
                }

                /// Reports whether the original structural resolver is bound.
                bool IsTopologyBound() const noexcept { return _base.IsTopologyBound(); }

                /// Preserves original structural context inspection outside method probes.
                std::optional<ContextIndex> CurrentContextIndex() const noexcept { return _base.CurrentContextIndex(); }

                /// Preserves the existing authoritative interruption predicate.
                bool IsInterrupted(ContextIndex index) const noexcept { return _base.IsInterrupted(index); }

                /// Routes the existing targeted wake without adding a second wait mechanism.
                ESPressio::Platform::Synchronization::SignalNotifyResult Wake(ContextIndex index) noexcept { return _base.Wake(index); }

                /// Waits through the existing targeted wake owner.
                ESPressio::Platform::Synchronization::SignalWaitResult Wait(
                    ContextIndex index,
                    ESPressio::Platform::Synchronization::WaitTimeout timeout
                ) noexcept {
                    return _base.Wait(
                        index,
                        timeout
                    );
                }

                /// Returns the one selected owner policy; this is not a provider search.
                TPolicy& BindingPolicy() noexcept { return *_binding; }

                /// Registers the exact record assigned to this direct topology slot.
                BindingOutcome EnterContext(typename Base::ContextIndex index) noexcept {
                    return _binding->Enter(index);
                }

                /// Retires only the record this native entry successfully bound.
                void LeaveContext(typename Base::ContextIndex index) noexcept {
                    _binding->Leave(index);
                }

        };

    } // ESPressio::Threading::Detail


    /// Compile-time opt-in to owner-qualified registration, startup acknowledgement and retirement.
    /// Not a capture gateway, identity allocator, scheduler or export/durability completion authority.
    /// @tparam TBindingProvider Qualified Platform CurrentContextBinding provider.
    /// @tparam TCapacity Exact topology managed execution-context capacity.
    /// @tparam TSignalProvider Existing Platform Signal provider for the startup acknowledgement wake.
    /// @tparam TSpinLockProvider Existing Platform SpinLock provider for fixed control-plane facts.
    template<class TBindingProvider, std::size_t TCapacity, class TSignalProvider, class TSpinLockProvider>
    class ContextBindingPolicy final {

        private:

            /// Validates the exact selected provider and its typed Composition contract.
            using BindingTraits = ESPressio::Platform::Execution::Detail::CurrentContextBindingProviderTraits<TBindingProvider>;

            /// Validates the existing synchronization owner; never invents a private lock.
            using LockTraits = ESPressio::Platform::Synchronization::Detail::SpinLockProviderTraits<TSpinLockProvider>;

            static_assert(sizeof(BindingTraits) > 0U && sizeof(LockTraits) > 0U);

            // Owner bindings and bounded control-plane state.

            /// Immutable Bootstrap mapping, ordered by the topology's existing direct context index.
            const std::array<ContextBindingRegistration<typename BindingTraits::ContextRecord>, TCapacity>* _mapping;

            /// One selected domain-neutral carrier, shared only through current-context operations.
            TBindingProvider _provider;

            /// Protects ordinary setup/retirement facts; probes never acquire this lock.
            mutable TSpinLockProvider _lock;

            /// One latched startup acknowledgement wake, not one semaphore per context.
            TSignalProvider _acknowledged;

            /// One byte per context: low nibble binding phase, three native-state bits and one abnormal-exit bit.
            std::array<std::uint8_t, TCapacity> _facts{};

            /// Startup result retained separately from per-context native completion.
            ContextBindingStartupResult _startup = ContextBindingStartupResult::NotAttempted;

            /// Prevents resetting facts or reusing this owner for another native incarnation.
            bool _prepared = false;


            // Synchronized fact publication.

            /// Publishes a binding phase without discarding the concurrent native lifecycle fact.
            void PublishPhase(
                std::size_t index,
                ContextBindingPhase phase
            ) noexcept {
                _lock.Acquire();
                _facts[index] = static_cast<std::uint8_t>(
                    (_facts[index] & 0xF0U) | static_cast<std::uint8_t>(phase)
                );
                static_cast<void>(_lock.Release());
                static_cast<void>(_acknowledged.Notify());
            }

            /// Publishes native ownership without overwriting registration or retirement evidence.
            void PublishNative(
                std::size_t index,
                ContextBindingNativeState state
            ) noexcept {
                _lock.Acquire();
                _facts[index] = static_cast<std::uint8_t>(
                    (_facts[index] & 0x8FU) | (static_cast<std::uint8_t>(state) << 4U)
                );
                static_cast<void>(_lock.Release());
            }

            /// Takes one bounded setup scan under the fact lock; never used by a probe.
            ContextBindingStartupResult ReadBarrier() const noexcept {
                _lock.Acquire();
                auto result = ContextBindingStartupResult::Ready;

                for (const auto facts : _facts) {
                    const auto phase = static_cast<ContextBindingPhase>(facts & 0x0FU);
                    if (phase == ContextBindingPhase::Pending) {
                        if (result == ContextBindingStartupResult::Ready) {
                            result = ContextBindingStartupResult::NotAttempted;
                        }
                    } else if (phase != ContextBindingPhase::Bound || (facts & 0x80U) != 0U) {
                        result = ContextBindingStartupResult::BindingRejected;
                    }
                }

                static_cast<void>(_lock.Release());
                return result;
            }

        public:

            // Public ownership vocabulary.

            /// Record Type borrowed by the selected Platform carrier.
            using ContextRecord = typename BindingTraits::ContextRecord;

            /// Exact mapping Type owned and frozen by application Bootstrap.
            using Mapping = std::array<ContextBindingRegistration<ContextRecord>, TCapacity>;

            /// Number of native execution contexts represented by this policy.
            static constexpr std::size_t ContextCapacity = TCapacity;

            /// Indicates the compile-time selection of registration and bounded startup.
            static constexpr bool IsSelected = true;

            /// Resolves the selected router without changing any resource-local template contract.
            /// @tparam TCount Topology's independently computed managed context capacity.
            /// @tparam TWakeProvider Existing targeted wake provider.
            template<std::size_t TCount, class TWakeProvider>
            using Router = Detail::BindingManagedContextRouter<TCount, TWakeProvider, ContextBindingPolicy>;


            // Construction and lifetime.

            /// Borrows a mapping whose records/identities are frozen before initialization.
            explicit ContextBindingPolicy(const Mapping& mapping) noexcept : _mapping(&mapping) {}

            /// Prevents borrowing temporary mapping storage.
            ContextBindingPolicy(Mapping&&) = delete;

            /// Prevents borrowing const temporary mapping storage.
            ContextBindingPolicy(const Mapping&&) = delete;

            /// Prevents copying native lifetime evidence or synchronization ownership.
            ContextBindingPolicy(const ContextBindingPolicy&) = delete;

            /// Prevents assigning native lifetime evidence or synchronization ownership.
            ContextBindingPolicy& operator =(const ContextBindingPolicy&) = delete;


            // Ordinary Bootstrap preparation and acknowledgement.

            /// Validates nonzero unique session identities and record addresses, once before native entry.
            ContextBindingStartupResult Prepare() noexcept {
                if (_prepared) { return ContextBindingStartupResult::InvalidMapping; }
                _prepared = true;

                for (std::size_t index = 0U; index < TCapacity; ++index) {
                    const auto& registration = (*_mapping)[index];
                    if (registration.Identity == 0U || registration.Record == nullptr) {
                        return _startup = ContextBindingStartupResult::InvalidMapping;
                    }
                    for (std::size_t prior = 0U; prior < index; ++prior) {
                        if ((*_mapping)[prior].Identity == registration.Identity || (*_mapping)[prior].Record == registration.Record) {
                            return _startup = ContextBindingStartupResult::InvalidMapping;
                        }
                    }
                }

                return ContextBindingStartupResult::Ready;
            }

            /// Records a native Start failure independently of the per-context binding facts.
            void RecordNativeStartFailure() noexcept { _startup = ContextBindingStartupResult::ProviderFailure; }

            /// Waits for every required Bound acknowledgement under the one caller-created Clock budget.
            ContextBindingStartupResult Await(Detail::MonotonicWaitBudget& budget) noexcept {
                if (budget.Mode() == Detail::MonotonicWaitMode::Forever) {
                    return _startup = ContextBindingStartupResult::ProviderFailure;
                }
                for (;;) {
                    const auto remaining = budget.Remaining();
                    if (remaining.IsNoWait()) { return _startup = ContextBindingStartupResult::TimedOut; }
                    const auto result = ReadBarrier();
                    if (result != ContextBindingStartupResult::NotAttempted) { return _startup = result; }
                    const auto wait = _acknowledged.Wait(remaining);
                    if (wait == ESPressio::Platform::Synchronization::SignalWaitResult::TimedOut) {
                        return _startup = ContextBindingStartupResult::TimedOut;
                    }
                    if (wait != ESPressio::Platform::Synchronization::SignalWaitResult::Signaled) {
                        return _startup = ContextBindingStartupResult::ProviderFailure;
                    }
                }
            }

            /// Returns the setup result; only the serialized Bootstrap/control owner mutates or reads it.
            ContextBindingStartupResult StartupResult() const noexcept { return _startup; }


            // Actual native entry and retirement; excluded from method instrumentation.

            /// Binds exactly one directly addressed record; a foreign binding is never replaced.
            ESPressio::Platform::Execution::CurrentContextBindResult Enter(std::size_t index) noexcept {
                const auto result = _provider.BindCurrent(*(*_mapping)[index].Record);
                const auto phase = result == ESPressio::Platform::Execution::CurrentContextBindResult::Bound
                    ? ContextBindingPhase::Bound
                    : result == ESPressio::Platform::Execution::CurrentContextBindResult::AlreadyBound
                        ? ContextBindingPhase::AlreadyBound
                        : ContextBindingPhase::UnsupportedContext;
                PublishPhase(
                    index,
                    phase
                );
                return result;
            }

            /// Retires only this owner's expected record, retaining exact anomalous outcomes.
            void Leave(std::size_t index) noexcept {
                const auto result = _provider.RetireCurrent(*(*_mapping)[index].Record);
                const auto phase = result == ESPressio::Platform::Execution::CurrentContextRetireResult::Retired
                    ? ContextBindingPhase::Retired
                    : result == ESPressio::Platform::Execution::CurrentContextRetireResult::NotBound
                        ? ContextBindingPhase::RetirementNotBound
                        : result == ESPressio::Platform::Execution::CurrentContextRetireResult::DifferentBinding
                            ? ContextBindingPhase::RetirementDifferentBinding
                            : ContextBindingPhase::RetirementUnsupportedContext;
                PublishPhase(
                    index,
                    phase
                );
            }


            // Native ownership facts, distinct from registration and export completion.

            /// Latches an exceptional native-body exit without clearing semantic activity or retirement facts.
            void MarkAbnormalExit(std::size_t index) noexcept {
                _lock.Acquire();
                _facts[index] |= 0x80U;
                static_cast<void>(_lock.Release());
                static_cast<void>(_acknowledged.Notify());
            }

            /// Records successful initialization without implying native entry or registration.
            void MarkInitialized(std::size_t index) noexcept {
                PublishNative(
                    index,
                    ContextBindingNativeState::Initialized
                );
            }

            /// Records successful Start without overwriting an acknowledgement already published by entry.
            void MarkStarted(std::size_t index) noexcept {
                PublishNative(
                    index,
                    ContextBindingNativeState::Started
                );
            }

            /// Returns a coherent fixed-slot snapshot or no payload for an invalid index.
            std::optional<ContextBindingSnapshot> Snapshot(std::size_t index) const noexcept {
                if (index >= TCapacity) { return std::nullopt; }
                _lock.Acquire();
                const auto facts = _facts[index];
                static_cast<void>(_lock.Release());
                return ContextBindingSnapshot{
                    static_cast<ContextBindingPhase>(facts & 0x0FU),
                    static_cast<ContextBindingNativeState>((facts >> 4U) & 0x07U),
                    (facts & 0x80U) != 0U
                };
            }

            /// Continues native teardown without resetting the caller's finite deadline or erasing facts.
            /// @tparam TExecutionProvider Existing native execution-context provider owning the addressed task.
            template<class TExecutionProvider>
            ThreadingFinalizationResult Cleanup(
                std::size_t index,
                TExecutionProvider& provider,
                Detail::MonotonicWaitBudget& budget
            ) noexcept {
                auto snapshot = *Snapshot(index);
                if (snapshot.NativeState == ContextBindingNativeState::Destroyed || snapshot.NativeState == ContextBindingNativeState::Uninitialized) {
                    return ThreadingFinalizationResult::Completed;
                }
                if (snapshot.NativeState == ContextBindingNativeState::Started) {
                    const auto joined = provider.Join(budget.Remaining());
                    if (joined != ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded) {
                        return joined == ESPressio::Platform::Execution::ExecutionJoinResult::TimedOut
                            ? ThreadingFinalizationResult::ExecutionNotQuiescent
                            : ThreadingFinalizationResult::ProviderFailure;
                    }
                    PublishNative(
                        index,
                        ContextBindingNativeState::Joined
                    );
                    snapshot = *Snapshot(index);
                }
                if (snapshot.HasAbnormalExit) { return ThreadingFinalizationResult::ProviderFailure; }
                if (snapshot.NativeState == ContextBindingNativeState::Joined &&
                    snapshot.Phase != ContextBindingPhase::Retired &&
                    snapshot.Phase != ContextBindingPhase::AlreadyBound &&
                    snapshot.Phase != ContextBindingPhase::UnsupportedContext) {
                    // A join cannot manufacture missing retirement evidence or clear a foreign record.
                    return ThreadingFinalizationResult::ProviderFailure;
                }
                if (provider.Destroy() != ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded) {
                    return ThreadingFinalizationResult::ProviderFailure;
                }
                PublishNative(
                    index,
                    ContextBindingNativeState::Destroyed
                );
                return ThreadingFinalizationResult::Completed;
            }

    };

} // ESPressio::Threading
