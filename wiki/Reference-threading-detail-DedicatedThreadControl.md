# src/threading/detail/DedicatedThreadControl.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `50dbad50650df9fc38dd17cd3b17a60a92a5d802`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Threading/blob/50dbad50650df9fc38dd17cd3b17a60a92a5d802/src/threading/detail/DedicatedThreadControl.hpp)

## Direct includes

- `cstdint`

## Documented declarations

### `_value`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Packed operational state and activation Phase.

```cpp
std::uint8_t _value;
```

### `StateMask`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bit mask containing the encoded Dedicated Thread operational state.

```cpp
static constexpr std::uint8_t StateMask = 0x03U;
```

### `PhaseMask`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bit mask containing the one-bit Dedicated Thread activation Phase.

```cpp
static constexpr std::uint8_t PhaseMask = 0x04U;
```

### `DedicatedThreadControl`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates control state in the never-started activation phase.

```cpp
DedicatedThreadControl() noexcept :
                _value(
                    static_cast<std::uint8_t>(
                        DedicatedThreadOperationalState::NeverStarted
                    )
                ) {}
```

### `State`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the internal Dedicated Thread operational state.

```cpp
DedicatedThreadOperationalState State() const noexcept
```

### `Phase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Returns the current activation Phase.

```cpp
bool Phase() const noexcept
```

### `IsStopRequested`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Indicates whether cooperative stop has been requested for the active activation.

```cpp
bool IsStopRequested() const noexcept
```

### `TryStart`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Starts a new activation and toggles the activation Phase under the owning runtime mutex.

```cpp
DedicatedThreadControlStartResult TryStart(
                bool& activationPhase
            ) noexcept
```

### `TryRequestStop`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Requests cooperative stop for the current activation.

The owning Dedicated Thread runtime serializes every mutation through its mutex.

```cpp
DedicatedThreadControlStopRequestResult TryRequestStop() noexcept
```

### `TryPublishStopped`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Publishes completion only for the activation Phase that actually returned.

The owning Dedicated Thread runtime serializes every mutation through its mutex.

```cpp
DedicatedThreadControlPublicationResult TryPublishStopped(
                bool activationPhase
            ) noexcept
```

