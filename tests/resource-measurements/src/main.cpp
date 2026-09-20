#include <cstddef>
#include <cstdint>
#include <new>
#include <tuple>
#include <utility>

#include <ESPressio_Platform_FreeRTOS.hpp>
#include <ESPressio_Platform_ESP_IDF.hpp>
#include <ESPressio_Threading.hpp>

namespace Measurement {

    namespace Threading = ESPressio::Threading;
    namespace Detail = ESPressio::Threading::Detail;

    /// Targeted-wake provider Type used by every measured runtime.
    using SignalProvider =
        ESPressio::Platform::FreeRTOS::Synchronization::SignalProvider;

#ifdef ARDUINO
    /// Concrete execution-context provider Type selected by the current framework surface.
    using ExecutionContextProvider =
        ESPressio::Platform::FreeRTOS::Execution::ExecutionContextProvider;
#else
    /// Concrete execution-context provider Type selected by the current framework surface.
    using ExecutionContextProvider =
        ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;
#endif

    /// Topology lifecycle SpinLock provider Type used by every measured runtime.
    using SpinLockProvider =
        ESPressio::Platform::ESPIDF::Synchronization::SpinLockProvider;

    /// Resource-local Mutex provider Type used by every measured runtime.
    using MutexProvider =
        ESPressio::Platform::FreeRTOS::Synchronization::MutexProvider;


#if EDP_THREADING_MEASUREMENT_SCENARIO != 0

    struct PrimaryPool final {};
    struct SecondaryPool final {};
    struct ControlPool final {};

    struct LeaseTask final {

        /// Returns one deterministic value from the Dedicated Worker measurement callable.
        int operator ()() const noexcept {
            return 7;
        }

    };

    struct ThreadA final {};
    struct ThreadB final {};

    struct ThreadBody final {

        /// Runs the Dedicated Thread measurement callable without additional application work.
        void operator ()(
            Threading::ThreadContext&
        ) noexcept {}

    };


    /// Sums the provider-rounded physical stack backing required by a set of managed contexts.
    /// @tparam TStackCapacities Semantic stack capacities requested by the measured contexts.
    template<std::size_t... TStackCapacities>
    constexpr std::size_t StackBackingTotal() noexcept {
        return (
            Detail::ExecutionContextBacking<
                ExecutionContextProvider,
                TStackCapacities
            >::PhysicalStackCapacity() +
            ... +
            0U
        );
    }


#if EDP_THREADING_MEASUREMENT_SCENARIO == 1

    struct Scenario final {

        /// Compile-time Threading topology measured by this scenario.
        using Topology = Threading::ThreadingTopology<
            Threading::TaskExecutionFacility<
                PrimaryPool,
                Threading::TaskRecordCapacity<8U>,
                Threading::CallableCapacity<64U>,
                Threading::ResultCapacity<32U>,
                Threading::Workers<
                    Threading::Worker<
                        Threading::StackCapacity<4096U>,
                        Threading::Priority<Threading::ThreadPriority::Normal>,
                        Threading::AnyAffinity
                    >
                >
            >
        >;

        /// Dedicated Thread binding tuple Type required by this scenario.
        using Bindings = std::tuple<>;

        /// Creates the Dedicated Thread binding tuple required to construct this scenario.
        static Bindings MakeBindings() {
            return {};
        }

        /// Number of Mutex-provider objects included in this scenario's synchronization accounting.
        static constexpr std::size_t MutexCount = 2U;

        /// Returns the provider-rounded physical stack backing for every context in this scenario.
        static constexpr std::size_t StackBackingBytes() noexcept {
            return StackBackingTotal<4096U>();
        }

