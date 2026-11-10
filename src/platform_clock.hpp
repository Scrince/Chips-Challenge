#pragma once

#include <cstdint>

namespace chips::platform {

// The original game seeds its CRT RNG from the low word of the Windows
// milliseconds-since-startup clock. Other platforms use a monotonic clock.
[[nodiscard]] std::uint16_t sessionRandomSeed() noexcept;

} // namespace chips::platform
