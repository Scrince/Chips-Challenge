#pragma once

#include <filesystem>
#include <system_error>

namespace chips::platform {

// Replace destination with a fully written temporary file on the same volume.
// An empty result means the replacement succeeded.
[[nodiscard]] std::error_code replaceFileAtomically(
    const std::filesystem::path& temporary,
    const std::filesystem::path& destination) noexcept;

} // namespace chips::platform