        /// References the representative Threading operations retained for this measurement scenario.
        /// @tparam TRuntime Concrete static Threading runtime Type exercised by the measurement scenario.
        template<class TRuntime>
        static void LinkSurface(
            TRuntime& runtime
        ) {
            auto dispatch = runtime.template Dispatch<PrimaryPool>(
                []() noexcept {
                    return 1;
                },
                Threading::TaskDispatchPolicy::AbandonImmediately
            );

            if (dispatch.IsSucceeded()) {
                auto task = dispatch.TakeTask();
                static_cast<void>(
                    task.State()
                );
                static_cast<void>(
                    task.Cancel()
                );
            }
        }

    };

#elif EDP_THREADING_MEASUREMENT_SCENARIO == 2

    struct Scenario final {

        /// Compile-time Threading topology measured by this scenario.
        using Topology = Threading::ThreadingTopology<
            Threading::TaskExecutionFacility<
                PrimaryPool,
                Threading::TaskRecordCapacity<16U>,
                Threading::CallableCapacity<64U>,
                Threading::ResultCapacity<32U>,
                Threading::Workers<
                    Threading::Worker<
                        Threading::StackCapacity<4096U>,
                        Threading::Priority<Threading::ThreadPriority::Normal>,
                        Threading::AnyAffinity
                    >,
                    Threading::Worker<
                        Threading::StackCapacity<4096U>,
                        Threading::Priority<Threading::ThreadPriority::Normal>,
                        Threading::AnyAffinity
                    >
                >
            >
        >;

        /// Dedicated Thread binding tuple Type required by this scenario.
        using Bindings = std::tuple<>;

        /// Creates the Dedicated Thread binding tuple required to construct this scenario.
        static Bindings MakeBindings() {
            return {};
        }

        /// Number of Mutex-provider objects included in this scenario's synchronization accounting.
        static constexpr std::size_t MutexCount = 2U;

        /// Returns the provider-rounded physical stack backing for every context in this scenario.
        static constexpr std::size_t StackBackingBytes() noexcept {
            return StackBackingTotal<4096U, 4096U>();
        }

        /// References the representative Threading operations retained for this measurement scenario.
        /// @tparam TRuntime Concrete static Threading runtime Type exercised by the measurement scenario.
        template<class TRuntime>
        static void LinkSurface(
            TRuntime& runtime
        ) {
            auto dispatch = runtime.template Dispatch<PrimaryPool>(
                []() noexcept {
                    return 2;
                },
                Threading::TaskDispatchPolicy::Queue
            );

            if (dispatch.IsSucceeded()) {
                auto task = dispatch.TakeTask();
                static_cast<void>(
                    task.Wait()
                );
            }
        }

    };

#elif EDP_THREADING_MEASUREMENT_SCENARIO == 3

    struct Scenario final {

        /// Compile-time Threading topology measured by this scenario.
        using Topology = Threading::ThreadingTopology<
            Threading::TaskExecutionFacility<
                ControlPool,
                Threading::TaskRecordCapacity<4U>,
                Threading::CallableCapacity<32U>,
                Threading::ResultCapacity<16U>,
                Threading::Workers<
                    Threading::Worker<
                        Threading::StackCapacity<2048U>,
                        Threading::Priority<Threading::ThreadPriority::Critical>,
                        Threading::AnyAffinity
                    >
                >
            >
        >;

        /// Dedicated Thread binding tuple Type required by this scenario.
        using Bindings = std::tuple<>;

        /// Creates the Dedicated Thread binding tuple required to construct this scenario.
        static Bindings MakeBindings() {
            return {};
        }

        /// Number of Mutex-provider objects included in this scenario's synchronization accounting.
        static constexpr std::size_t MutexCount = 2U;

        /// Returns the provider-rounded physical stack backing for every context in this scenario.
        static constexpr std::size_t StackBackingBytes() noexcept {
            return StackBackingTotal<2048U>();
        }

