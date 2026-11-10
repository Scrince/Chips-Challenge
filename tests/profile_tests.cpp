#include "profile.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

int main() {
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto directory = std::filesystem::temp_directory_path() /
        ("chips-native-profile-test-" + std::to_string(unique));
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) {
        std::cerr << "Cannot create profile test directory: " << error.message() << '\n';
        return EXIT_FAILURE;
    }
    const auto profilePath = directory / "profile.bin";
    chips::CampaignProfile expected;
    expected.highestUnlocked = 12;
    expected.lastSelectedLevel = 8;
    expected.completed[6] = true;
    expected.bestTimes[6] = 93;
    expected.effectsVolumePercent = 35;
    expected.effectsMuted = true;
    expected.musicVolumePercent = 42;
    expected.musicMuted = true;
    expected.movementBindings = {41, 42, 43, 44};
    expected.uiScalePercent = 150;
    expected.fullscreen = true;
    expected.windowWidth = 1440;
    expected.windowHeight = 900;
    chips::saveProfile(profilePath, expected);

    std::string warning;
    const auto loaded = chips::loadProfile(profilePath, &warning);
    if (!warning.empty() || loaded.highestUnlocked != expected.highestUnlocked ||
        loaded.lastSelectedLevel != expected.lastSelectedLevel || !loaded.completed[6] ||
        loaded.bestTimes[6] != 93 || loaded.effectsVolumePercent != 35 || !loaded.effectsMuted ||
        loaded.musicVolumePercent != expected.musicVolumePercent || !loaded.musicMuted ||
        loaded.movementBindings != expected.movementBindings ||
        loaded.uiScalePercent != expected.uiScalePercent || loaded.fullscreen != expected.fullscreen ||
        loaded.windowWidth != expected.windowWidth || loaded.windowHeight != expected.windowHeight) {
        std::cerr << "Version 8 profile round-trip failed: " << warning << '\n';
        return EXIT_FAILURE;
    }

    std::ifstream input(profilePath, std::ios::binary);
    const std::vector<unsigned char> current((std::istreambuf_iterator<char>(input)), {});
    input.close();
    auto writeVariant = [&](const std::string& name, std::vector<unsigned char> bytes,
                            std::uint16_t version) {
        bytes[8] = static_cast<unsigned char>(version & 0xFF);
        bytes[9] = static_cast<unsigned char>(version >> 8);
        const auto variantPath = directory / name;
        std::ofstream output(variantPath, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
        return variantPath;
    };
    auto version4 = current;
    version4.resize(version4.size() - 8); // Remove version-6/7/8 presentation and music settings.
    version4.erase(version4.begin() + 14, version4.begin() + 16); // v5 slot-count word.
    const auto version4Path = writeVariant("version4.bin", version4, 4);
    const auto migrated4 = chips::loadProfile(version4Path, &warning);
    if (!warning.empty() || migrated4.highestUnlocked != expected.highestUnlocked ||
        !migrated4.completed[6] || migrated4.bestTimes[6] != 93 ||
        migrated4.uiScalePercent != expected.uiScalePercent) {
        std::cerr << "Version 4 profile migration failed: " << warning << '\n';
        return EXIT_FAILURE;
    }

    auto version3 = version4;
    version3.resize(version3.size() - 2);
    const auto version3Path = writeVariant("version3.bin", version3, 3);
    const auto migrated3 = chips::loadProfile(version3Path, &warning);
    if (!warning.empty() || migrated3.movementBindings != expected.movementBindings ||
        migrated3.uiScalePercent != 100) {
        std::cerr << "Version 3 profile migration failed: " << warning << '\n';
        return EXIT_FAILURE;
    }

    auto version2 = version3;
    version2.resize(version2.size() - 8);
    const auto version2Path = writeVariant("version2.bin", version2, 2);
    const auto migrated2 = chips::loadProfile(version2Path, &warning);
    if (!warning.empty() || migrated2.effectsVolumePercent != 35 || !migrated2.effectsMuted ||
        migrated2.movementBindings != chips::CampaignProfile{}.movementBindings ||
        migrated2.uiScalePercent != 100) {
        std::cerr << "Version 2 profile migration failed: " << warning << '\n';
        return EXIT_FAILURE;
    }

    auto legacy = version2;
    legacy.resize(legacy.size() - 2);
    const auto legacyPath = writeVariant("legacy.bin", legacy, 1);
    const auto migrated = chips::loadProfile(legacyPath, &warning);
    if (!warning.empty() || migrated.highestUnlocked != expected.highestUnlocked ||
        migrated.effectsVolumePercent != 65 || migrated.effectsMuted || migrated.uiScalePercent != 100) {
        std::cerr << "Version 1 profile migration failed: " << warning << '\n';
        return EXIT_FAILURE;
    }

    const auto writeRawVariant = [&](const std::string& name,
                                     const std::vector<unsigned char>& bytes) {
        const auto variantPath = directory / name;
        std::ofstream output(variantPath, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
        output.close();
        return variantPath;
    };
    const auto expectFreshWithWarning = [&](const std::filesystem::path& invalidPath,
                                             const std::string& warningFragment,
                                             const std::string& label) {
        std::string invalidWarning;
        const auto fresh = chips::loadProfile(invalidPath, &invalidWarning);
        const bool freshDefaults = fresh.highestUnlocked == 1 && fresh.lastSelectedLevel == 1 &&
            fresh.completed.size() == chips::kCampaignLevelCount &&
            std::all_of(fresh.completed.begin(), fresh.completed.end(),
                        [](std::uint8_t completed) { return completed == 0; }) &&
            std::all_of(fresh.bestTimes.begin(), fresh.bestTimes.end(),
                        [](std::uint32_t time) { return time == 0xFFFFFFFFU; });
        if (!freshDefaults || invalidWarning.find(warningFragment) == std::string::npos) {
            std::cerr << label << " did not recover with fresh defaults and an actionable warning: "
                      << invalidWarning << '\n';
            return false;
        }
        return true;
    };
    warning.clear();
    const auto missing = chips::loadProfile(directory / "missing-profile.bin", &warning);
    if (!warning.empty() || missing.highestUnlocked != 1 || missing.lastSelectedLevel != 1 ||
        std::any_of(missing.completed.begin(), missing.completed.end(),
                    [](std::uint8_t completed) { return completed != 0; })) {
        std::cerr << "Missing profile did not start with fresh progress.\n";
        return EXIT_FAILURE;
    }
    const auto badSignaturePath = writeRawVariant("bad-signature.bin", {'B','A','D','P','R','O','F','1','!','!'});
    if (!expectFreshWithWarning(badSignaturePath, "signature is invalid", "Bad-signature profile"))
        return EXIT_FAILURE;
    auto truncatedBytes = current;
    truncatedBytes.resize(9);
    const auto truncatedPath = writeRawVariant("truncated.bin", truncatedBytes);
    if (!expectFreshWithWarning(truncatedPath, "invalid size", "Truncated profile"))
        return EXIT_FAILURE;
    auto unsupportedBytes = current;
    unsupportedBytes[8] = 9;
    unsupportedBytes[9] = 0;
    const auto unsupportedPath = writeRawVariant("unsupported-version.bin", unsupportedBytes);
    if (!expectFreshWithWarning(unsupportedPath, "unsupported", "Unsupported-version profile"))
        return EXIT_FAILURE;
    auto invalidCompletionBytes = current;
    invalidCompletionBytes[16] = 2;
    const auto invalidCompletionPath = writeRawVariant("invalid-completion.bin", invalidCompletionBytes);
    if (!expectFreshWithWarning(invalidCompletionPath, "completion data is invalid",
                                "Invalid-completion profile"))
        return EXIT_FAILURE;
    auto invalidScaleBytes = current;
    invalidScaleBytes[invalidScaleBytes.size() - 9] = 126;
    const auto invalidScalePath = writeRawVariant("invalid-scale.bin", invalidScaleBytes);
    if (!expectFreshWithWarning(invalidScalePath, "display scale is invalid",
                                "Invalid-scale profile"))
        return EXIT_FAILURE;

    chips::CampaignProfile largePack;
    largePack.completed.assign(300, 0);
    largePack.bestTimes.assign(300, 0xFFFFFFFFU);
    largePack.highestUnlocked = 250;
    largePack.lastSelectedLevel = 220;
    largePack.completed[219] = 1;
    largePack.bestTimes[219] = 42;
    if (!chips::recordLevelSelection(largePack, 299, 300) || largePack.lastSelectedLevel != 300) {
        std::cerr << "Custom-pack level selection was not stored as a one-based profile level.\n";
        return EXIT_FAILURE;
    }
    const auto largePath = directory / "large-pack.bin";
    chips::saveProfile(largePath, largePack);
    const auto largeLoaded = chips::loadProfile(largePath, &warning, 300);
    if (!warning.empty() || largeLoaded.completed.size() != 300 ||
        largeLoaded.highestUnlocked != 250 || largeLoaded.lastSelectedLevel != 300 ||
        !largeLoaded.completed[219] || largeLoaded.bestTimes[219] != 42) {
        std::cerr << "Variable-length custom pack profile round-trip failed: " << warning << '\n';
        return EXIT_FAILURE;
    }

    const auto expanded = chips::loadProfile(profilePath, &warning, 300);
    if (!warning.empty() || expanded.completed.size() != 300 || !expanded.completed[6] ||
        expanded.bestTimes[6] != 93 || expanded.completed[299] != 0 ||
        expanded.bestTimes[299] != 0xFFFFFFFFU) {
        std::cerr << "Profile resize/migration to custom pack size failed: " << warning << '\n';
        return EXIT_FAILURE;
    }

    chips::CampaignProfile campaign;
    if (!chips::recordLevelSelection(campaign, chips::kCampaignLevelCount - 1,
                                     chips::kCampaignLevelCount) ||
        campaign.lastSelectedLevel != chips::kCampaignLevelCount) {
        std::cerr << "Last campaign selection was not stored as a one-based profile level.\n";
        return EXIT_FAILURE;
    }
    if (!chips::recordLevelCompletion(campaign, 0, chips::kCampaignLevelCount, true, 90) ||
        !campaign.completed[0] || campaign.highestUnlocked != 2 || campaign.bestTimes[0] != 90) {
        std::cerr << "First timed campaign completion did not mark progress, unlock level 2, and store its time.\n";
        return EXIT_FAILURE;
    }
    if (!chips::recordLevelCompletion(campaign, 0, chips::kCampaignLevelCount, true, 100) ||
        campaign.bestTimes[0] != 90 ||
        !chips::recordLevelCompletion(campaign, 0, chips::kCampaignLevelCount, true, 75) ||
        campaign.bestTimes[0] != 75) {
        std::cerr << "Timed campaign retry did not preserve the best time.\n";
        return EXIT_FAILURE;
    }
    if (!chips::recordLevelCompletion(campaign, 1, chips::kCampaignLevelCount, false, 20) ||
        !campaign.completed[1] || campaign.highestUnlocked != 3 ||
        campaign.bestTimes[1] != 0xFFFFFFFFU) {
        std::cerr << "Untimed campaign completion changed time data or failed to unlock the next level.\n";
        return EXIT_FAILURE;
    }
    campaign.highestUnlocked = static_cast<std::uint16_t>(chips::kCampaignLevelCount - 1);
    const auto finalLevel = chips::kCampaignLevelCount - 1;
    if (!chips::recordLevelCompletion(campaign, finalLevel, chips::kCampaignLevelCount, false, 0) ||
        !campaign.completed[finalLevel] || campaign.highestUnlocked != chips::kCampaignLevelCount) {
        std::cerr << "Final campaign completion did not record progress without unlocking past the campaign.\n";
        return EXIT_FAILURE;
    }
    const auto beforeInvalidCompletion = campaign;
    if (chips::recordLevelSelection(campaign, chips::kCampaignLevelCount,
                                    chips::kCampaignLevelCount) ||
        chips::recordLevelSelection(campaign, 0, 0x10000U) ||
        campaign.lastSelectedLevel != beforeInvalidCompletion.lastSelectedLevel) {
        std::cerr << "Invalid level selection modified the saved last-selected level.\n";
        return EXIT_FAILURE;
    }
    if (chips::recordLevelCompletion(campaign, chips::kCampaignLevelCount,
                                     chips::kCampaignLevelCount, true, 1) ||
        chips::recordLevelCompletion(campaign, 0, 0x10000U, true, 1) ||
        campaign.completed != beforeInvalidCompletion.completed ||
        campaign.bestTimes != beforeInvalidCompletion.bestTimes ||
        campaign.highestUnlocked != beforeInvalidCompletion.highestUnlocked) {
        std::cerr << "Out-of-range campaign completion modified profile progress.\n";
        return EXIT_FAILURE;
    }
    chips::saveProfile(profilePath, campaign);
    const auto savedCampaign = chips::loadProfile(profilePath, &warning);
    if (!warning.empty() || savedCampaign.highestUnlocked != chips::kCampaignLevelCount ||
        savedCampaign.lastSelectedLevel != chips::kCampaignLevelCount ||
        !savedCampaign.completed[0] || !savedCampaign.completed[1] ||
        !savedCampaign.completed[finalLevel] || savedCampaign.bestTimes[0] != 75 ||
        savedCampaign.bestTimes[1] != 0xFFFFFFFFU) {
        std::cerr << "Recorded campaign completion did not persist across profile reload: " << warning << '\n';
        return EXIT_FAILURE;
    }

    std::filesystem::remove_all(directory, error);
    std::cout << "Profile settings, campaign completion, best times, and migrations passed.\n";
    return EXIT_SUCCESS;
}
