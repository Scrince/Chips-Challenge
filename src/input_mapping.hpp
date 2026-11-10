#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace chips {

enum class BindingRejection : std::uint8_t { none, reserved, duplicate };

// Platform adapters provide their native key codes; mapping remains independent
// of SDL, Win32, and any particular keyboard API.
[[nodiscard]] int movementIndexForKey(std::uint16_t keyCode,
                                      const std::array<std::uint16_t, 4>& bindings,
                                      const std::array<std::uint16_t, 4>& fallbackKeys);
[[nodiscard]] bool isReservedKey(std::uint16_t keyCode,
                                 std::span<const std::uint16_t> reservedKeys);
[[nodiscard]] BindingRejection validateMovementBinding(
    std::uint16_t keyCode, std::size_t directionIndex,
    const std::array<std::uint16_t, 4>& bindings,
    std::span<const std::uint16_t> reservedKeys,
    std::span<const std::uint16_t> fallbackKeys);

} // namespace chips
