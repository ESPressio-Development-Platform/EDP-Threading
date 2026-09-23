# src/threading/detail/WaitRegistration.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `ea0bf415eacd70064c3c7216a6c1e6a48cd85038`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/ea0bf415eacd70064c3c7216a6c1e6a48cd85038/src/threading/detail/WaitRegistration.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `limits`
- `TaskRecord.hpp`

## Documented declarations

### `ExecutionContextIndexTraits`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ExecutionContextIndexTraits`.
- **Template parameter `TContextCapacity`:** Number of managed execution contexts represented by the topology.

```cpp
template<std::size_t TContextCapacity>
    struct ExecutionContextIndexTraits final
```

### `Type`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Smallest index Type able to address every managed execution context plus an invalid sentinel.

```cpp
using Type = typename SmallestIndex<TContextCapacity>::Type;
```

### `Invalid`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Sentinel which cannot identify a valid managed execution context.

```cpp
static constexpr Type Invalid = SmallestIndex<TContextCapacity>::Invalid;
```

### `TRecordIndex`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `TaskWaitRegistration`.
- **Template parameter `TRecordIndex`:** Compact Task-record index Type stored by a wait registration.
- **Template parameter `TContextIndex`:** Dense managed execution-context index Type stored by a registration.

```cpp
template<class TRecordIndex, class TContextIndex>
    struct TaskWaitRegistration final
```

### `ContextIndexType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Managed execution-context index Type used by this registration.

```cpp
using ContextIndexType = TContextIndex;
```

### `TRecordIndex RecordIndex{};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Target Task record index.

```cpp
TRecordIndex RecordIndex{};
```

### `Phase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Target Task record incarnation Phase.

```cpp
bool Phase = false;
```

### `max`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waiting managed execution context, or the invalid sentinel while inactive.

```cpp
TContextIndex WaitingContextIndex = std::numeric_limits<TContextIndex>::max();
```

### `IsActive`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this registration currently participates in target wake discovery.

```cpp
bool IsActive() const noexcept
```

### `Clear`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns this registration to its structurally inactive state.

```cpp
void Clear() noexcept
```

### `TContextIndex`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ThreadJoinRegistration`.
- **Template parameter `TContextIndex`:** Dense managed execution-context index Type stored by a registration.

```cpp
template<class TContextIndex>
    struct ThreadJoinRegistration final
```

### `ContextIndexType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Managed execution-context index Type used by this registration.

```cpp
using ContextIndexType = TContextIndex;
```

### `Phase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Target Dedicated Thread activation Phase.

```cpp
bool Phase = false;
```

### `max`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waiting managed execution context, or the invalid sentinel while inactive.

```cpp
TContextIndex WaitingContextIndex = std::numeric_limits<TContextIndex>::max();
```

### `IsActive`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this registration currently participates in target wake discovery.

```cpp
bool IsActive() const noexcept
```

### `Clear`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns this registration to its structurally inactive state.

```cpp
void Clear() noexcept
```

### `TContextIndex`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `AdmissionWaitRegistration`.
- **Template parameter `TContextIndex`:** Dense managed execution-context index Type stored by a registration.

```cpp
template<class TContextIndex>
    struct AdmissionWaitRegistration final
```

### `ContextIndexType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Managed execution-context index Type used by this registration.

```cpp
using ContextIndexType = TContextIndex;
```

### `max`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waiting managed execution context, or the invalid sentinel while inactive.

```cpp
TContextIndex WaitingContextIndex = std::numeric_limits<TContextIndex>::max();
```

### `IsActive`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this registration currently participates in admission-capacity wake discovery.

```cpp
bool IsActive() const noexcept
```

### `Clear`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns this registration to its structurally inactive state.

```cpp
void Clear() noexcept
```

### `TContextIndex`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `ShutdownWaitRegistration`.
- **Template parameter `TContextIndex`:** Dense managed execution-context index Type stored by a registration.

```cpp
template<class TContextIndex>
    struct ShutdownWaitRegistration final
```

### `ContextIndexType`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Managed execution-context index Type used by this registration.

```cpp
using ContextIndexType = TContextIndex;
```

### `max`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Waiting managed execution context, or the invalid sentinel while inactive.

```cpp
TContextIndex WaitingContextIndex = std::numeric_limits<TContextIndex>::max();
```

### `IsActive`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether this registration currently participates in shutdown wake discovery.

```cpp
bool IsActive() const noexcept
```

### `Clear`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns this registration to its structurally inactive state.

```cpp
void Clear() noexcept
```

### `TRegistration`

**Classification:** PRIVATE IMPLEMENTATION

Defines the compile-time contract for `RegistrationSet`.
- **Template parameter `TRegistration`:** Registration record Type stored in the bounded registration set.
- **Template parameter `TCapacity`:** Compile-time bounded capacity represented by this Type.

```cpp
template<class TRegistration, std::size_t TCapacity>
    class RegistrationSet final
```

### `TRegistration _registrations[TCapacity];`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Specialized waiter registrations owned directly by the target resource.

```cpp
TRegistration _registrations[TCapacity];
```

### `Register`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Publishes one complete active registration into the first inactive slot.

The target resource must serialize this operation with target predicate checks,
terminal/phase publication and registration removal.

```cpp
WaitRegistrationStatus Register(
                const TRegistration& registration,
                std::size_t& registrationIndex
            ) noexcept
```

### `Unregister`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Removes one previously published registration.

The caller must supply an index obtained from a successful Register operation and
must serialize removal with the owning target resource's publication protocol.

```cpp
void Unregister(
                std::size_t registrationIndex
            ) noexcept
```

### `ActiveCount`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the number of currently active registrations.

```cpp
std::size_t ActiveCount() const noexcept
```

### `TPredicate`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the number of active registrations satisfying one bounded predicate.
- **Template parameter `TPredicate`:** Predicate callable Type used to locate a registration.

```cpp
template<class TPredicate>
            std::size_t MatchingCount(
                TPredicate&& predicate
            ) const
```

### `TVisitor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Visits every active registration in bounded storage order.
- **Template parameter `TVisitor`:** Visitor callable Type invoked for active registrations.

```cpp
template<class TVisitor>
            void VisitActive(
                TVisitor&& visitor
            )
```

