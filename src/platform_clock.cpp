#include "platform_clock.hpp"

#include <chrono>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace chips::platform {

std::uint16_t sessionRandomSeed() noexcept {
#ifdef _WIN32
    return static_cast<std::uint16_t>(GetTickCount() & 0xFFFFU);
#else
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    return static_cast<std::uint16_t>(static_cast<std::uint64_t>(milliseconds) & 0xFFFFU);
#endif
}

} // namespace chips::platform
