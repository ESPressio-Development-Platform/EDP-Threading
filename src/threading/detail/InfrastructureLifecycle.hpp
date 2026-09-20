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

            void PublishState(
                InfrastructureState state
            ) noexcept {
                _stateLock.Acquire();

                _state = static_cast<std::uint8_t>(
                    state
                );

                static_cast<void>(
                    _stateLock.Release()
                );
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
            static void WakeStarted(
                TTuple& resources,
                std::size_t startedCount
            ) noexcept {
                if constexpr (
                    TIndex < std::tuple_size_v<TTuple>
                ) {
                    if (TIndex < startedCount) {
                        std::get<TIndex>(
                            resources
                        ).RequestInfrastructureTermination();
                    }

                    WakeStarted<TIndex + 1U>(
                        resources,
                        startedCount
                    );
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

            InfrastructureLifecycle() noexcept = default;


            // State observation.

            InfrastructureState State() const noexcept {
                return ReadState();
            }

            bool CanActivate() const noexcept {
                return State() == InfrastructureState::Started;
            }

            bool ShouldTerminate() const noexcept {
                const auto state = State();

                return state == InfrastructureState::StartRollback ||
                    state == InfrastructureState::ShuttingDown ||
                    state == InfrastructureState::ShutdownComplete;
            }

            static bool CanActivateThunk(
                const void* context
            ) noexcept {
                return static_cast<const InfrastructureLifecycle*>(
                    context
                )->CanActivate();
            }

            static bool ShouldTerminateThunk(
                const void* context
            ) noexcept {
                return static_cast<const InfrastructureLifecycle*>(
                    context
                )->ShouldTerminate();
            }


            // Bootstrap phase publication.

            ThreadingInitializationResult CommitInitialization() noexcept {
                const auto state = State();

                if (state != InfrastructureState::Uninitialized) {
                    return ThreadingInitializationResult::AlreadyInitialized;
                }

                PublishState(
                    InfrastructureState::Initialized
                );

                return ThreadingInitializationResult::Succeeded;
            }


            // Transactional infrastructure Start.

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
                    PublishState(
                        InfrastructureState::StartRollback
                    );

                    WakeStarted<0U>(
                        resourceTuple,
                        startedCount
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

                PublishState(
                    InfrastructureState::Started
                );

                return ThreadingStartResult::Succeeded;
            }


            // Shutdown initiation.

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

                PublishState(
                    InfrastructureState::ShuttingDown
                );

                return ThreadingShutdownResult::Accepted;
            }

            void PublishShutdownComplete() noexcept {
                PublishState(
                    InfrastructureState::ShutdownComplete
                );
            }


    };

} // ESPressio::Threading::Detail
