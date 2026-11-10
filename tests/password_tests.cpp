#include "levelpack.hpp"
#include "password.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    try {
        const auto levels = chips::loadLevelPack(CHIPS_TEST_PACK_PATH);
        if (levels.size() != chips::kCampaignLevelCount) {
            std::cerr << "Password test expected all 149 campaign levels\n";
            return EXIT_FAILURE;
        }

        const auto first = chips::findLevelByPassword("BDHP", levels);
        if (!first || *first != 0) {
            std::cerr << "Canonical level 1 password did not resolve to level 1\n";
            return EXIT_FAILURE;
        }
        const auto mixedCase = chips::findLevelByPassword(" b d h p ", levels);
        if (!mixedCase || *mixedCase != 0) {
            std::cerr << "Password matching did not ignore case and ASCII spaces\n";
            return EXIT_FAILURE;
        }
        if (chips::findLevelByPassword("BDH?", levels) ||
            chips::findLevelByPassword(" \t\r\n ", levels) ||
            chips::findLevelByPassword("NO-SUCH-LEVEL", levels)) {
            std::cerr << "Invalid or empty password unexpectedly matched a level\n";
            return EXIT_FAILURE;
        }

        for (std::size_t i = 0; i < levels.size(); ++i) {
            const auto found = chips::findLevelByPassword(levels[i].password, levels);
            if (!found || levels[*found].password != levels[i].password) {
                std::cerr << "Campaign password did not resolve at level " << i + 1 << '\n';
                return EXIT_FAILURE;
            }
        }

        std::cout << "Password normalization and lookup validated against all 149 levels.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Password tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
