#include "password.hpp"

#include <string>

namespace chips {
namespace {

std::string normalizePassword(std::string_view raw) {
    std::string normalized;
    normalized.reserve(raw.size());
    for (const unsigned char ch : raw) {
        if (ch >= 'a' && ch <= 'z') normalized.push_back(static_cast<char>(ch - 'a' + 'A'));
        else if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
            normalized.push_back(static_cast<char>(ch));
        else if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') continue;
        else return {};
    }
    return normalized;
}

} // namespace

std::optional<std::size_t> findLevelByPassword(
    std::string_view input, const std::vector<Level>& levels) {
    const auto entered = normalizePassword(input);
    if (entered.empty()) return std::nullopt;
    for (std::size_t index = 0; index < levels.size(); ++index) {
        if (entered == normalizePassword(levels[index].password)) return index;
    }
    return std::nullopt;
}

bool passwordMatches(std::string_view input, std::string_view password) {
    const auto entered = normalizePassword(input);
    return !entered.empty() && entered == normalizePassword(password);
}

} // namespace chips
