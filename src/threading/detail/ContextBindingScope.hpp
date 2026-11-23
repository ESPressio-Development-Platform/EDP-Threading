#pragma once

#include <cstddef>

#include "ManagedContextRouter.hpp"

namespace ESPressio::Threading::Detail {

    /// Compile-time absence of current-context registration; retains the original router Type.
    struct UnselectedContextBinding final {

        /// Indicates that no context binding resources or operations are selected.
        static constexpr bool IsSelected = false;

        /// Resolves the original router without adding storage or a dependency on binding providers.
        /// @tparam TCapacity Number of managed execution contexts.
        /// @tparam TSignalProvider Existing targeted wake provider.
        template<std::size_t TCapacity, class TSignalProvider>
        using Router = ManagedContextRouter<TCapacity, TSignalProvider>;

    };


    /// Detects opt-in without imposing a new contract on existing test/application routers.
    /// @tparam TRouter Existing or explicitly binding-aware managed context router.
    template<class TRouter>
    inline constexpr bool HasContextBinding = []() constexpr {
        if constexpr (requires { TRouter::HasContextBinding; }) {
            return TRouter::HasContextBinding;
        } else {
            return false;
        }
    }();


    /// Empty default entry guard, eliminated along with all registration calls.
    /// @tparam TRouter Router selected by the owning topology.
    /// @tparam TSelected Whether the router explicitly selects context binding.
    template<class TRouter, bool TSelected = HasContextBinding<TRouter>>
    class ContextBindingScope final {

        public:

            /// Accepts existing topology coordinates without requiring an additional router alias.
            /// @tparam TIndex Existing resource-local context index Type.
            template<class TIndex>
            ContextBindingScope(
                TRouter&,
                TIndex
            ) noexcept {}

            /// Indicates that ordinary unselected execution may proceed.
            bool IsBound() const noexcept { return true; }

    };


    /// Binds at genuine native entry and retires after the complete persistent inner body returns.
    /// @tparam TRouter Explicitly selected binding-aware managed context router.
    template<class TRouter>
    class ContextBindingScope<TRouter, true> final {

        private:

            /// Address-stable router retaining the Bootstrap-owned binding policy.
            TRouter* _router;

            /// Direct topology slot belonging to this native incarnation.
            typename TRouter::ContextIndex _index;

            /// Records whether this scope owns a binding which it may retire.
            bool _bound;

        public:

            /// Performs one owner-qualified bind before entering any eligible inner work.
            ContextBindingScope(
                TRouter& router,
                typename TRouter::ContextIndex index
            ) noexcept :
                _router(&router),
                _index(index),
                _bound(router.EnterContext(index) == TRouter::BindingOutcome::Bound) {}

            /// Prevents copying ownership of native retirement.
            ContextBindingScope(const ContextBindingScope&) = delete;

            /// Prevents assigning ownership of native retirement.
            ContextBindingScope& operator =(const ContextBindingScope&) = delete;

            /// Retires only a successfully acquired expected record, after the final inner scope.
            ~ContextBindingScope() noexcept {
                if (_bound) {
                    _router->LeaveContext(_index);
                }
            }

            /// Indicates whether this entry owns the required binding.
            bool IsBound() const noexcept { return _bound; }

    };

} // ESPressio::Threading::Detail
