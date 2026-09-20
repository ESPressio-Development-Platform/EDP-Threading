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

            template<std::size_t TIndex, class TTuple>
            static bool StartNext(
                TTuple& resources,
                std::size_t& startedCount
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return true;
                } else {
                    auto& resource = std::get<TIndex>(
                        resources
                    );

                    if (
                        resource.StartInfrastructure() !=
                        ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
                    ) {
                        return false;
                    }

                    ++startedCount;

                    return StartNext<TIndex + 1U>(
                        resources,
                        startedCount
                    );
                }
            }

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

            template<std::size_t TIndex, class TTuple>
            static bool JoinStarted(
                TTuple& resources,
                std::size_t startedCount
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return true;
                } else {
                    bool joined = true;

                    if (TIndex < startedCount) {
                        joined =
                            std::get<TIndex>(
                                resources
                            ).JoinInfrastructure(
                                ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                            ) ==
                            ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded;
                    }

                    return JoinStarted<TIndex + 1U>(
                        resources,
                        startedCount
                    ) &&
                        joined;
                }
            }

            template<std::size_t TIndex, class TTuple>
            static bool DestroyAll(
                TTuple& resources
            ) noexcept {
                if constexpr (
                    TIndex == std::tuple_size_v<TTuple>
                ) {
                    return true;
                } else {
                    const bool destroyed =
                        std::get<TIndex>(
                            resources
                        ).DestroyInfrastructure() ==
                        ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;

                    return DestroyAll<TIndex + 1U>(
                        resources
                    ) &&
                        destroyed;
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

                if (!StartNext<0U>(
                    resourceTuple,
                    startedCount
                )) {
                    PublishState(
                        InfrastructureState::StartRollback
                    );

                    WakeStarted<0U>(
                        resourceTuple,
                        startedCount
                    );

                    const bool joined = JoinStarted<0U>(
                        resourceTuple,
                        startedCount
                    );

                    const bool destroyed = DestroyAll<0U>(
                        resourceTuple
                    );

                    static_cast<void>(
                        joined
                    );

                    static_cast<void>(
                        destroyed
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
