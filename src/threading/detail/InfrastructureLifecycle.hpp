#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>

#include "../ThreadingTypes.hpp"

namespace ESPressio::Threading::Detail {

    enum class InfrastructureState : std::uint8_t {
        Uninitialized = 0,
        Initialized = 1,
        Started = 2,
        StartRollback = 3,
        ShuttingDown = 4,
        ShutdownComplete = 5
    };


    /// Defines the compile-time contract for `InfrastructureLifecycle`.
    /// @tparam TSpinLockProvider Concrete Platform SpinLock provider Type protecting lifecycle publication.
    template<class TSpinLockProvider>
    class InfrastructureLifecycle final {

        static_assert(
            sizeof(
                ESPressio::Platform::Synchronization::Detail::SpinLockProviderTraits<
                    TSpinLockProvider
                >
            ) > 0U,
            "Threading infrastructure lifecycle requires a conforming Platform SpinLock provider"
        );

        private:

            // Authoritative global lifecycle state.

            /// One-byte application-wide Threading lifecycle state.
            std::uint8_t _state =
                static_cast<std::uint8_t>(
                    InfrastructureState::Uninitialized
                );

            /// Short-duration topology-wide synchronization protecting lifecycle publication.
            mutable TSpinLockProvider _stateLock;


            // Lifecycle state synchronization.

            /// Reads the authoritative lifecycle state under the topology-wide SpinLock.
            InfrastructureState ReadState() const noexcept {
                _stateLock.Acquire();

                const auto state = static_cast<InfrastructureState>(
                    _state
                );

                static_cast<void>(
                    _stateLock.Release()
                );

                return state;
            }

            /// Publishes one authoritative lifecycle transition under the topology-wide SpinLock.
            ESPressio::Platform::Synchronization::SpinLockReleaseResult PublishState(
                InfrastructureState state
            ) noexcept {
                _stateLock.Acquire();

                _state = static_cast<std::uint8_t>(
                    state
                );

                return _stateLock.Release();
            }


            // Transactional Start helpers.

            /// Defines the compile-time contract for `StartNext`.
            /// @tparam TTuple Tuple Type containing the resources traversed by this helper.
            /// @tparam TIndex Compile-time tuple/resource index used by the recursive traversal.
            template<std::size_t TIndex, class TTuple>
            static ESPressio::Platform::Execution::ExecutionStartResult StartNext(
                TTuple& resources,
                std::size_t& startedCount
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return ESPressio::Platform::Execution::ExecutionStartResult::Succeeded;
                } else {
                    auto& resource = std::get<TIndex>(
                        resources
                    );

                    const auto result = resource.StartInfrastructure();

                    if (
                        result !=
                        ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
                    ) {
                        return result;
                    }

                    ++startedCount;

                    return StartNext<TIndex + 1U>(
                        resources,
                        startedCount
                    );
                }
            }