        /// References the representative Threading operations retained for this measurement scenario.
        /// @tparam TRuntime Concrete static Threading runtime Type exercised by the measurement scenario.
        template<class TRuntime>
        static void LinkSurface(
            TRuntime& runtime
        ) {
            auto dispatch = runtime.template Dispatch<ControlPool>(
                []() noexcept {
                    return 3;
                },
                Threading::TaskDispatchPolicy::QueueWithTimeout,
                Threading::Duration::FromNanoseconds(
                    10000000LL
                )
            );

            if (dispatch.IsSucceeded()) {
                auto task = dispatch.TakeTask();
                static_cast<void>(
                    task.WaitFor(
                        Threading::Duration::FromNanoseconds(
                            10000000LL
                        )
                    )
                );
            }
        }

    };

#elif EDP_THREADING_MEASUREMENT_SCENARIO == 4

    struct Scenario final {

        /// Compile-time Threading topology measured by this scenario.
        using Topology = Threading::ThreadingTopology<
            Threading::DedicatedWorkerLease<
                LeaseTask,
                Threading::TaskRecordCapacity<3U>,
                Threading::CallableCapacity<32U>,
                Threading::ResultCapacity<16U>,
                Threading::StackCapacity<2048U>,
                Threading::Priority<Threading::ThreadPriority::Critical>,
                Threading::AnyAffinity
            >
        >;

        /// Dedicated Thread binding tuple Type required by this scenario.
        using Bindings = std::tuple<>;

        /// Creates the Dedicated Thread binding tuple required to construct this scenario.
        static Bindings MakeBindings() {
            return {};
        }

        /// Number of Mutex-provider objects included in this scenario's synchronization accounting.
        static constexpr std::size_t MutexCount = 2U;

        /// Returns the provider-rounded physical stack backing for every context in this scenario.
        static constexpr std::size_t StackBackingBytes() noexcept {
            return StackBackingTotal<2048U>();
        }

        /// References the representative Threading operations retained for this measurement scenario.
        /// @tparam TRuntime Concrete static Threading runtime Type exercised by the measurement scenario.
        template<class TRuntime>
        static void LinkSurface(
            TRuntime& runtime
        ) {
            auto dispatch = runtime.template DispatchDedicated<LeaseTask>(
                LeaseTask{},
                Threading::TaskDispatchPolicy::Queue
            );

            if (dispatch.IsSucceeded()) {
                auto task = dispatch.TakeTask();
                static_cast<void>(
                    task.State()
                );
            }
        }

    };

#elif EDP_THREADING_MEASUREMENT_SCENARIO == 5

    struct Scenario final {

        /// Compile-time Threading topology measured by this scenario.
        using Topology = Threading::ThreadingTopology<
            Threading::DedicatedThread<
                ThreadA,
                Threading::StackCapacity<4096U>,
                Threading::Priority<Threading::ThreadPriority::High>,
                Threading::AnyAffinity
            >
        >;

        /// Concrete Dedicated Thread binding Type used by this scenario.
        using Binding = decltype(
            Threading::BindDedicatedThread<ThreadA>(
                ThreadBody{}
            )
        );

        /// Dedicated Thread binding tuple Type required by this scenario.
        using Bindings = std::tuple<Binding>;

        /// Creates the Dedicated Thread binding tuple required to construct this scenario.
        static Bindings MakeBindings() {
            return std::make_tuple(
                Threading::BindDedicatedThread<ThreadA>(
                    ThreadBody{}
                )
            );
        }

        /// Number of Mutex-provider objects included in this scenario's synchronization accounting.
        static constexpr std::size_t MutexCount = 2U;

        /// Returns the provider-rounded physical stack backing for every context in this scenario.
        static constexpr std::size_t StackBackingBytes() noexcept {
            return StackBackingTotal<4096U>();
        }

        /// References the representative Threading operations retained for this measurement scenario.
        /// @tparam TRuntime Concrete static Threading runtime Type exercised by the measurement scenario.
        template<class TRuntime>
        static void LinkSurface(
            TRuntime& runtime
        ) {
            auto thread = runtime.template ThreadHandle<ThreadA>();

            static_cast<void>(
                thread.Start()
            );
            static_cast<void>(
                thread.RequestStop()
            );
            static_cast<void>(
                thread.Join()
            );
        }

    };

#elif EDP_THREADING_MEASUREMENT_SCENARIO == 6

