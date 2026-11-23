# src/ESPressio_Threading_ContextBinding.hpp

**Classification:** PUBLIC ENTRY POINT. Aggregates the optional typed binding policy and ordinary Threading public surface. It is deliberately separate from ESPressio_Threading.hpp so unselected consumers need neither the new Platform header nor any carrier instance. Adds no declarations or retained state. Direct includes: threading/ContextBinding.hpp and ESPressio_Threading.hpp. [Complete contract](Context-Binding).