            /// Defines the compile-time contract for `WakeStarted`.
            /// @tparam TTuple Tuple Type containing the resources traversed by this helper.
            /// @tparam TIndex Compile-time tuple/resource index used by the recursive traversal.
            template<std::size_t TIndex, class TTuple>
            static ESPressio::Platform::Synchronization::SignalNotifyResult WakeStarted(
                TTuple& resources,
                std::size_t startedCount
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled;
                } else {
                    ESPressio::Platform::Synchronization::SignalNotifyResult result =
                        ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled;

                    if (TIndex < startedCount) {
                        result = std::get<TIndex>(
                            resources
                        ).RequestInfrastructureTermination();
                    }

                    const auto tailResult = WakeStarted<TIndex + 1U>(
                        resources,
                        startedCount
                    );

                    return result != ESPressio::Platform::Synchronization::SignalNotifyResult::Signaled
                        ? result
                        : tailResult;
                }
            }

            /// Defines the compile-time contract for `JoinStarted`.
            /// @tparam TTuple Tuple Type containing the resources traversed by this helper.
            /// @tparam TIndex Compile-time tuple/resource index used by the recursive traversal.
            template<std::size_t TIndex, class TTuple>
            static ESPressio::Platform::Execution::ExecutionJoinResult JoinStarted(
                TTuple& resources,
                std::size_t startedCount
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded;
                } else {
                    ESPressio::Platform::Execution::ExecutionJoinResult result =
                        ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded;

                    if (TIndex < startedCount) {
                        result = std::get<TIndex>(
                            resources
                        ).JoinInfrastructure(
                            ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                        );
                    }

                    const auto tailResult = JoinStarted<TIndex + 1U>(
                        resources,
                        startedCount
                    );

                    return result != ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded
                        ? result
                        : tailResult;
                }
            }

            /// Defines the compile-time contract for `DestroyAll`.
            /// @tparam TTuple Tuple Type containing the resources traversed by this helper.
            /// @tparam TIndex Compile-time tuple/resource index used by the recursive traversal.
            template<std::size_t TIndex, class TTuple>
            static ESPressio::Platform::Execution::ExecutionDestroyResult DestroyAll(
                TTuple& resources
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;
                } else {
                    const auto result = std::get<TIndex>(
                        resources
                    ).DestroyInfrastructure();

                    const auto tailResult = DestroyAll<TIndex + 1U>(
                        resources
                    );

                    return result != ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded
                        ? result
                        : tailResult;
                }
            }

        public:

            /// Creates the lifecycle in its Uninitialized state.
            InfrastructureLifecycle() noexcept = default;


            // State observation.

            /// Returns the synchronized application-wide Threading infrastructure state.
            InfrastructureState State() const noexcept {
                return ReadState();
            }

            /// Indicates whether semantic Task/Thread activation is currently permitted.
            bool CanActivate() const noexcept {
                return State() == InfrastructureState::Started;
            }

            /// Indicates whether persistent managed contexts must cooperatively terminate.
            bool ShouldTerminate() const noexcept {
                const auto state = State();

                return state == InfrastructureState::StartRollback ||
                    state == InfrastructureState::ShuttingDown ||
                    state == InfrastructureState::ShutdownComplete;
            }

            /// Type-erased lifecycle predicate used by statically owned resources before semantic activation.
            static bool CanActivateThunk(
                const void* context
            ) noexcept {
                return static_cast<const InfrastructureLifecycle*>(
                    context
                )->CanActivate();
            }

            /// Type-erased lifecycle predicate used by managed contexts to observe rollback or shutdown.
            static bool ShouldTerminateThunk(
                const void* context
            ) noexcept {
                return static_cast<const InfrastructureLifecycle*>(
                    context
                )->ShouldTerminate();
            }


            // Bootstrap phase publication.

            /// Commits the global initialization barrier after every topology resource initialized successfully.
            ThreadingInitializationResult CommitInitialization() noexcept {
                const auto state = State();

                if (state != InfrastructureState::Uninitialized) {
                    return ThreadingInitializationResult::AlreadyInitialized;
                }

                return PublishState(
                    InfrastructureState::Initialized
                ) == ESPressio::Platform::Synchronization::SpinLockReleaseResult::Released
                    ? ThreadingInitializationResult::Succeeded
                    : ThreadingInitializationResult::ProviderFailure;
            }


            // Transactional infrastructure Start.

            /// Starts all supplied topology resources transactionally in argument order.
            /// @tparam TResources Concrete topology-owned resource Types participating in infrastructure Start.
            template<class... TResources>
            ThreadingStartResult Start(
                TResources&... resources
            ) noexcept {
                const auto state = State();

                if (state == InfrastructureState::Started) {
                    return ThreadingStartResult::AlreadyStarted;
                }

                if (state != InfrastructureState::Initialized) {
                    return ThreadingStartResult::NotInitialized;
                }

                auto resourceTuple = std::forward_as_tuple(
                    resources...
                );

                std::size_t startedCount = 0U;

                if (
                    StartNext<0U>(
                        resourceTuple,
                        startedCount
                    ) != ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
                ) {
                    static_cast<void>(
                        PublishState(
                            InfrastructureState::StartRollback
                        )
                    );

                    static_cast<void>(
                        WakeStarted<0U>(
                            resourceTuple,
                            startedCount
                        )
                    );

                    const auto joinResult = JoinStarted<0U>(
                        resourceTuple,
                        startedCount
                    );

                    const auto destroyResult = DestroyAll<0U>(
                        resourceTuple
                    );

                    static_cast<void>(
                        joinResult
                    );

                    static_cast<void>(
                        destroyResult
                    );

                    return ThreadingStartResult::ProviderFailure;
                }

                return PublishState(
                    InfrastructureState::Started
                ) == ESPressio::Platform::Synchronization::SpinLockReleaseResult::Released
                    ? ThreadingStartResult::Succeeded
                    : ThreadingStartResult::ProviderFailure;
            }


            // Shutdown initiation.

            /// Begins the terminal application-wide shutdown transition without blocking for execution completion.
            ThreadingShutdownResult BeginShutdown() noexcept {
                const auto state = State();

                if (state == InfrastructureState::ShutdownComplete) {
                    return ThreadingShutdownResult::AlreadyCompleted;
                }

                if (state == InfrastructureState::ShuttingDown) {
                    return ThreadingShutdownResult::AlreadyShuttingDown;
                }

                if (state != InfrastructureState::Started) {
                    return ThreadingShutdownResult::NotStarted;
                }

                // The ordinary SpinLock Release contract has no provider-failure outcome; the
                // interrupt-context-only rejection cannot occur on this application-context path.
                static_cast<void>(
                    PublishState(
                        InfrastructureState::ShuttingDown
                    )
                );

                return ThreadingShutdownResult::Accepted;
            }

            /// Publishes the terminal ShutdownComplete state after all managed contexts are destroyed.
            ESPressio::Platform::Synchronization::SpinLockReleaseResult PublishShutdownComplete() noexcept {
                return PublishState(
                    InfrastructureState::ShutdownComplete
                );
            }


    };

} // ESPressio::Threading::Detail