    struct Scenario final {

        /// Compile-time Threading topology measured by this scenario.
        using Topology = Threading::ThreadingTopology<
            Threading::TaskExecutionFacility<
                PrimaryPool,
                Threading::TaskRecordCapacity<8U>,
                Threading::CallableCapacity<64U>,
                Threading::ResultCapacity<32U>,
                Threading::Workers<
                    Threading::Worker<
                        Threading::StackCapacity<4096U>,
                        Threading::Priority<Threading::ThreadPriority::Normal>,
                        Threading::AnyAffinity
                    >
                >
            >,
            Threading::TaskExecutionFacility<
                SecondaryPool,
                Threading::TaskRecordCapacity<4U>,
                Threading::CallableCapacity<32U>,
                Threading::ResultCapacity<16U>,
                Threading::Workers<
                    Threading::Worker<
                        Threading::StackCapacity<2048U>,
                        Threading::Priority<Threading::ThreadPriority::High>,
                        Threading::AnyAffinity
                    >
                >
            >,
            Threading::DedicatedWorkerLease<
                LeaseTask,
                Threading::TaskRecordCapacity<3U>,
                Threading::CallableCapacity<32U>,
                Threading::ResultCapacity<16U>,
                Threading::StackCapacity<2048U>,
                Threading::Priority<Threading::ThreadPriority::Critical>,
                Threading::AnyAffinity
            >,
            Threading::DedicatedThread<
                ThreadA,
                Threading::StackCapacity<4096U>,
                Threading::Priority<Threading::ThreadPriority::High>,
                Threading::AnyAffinity
            >,
            Threading::DedicatedThread<
                ThreadB,
                Threading::StackCapacity<2048U>,
                Threading::Priority<Threading::ThreadPriority::Low>,
                Threading::AnyAffinity
            >
        >;

        /// Concrete Dedicated Thread binding Type used by this scenario.
        using BindingA = decltype(
            Threading::BindDedicatedThread<ThreadA>(
                ThreadBody{}
            )
        );

        /// Concrete Dedicated Thread binding Type used by this scenario.
        using BindingB = decltype(
            Threading::BindDedicatedThread<ThreadB>(
                ThreadBody{}
            )
        );

        /// Dedicated Thread binding tuple Type required by this scenario.
        using Bindings = std::tuple<
            BindingA,
            BindingB
        >;

        /// Creates the Dedicated Thread binding tuple required to construct this scenario.
        static Bindings MakeBindings() {
            return std::make_tuple(
                Threading::BindDedicatedThread<ThreadA>(
                    ThreadBody{}
                ),
                Threading::BindDedicatedThread<ThreadB>(
                    ThreadBody{}
                )
            );
        }

        /// Number of Mutex-provider objects included in this scenario's synchronization accounting.
        static constexpr std::size_t MutexCount = 6U;

        /// Returns the provider-rounded physical stack backing for every context in this scenario.
        static constexpr std::size_t StackBackingBytes() noexcept {
            return StackBackingTotal<
                4096U,
                2048U,
                2048U,
                4096U,
                2048U
            >();
        }

