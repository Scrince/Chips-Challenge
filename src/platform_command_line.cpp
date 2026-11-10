#include "platform_command_line.hpp"

#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#endif

namespace chips::platform {

std::optional<Utf8Arguments> Utf8Arguments::fromNative(int argc, char* const argv[]) {
    Utf8Arguments result;
#ifdef _WIN32
    (void)argc;
    (void)argv;
    int wideCount = 0;
    LPWSTR* wideArguments = CommandLineToArgvW(GetCommandLineW(), &wideCount);
    if (!wideArguments) return std::nullopt;
    result.values_.reserve(static_cast<std::size_t>(wideCount));
    for (int i = 0; i < wideCount; ++i) {
        const int required = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
            wideArguments[i], -1, nullptr, 0, nullptr, nullptr);
        if (required <= 0) {
            LocalFree(wideArguments);
            return std::nullopt;
        }
        std::string value(static_cast<std::size_t>(required), '\0');
        if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wideArguments[i], -1,
                value.data(), required, nullptr, nullptr) <= 0) {
            LocalFree(wideArguments);
            return std::nullopt;
        }
        value.resize(static_cast<std::size_t>(required - 1));
        result.values_.push_back(std::move(value));
    }
    LocalFree(wideArguments);
#else
    if (argc < 0 || (argc > 0 && argv == nullptr)) return std::nullopt;
    result.values_.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        if (argv[i] == nullptr) return std::nullopt;
        result.values_.emplace_back(argv[i]);
    }
#endif
    return result;
}

int Utf8Arguments::argc() const noexcept {
    return static_cast<int>(values_.size());
}

char** Utf8Arguments::argv() noexcept {
    pointers_.clear();
    pointers_.reserve(values_.size() + 1);
    for (auto& value : values_) pointers_.push_back(value.data());
    pointers_.push_back(nullptr);
    return pointers_.data();
}

} // namespace chips::platform
