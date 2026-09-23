#pragma once

#include <cstddef>
#include <cstdint>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Threading::Detail {

    enum class WorkerExecutionInitializationResult : std::uint8_t {
        Succeeded = 0,
        ProviderFailure = 1
    };


    /// Defines the compile-time contract for `ExecutionContextBacking`.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TStackCapacity Semantic stack-capacity value requested by the topology.
    template<class TExecutionContextProvider, std::size_t TStackCapacity>
    class ExecutionContextBacking final {

        static_assert(
            TStackCapacity > 0U,
            "Managed execution contexts require positive semantic stack capacity"
        );

        private:

            // Provider-declared backing requirements.

            /// Composition capability set exposed by the execution-context provider.
            using Capabilities = typename TExecutionContextProvider::CompositionOffers;
            /// Resolved capability-property set for the execution-context provider.
            using Properties = typename Capabilities::template PropertiesFor<
                ESPressio::Platform::Execution::ExecutionContext
            >;

            /// Caller-owned native execution-control backing bytes required by the provider.
            static constexpr std::size_t ControlBytes =
                Properties::template Value<
                    ESPressio::Platform::Execution::ControlStorageBytes
                >;

            /// Required alignment of caller-owned execution-control backing.
            static constexpr std::size_t ControlAlignment =
                Properties::template Value<
                    ESPressio::Platform::Execution::ControlStorageAlignment
                >;

            /// Required alignment of caller-owned execution stack backing.
            static constexpr std::size_t StackAlignment =
                Properties::template Value<
                    ESPressio::Platform::Execution::StackStorageAlignment
                >;

            /// Provider allocation granularity used when reserving stack bytes.
            static constexpr std::size_t StackGranularity =
                Properties::template Value<
                    ESPressio::Platform::Execution::StackAllocationGranularityBytes
                >;

            /// Physical stack reservation after rounding the semantic request to provider granularity.
            static constexpr std::size_t RoundedStackBytes =
                (
                    (
                        TStackCapacity +
                        StackGranularity -
                        1U
                    ) /
                    StackGranularity
                ) *
                StackGranularity;

            static_assert(
                ControlBytes > 0U,
                "ExecutionContext provider must require positive caller-supplied control storage"
            );

            static_assert(
                ControlAlignment > 0U &&
                StackAlignment > 0U &&
                StackGranularity > 0U,
                "ExecutionContext provider backing alignment/granularity must be positive"
            );


            // Caller-owned deterministic backing.

            /// Native provider control backing.
            alignas(ControlAlignment) std::byte _control[ControlBytes];

            /// Native execution stack backing rounded to provider allocation granularity.
            alignas(StackAlignment) std::byte _stack[RoundedStackBytes];

        public:

            // Resource inspection.

            /// Semantic minimum stack capacity requested by Threading topology.
            static constexpr std::size_t SemanticStackCapacity() noexcept {
                return TStackCapacity;
            }

            /// Physical caller-owned stack bytes reserved for the concrete provider.
            static constexpr std::size_t PhysicalStackCapacity() noexcept {
                return RoundedStackBytes;
            }

            /// Physical caller-owned native control bytes reserved for the concrete provider.
            static constexpr std::size_t PhysicalControlCapacity() noexcept {
                return ControlBytes;
            }


            // Platform storage view.

            /// Returns the caller-owned storage supplied during ExecutionContext initialization.
            ESPressio::Platform::Execution::ExecutionStorage Storage() noexcept {
                ESPressio::Platform::Execution::ExecutionStorage storage;
                storage.ControlAddress = _control;
                storage.ControlBytes = ControlBytes;
                storage.StackAddress = _stack;
                storage.StackBytes = RoundedStackBytes;
                return storage;
            }

    };


    /// Defines the compile-time contract for `TaskWorkerExecutionContext`.
    /// @tparam TExecutionContextProvider Concrete Platform execution-context provider Type used for managed execution.
    /// @tparam TTaskFacility Task facility Type serviced by the Worker context.
    /// @tparam TManagedContextRouter Managed-context router Type used for identity, interruption, and targeted wakes.
    /// @tparam TStackCapacity Semantic stack-capacity value requested by the topology.
    template<class TExecutionContextProvider, std::size_t TStackCapacity, class TTaskFacility, class TManagedContextRouter>
    class TaskWorkerExecutionContext final {

        private:

            // Provider contract.

            /// Validated Platform execution-context provider contract traits.
            using ProviderTraits =
                ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<
                    TExecutionContextProvider
                >;

            static_assert(
                sizeof(ProviderTraits) > 0U,
                "Task Worker requires a valid Platform ExecutionContext provider"
            );


            // Provider and deterministic backing.

            /// Concrete Platform execution-context provider.
            TExecutionContextProvider _provider;

            /// Caller-owned native control and stack backing.
            ExecutionContextBacking<
                TExecutionContextProvider,
                TStackCapacity
            > _backing;


            // Topology bindings.

            /// Non-owning Task facility served by this Worker.
            TTaskFacility* _facility;

            /// Non-owning managed-context router owning this Worker's targeted wake primitive.
            TManagedContextRouter* _router;

            /// Dense topology-wide execution-context index assigned to this Worker.
            typename TTaskFacility::ManagedContextIndex _contextIndex;

            /// Non-owning authoritative shutdown-state context.
            const void* _shutdownContext;

            /// Predicate reading authoritative terminal-shutdown intent.
            bool (*_isShutdownRequested)(const void*) noexcept;


            // Worker trampoline.

            /// Runs the persistent Worker loop after infrastructure Start.
            static void Entry(
                void* parameter
            ) noexcept {
                auto* self = static_cast<TaskWorkerExecutionContext*>(
                    parameter
                );

                if (self == nullptr) {
                    return;
                }

                static_cast<void>(
                    self->_facility->WorkerBecameAvailable(
                        self->_contextIndex
                    )
                );

                for (;;) {
                    if (
                        self->_isShutdownRequested(
                            self->_shutdownContext
                        )
                    ) {
                        return;
                    }

                    const auto binding = self->_facility->AssignedTaskForContext(
                        self->_contextIndex
                    );

                    if (binding.has_value()) {
                        const auto outcome = self->_facility->Invoke(
                            binding->RecordIndex,
                            binding->Phase
                        );

                        static_cast<void>(
                            self->_facility->CompleteWorkerTask(
                                self->_contextIndex,
                                binding->RecordIndex,
                                binding->Phase,
                                outcome
                            )
                        );

                        continue;
                    }

                    static_cast<void>(
                        self->_router->Wait(
                            self->_contextIndex,
                            ESPressio::Platform::Synchronization::WaitTimeout::Forever()
                        )
                    );
                }
            }

        public:

            /// Lightweight constructor arguments used to direct-construct a non-movable Worker inside static tuple storage.
            struct ConstructionArguments final {

                /// Task facility served by this Worker.
                TTaskFacility* Facility = nullptr;

                /// Managed-context router owning this Worker's targeted wake primitive.
                TManagedContextRouter* Router = nullptr;

                /// Dense topology-wide execution-context index assigned to this Worker.
                typename TTaskFacility::ManagedContextIndex ContextIndex{};

                /// Authoritative shutdown-state context.
                const void* ShutdownContext = nullptr;

                /// Predicate reading authoritative terminal-shutdown intent.
                bool (*IsShutdownRequested)(const void*) noexcept = nullptr;

            };


            // Construction.

            /// Direct-constructs one statically provisioned Worker from lightweight topology bindings.
            explicit TaskWorkerExecutionContext(
                ConstructionArguments arguments
            ) noexcept :
                _facility(arguments.Facility),
                _router(arguments.Router),
                _contextIndex(arguments.ContextIndex),
                _shutdownContext(arguments.ShutdownContext),
                _isShutdownRequested(arguments.IsShutdownRequested) {}

            /// Binds one statically provisioned Worker to its facility, wake router and shutdown predicate.
            TaskWorkerExecutionContext(
                TTaskFacility& facility,
                TManagedContextRouter& router,
                typename TTaskFacility::ManagedContextIndex contextIndex,
                const void* shutdownContext,
                bool (*isShutdownRequested)(const void*) noexcept
            ) noexcept :
                TaskWorkerExecutionContext(
                    ConstructionArguments {
                        &facility,
                        &router,
                        contextIndex,
                        shutdownContext,
                        isShutdownRequested
                    }
                ) {}


            // Infrastructure lifecycle.

            /// Initializes the Platform execution context without starting Worker execution.
            WorkerExecutionInitializationResult Initialize(
                ESPressio::Platform::Execution::ExecutionPriority priority,
                ESPressio::Platform::Execution::ProcessorAffinity affinity,
                const char* name = nullptr
            ) noexcept {
                ESPressio::Platform::Execution::ExecutionConfiguration configuration;
                configuration.Priority = priority;
                configuration.Affinity = affinity;
                configuration.Name = name;

                const auto result = _provider.Initialize(
                    _backing.Storage(),
                    configuration,
                    &Entry,
                    this
                );

                if (
                    result !=
                    ESPressio::Platform::Execution::ExecutionInitializationResult::Succeeded
                ) {
                    static_cast<void>(
                        _provider.Destroy()
                    );

                    return WorkerExecutionInitializationResult::ProviderFailure;
                }

                return WorkerExecutionInitializationResult::Succeeded;
            }

            /// Starts the already initialized persistent Worker trampoline.
            ESPressio::Platform::Execution::ExecutionStartResult StartInfrastructure() noexcept {
                return _provider.Start();
            }

            /// Compatibility spelling retained inside the implementation while Bootstrap is integrated.
            ESPressio::Platform::Execution::ExecutionStartResult Start() noexcept {
                return StartInfrastructure();
            }

            /// Wakes the persistent Worker so rollback/shutdown termination is re-evaluated.
            ESPressio::Platform::Synchronization::SignalNotifyResult RequestInfrastructureTermination() noexcept {
                return _router->Wake(
                    _contextIndex
                );
            }

            /// Joins the Worker after authoritative shutdown has caused its trampoline to return.
            ESPressio::Platform::Execution::ExecutionJoinResult Join(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                return _provider.Join(
                    timeout
                );
            }

            /// Destroys the joined native execution context.
            ESPressio::Platform::Execution::ExecutionDestroyResult Destroy() noexcept {
                return _provider.Destroy();
            }


            // Context identity.

            /// Indicates whether this Worker's Platform context is the current execution context.
            bool IsCurrentContext() const noexcept {
                return _provider.IsCurrentContext();
            }

            /// Indicates whether the current Worker execution should cooperatively return from a Threading-owned block.
            bool IsInterrupted() noexcept {
                return _isShutdownRequested(
                    _shutdownContext
                ) ||
                    _facility->IsCancellationRequestedForContext(
                        _contextIndex
                    );
            }


            // Resource inspection.

            /// Returns the concrete provider object size counted separately from caller backing.
            static constexpr std::size_t ProviderObjectBytes() noexcept {
                return sizeof(TExecutionContextProvider);
            }

            /// Returns caller-owned native control backing bytes.
            static constexpr std::size_t ControlBackingBytes() noexcept {
                return ExecutionContextBacking<
                    TExecutionContextProvider,
                    TStackCapacity
                >::PhysicalControlCapacity();
            }

            /// Returns caller-owned physical stack backing bytes.
            static constexpr std::size_t StackBackingBytes() noexcept {
                return ExecutionContextBacking<
                    TExecutionContextProvider,
                    TStackCapacity
                >::PhysicalStackCapacity();
            }

    };

} // ESPressio::Threading::Detail
