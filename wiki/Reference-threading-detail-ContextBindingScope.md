# src/threading/detail/ContextBindingScope.hpp

**Classification:** PRIVATE IMPLEMENTATION. Consumes the existing ManagedContextRouter only. No SDK, Profiling or optional Platform binding header is included on the default path.

| Declaration | Purpose and invariant |
|---|---|
| UnselectedContextBinding / IsSelected | Compile-time absence, false; no retained fields |
| UnselectedContextBinding::Router<TCapacity,TSignalProvider> | Alias to the exact original router Type |
| HasContextBinding<TRouter> | Detects the explicit marker without imposing another alias on existing custom routers |
| ContextBindingScope<TRouter,false> | Empty default guard; templated constructor accepts the existing index Type without retaining it; IsBound always true |
| ContextBindingScope<TRouter,true> | Native-entry lifetime guard; constructor binds the direct slot, destructor retires only a successful own binding |
| _router / _index / _bound | Temporary selected stack ownership, not per-Worker retained storage; records exact retirement authority |
| deleted copy constructor/operator= | Prevent duplicate retirement ownership; user destructor and deleted copies also prevent implicit move |
| IsBound() | Predicate selecting eligible inner execution; failure returns from outer entry without retirement of a foreign record |

The selected guard relies on its owning wrapper to exclude entry from AST probes and to preserve policy/mapping lifetimes. Exception-enabled inner unwinding ends before guard retirement. Abort/reset does not imply unwinding. [Owning lifecycle](Context-Binding).