        /// References the representative Threading operations retained for this measurement scenario.
        /// @tparam TRuntime Concrete static Threading runtime Type exercised by the measurement scenario.
        template<class TRuntime>
        static void LinkSurface(
            TRuntime& runtime
        ) {
            auto primaryDispatch = runtime.template Dispatch<PrimaryPool>(
                []() noexcept {
                    return 6;
                },
                Threading::TaskDispatchPolicy::Queue
            );

            auto secondaryDispatch = runtime.template Dispatch<SecondaryPool>(
                []() noexcept {
                    return 7;
                },
                Threading::TaskDispatchPolicy::Queue
            );

            static_cast<void>(
                runtime.template DispatchDedicated<LeaseTask>(
                    LeaseTask{},
                    Threading::TaskDispatchPolicy::Queue
                )
            );

            if (
                primaryDispatch.IsSucceeded() &&
                secondaryDispatch.IsSucceeded()
            ) {
                auto primaryTask = primaryDispatch.TakeTask();
                auto secondaryTask = secondaryDispatch.TakeTask();

                static_cast<void>(
                    primaryTask.Wait()
                );

                static_cast<void>(
                    secondaryTask.Wait()
                );
            }

            auto threadA = runtime.template ThreadHandle<ThreadA>();
            auto threadB = runtime.template ThreadHandle<ThreadB>();

            static_cast<void>(
                threadA.Start()
            );
            static_cast<void>(
                threadB.Start()
            );
            static_cast<void>(
                threadA.RequestStop()
            );
            static_cast<void>(
                threadB.RequestStop()
            );
        }

    };

#else
#error EDP_THREADING_MEASUREMENT_SCENARIO must be between 0 and 6
#endif


    /// Concrete static Threading runtime Type compiled for the selected measurement scenario.
    using Runtime = Threading::StaticThreadingRuntime<
        Scenario::Topology,
        Scenario::Bindings,
        SignalProvider,
        ExecutionContextProvider,
        SpinLockProvider,
        MutexProvider
    >;

    /// Number of managed execution contexts owned by the selected scenario topology.
    static constexpr std::size_t ContextCount =
        Scenario::Topology::ManagedExecutionContextCount;

    /// Execution-context backing helper used to query provider control-storage requirements.
    using RepresentativeBacking = Detail::ExecutionContextBacking<
        ExecutionContextProvider,
        1U
    >;

    /// Total bytes occupied by concrete execution-context provider objects.
    static constexpr std::size_t ExecutionProviderBytes =
        ContextCount *
        sizeof(
            ExecutionContextProvider
        );

    /// Total caller-owned native execution-control backing bytes.
    static constexpr std::size_t ControlBackingBytes =
        ContextCount *
        RepresentativeBacking::PhysicalControlCapacity();

    /// Total provider-rounded physical stack backing bytes.
    static constexpr std::size_t StackBackingBytes =
        Scenario::StackBackingBytes();

    /// Total targeted-wake, Mutex, and lifecycle SpinLock provider-object bytes.
    static constexpr std::size_t SynchronizationWakeBytes =
        ContextCount *
            sizeof(
                SignalProvider
            ) +
        Scenario::MutexCount *
            sizeof(
                MutexProvider
            ) +
        sizeof(
            SpinLockProvider
        );

    /// Exact target-compiled size of the complete static Threading runtime.
    static constexpr std::size_t RuntimeBytes =
        sizeof(
            Runtime
        );

    static_assert(
        RuntimeBytes >=
            ExecutionProviderBytes +
            ControlBackingBytes +
            StackBackingBytes +
            SynchronizationWakeBytes,
        "Resource measurement category accounting exceeds complete Runtime size"
    );

    /// Runtime bytes remaining after subtracting the separately reported provider/backing categories.
    static constexpr std::size_t ThreadingIntrinsicBytes =
        RuntimeBytes -
        ExecutionProviderBytes -
        ControlBackingBytes -
        StackBackingBytes -
        SynchronizationWakeBytes;

    static_assert(
        ThreadingIntrinsicBytes > 0U,
        "Representative Threading runtime must retain positive intrinsic storage"
    );


