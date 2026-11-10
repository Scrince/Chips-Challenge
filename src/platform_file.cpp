#include "platform_file.hpp"

#include <cerrno>
#include <cstdio>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace chips::platform {

std::error_code replaceFileAtomically(const std::filesystem::path& temporary,
                                      const std::filesystem::path& destination) noexcept {
#ifdef _WIN32
    if (MoveFileExW(temporary.c_str(), destination.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        return {};
    return {static_cast<int>(GetLastError()), std::system_category()};
#else
    if (std::rename(temporary.c_str(), destination.c_str()) == 0) return {};
    return {errno, std::generic_category()};
#endif
}

} // namespace chips::platform
