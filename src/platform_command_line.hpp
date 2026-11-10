#pragma once

#include <optional>
#include <string>
#include <vector>

namespace chips::platform {

// Owns UTF-8 command-line arguments for the lifetime of main(). Windows argv
// is rebuilt from the native wide command line; other platforms copy argv.
class Utf8Arguments {
public:
    [[nodiscard]] static std::optional<Utf8Arguments> fromNative(int argc, char* const argv[]);
    [[nodiscard]] int argc() const noexcept;
    [[nodiscard]] char** argv() noexcept;

private:
    std::vector<std::string> values_;
    std::vector<char*> pointers_;
};

} // namespace chips::platform