    extern "C" {

        __attribute__((used))
        /// Retained BSS symbol sized to the calculated Threading-intrinsic byte count.
        std::byte EDP_Threading_Measurement_Intrinsic[
            ThreadingIntrinsicBytes
        ];

        __attribute__((used))
        /// Retained BSS symbol sized to the execution-provider object byte count.
        std::byte EDP_Threading_Measurement_ProviderObjects[
            ExecutionProviderBytes
        ];

        __attribute__((used))
        /// Retained BSS symbol sized to the native execution-control backing byte count.
        std::byte EDP_Threading_Measurement_ControlBacking[
            ControlBackingBytes
        ];

        __attribute__((used))
        /// Retained BSS symbol sized to the provider-rounded stack backing byte count.
        std::byte EDP_Threading_Measurement_StackBacking[
            StackBackingBytes
        ];

        __attribute__((used))
        /// Retained BSS symbol sized to the synchronization/wake provider byte count.
        std::byte EDP_Threading_Measurement_SynchronizationWake[
            SynchronizationWakeBytes
        ];

        /// Accumulator used solely to create observable references to retained measurement symbols.
        volatile std::uintptr_t EDP_Threading_Measurement_Keep = 0U;

    }


    /// Placement-constructs the selected runtime so its constructor remains linked into the measurement image.
    Runtime* ConstructRuntime(
        void* storage
    ) {
        return new (
            storage
        ) Runtime(
            Scenario::MakeBindings()
        );
    }


    /// References representative lifecycle and execution APIs so scenario code-size remains linked for measurement.
    void LinkRuntimeSurface(
        Runtime& runtime
    ) {
        static_cast<void>(
            runtime.Initialize()
        );

        static_cast<void>(
            runtime.Start()
        );

        Scenario::LinkSurface(
            runtime
        );

        static_cast<void>(
            runtime.BeginShutdown()
        );

        static_cast<void>(
            runtime.IsExecutionQuiescent()
        );

        static_cast<void>(
            runtime.FinalizeShutdown()
        );

        static_cast<void>(
            runtime.WaitForShutdown()
        );
    }


    /// Function-pointer Type retaining the scenario runtime constructor surface.
    using ConstructFunction = Runtime* (*)(
        void*
    );

    /// Function-pointer Type retaining the representative scenario API surface.
    using LinkFunction = void (*)(
        Runtime&
    );

    __attribute__((used))
    /// Retained constructor reference preventing link-time removal of runtime construction code.
    volatile ConstructFunction KeepConstructor =
        &ConstructRuntime;

    __attribute__((used))
    /// Retained API-surface reference preventing link-time removal of measured Threading code.
    volatile LinkFunction KeepLinkedSurface =
        &LinkRuntimeSurface;


    /// Creates observable references to measurement symbols so the linker retains their target-compiled sizes.
    void KeepMeasurementSymbols() noexcept {
        EDP_Threading_Measurement_Keep ^=
            reinterpret_cast<std::uintptr_t>(
                EDP_Threading_Measurement_Intrinsic
            );

        EDP_Threading_Measurement_Keep ^=
            reinterpret_cast<std::uintptr_t>(
                EDP_Threading_Measurement_ProviderObjects
            );

        EDP_Threading_Measurement_Keep ^=
            reinterpret_cast<std::uintptr_t>(
                EDP_Threading_Measurement_ControlBacking
            );

        EDP_Threading_Measurement_Keep ^=
            reinterpret_cast<std::uintptr_t>(
                EDP_Threading_Measurement_StackBacking
            );

        EDP_Threading_Measurement_Keep ^=
            reinterpret_cast<std::uintptr_t>(
                EDP_Threading_Measurement_SynchronizationWake
            );

        if (
            KeepConstructor == nullptr ||
            KeepLinkedSurface == nullptr
        ) {
            EDP_Threading_Measurement_Keep ^= 1U;
        }
    }

#else

    void KeepMeasurementSymbols() noexcept {}

#endif

} // Measurement


#ifdef ARDUINO


/// Retains measurement symbols from the Arduino framework entrypoint without executing Threading work.
void setup() {
    Measurement::KeepMeasurementSymbols();
}

/// Leaves the build-only Arduino measurement application idle.
void loop() {}

#else

/// Retains measurement symbols from the ESP-IDF application entrypoint without executing Threading work.
extern "C" void app_main() {
    Measurement::KeepMeasurementSymbols();
}

#endif
