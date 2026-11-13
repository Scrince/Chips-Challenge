#pragma once

#include "audio.hpp"
#include "engine.hpp"
#include "levelpack.hpp"
#include "profile.hpp"

#include <SDL3/SDL.h>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct MotionAnimation {
    int from{-1};
    int to{-1};
    Uint64 start{};
    bool teleported{};
};

inline constexpr std::size_t kBestTimesPerPage = 9;
inline constexpr std::size_t kNoPasswordTarget = static_cast<std::size_t>(-1);
inline constexpr int topMenuItemCount(int menu) {
    return menu == 0 ? 4 : menu == 1 ? 4 : menu == 2 ? 6 : menu == 3 ? 5 : 0;
}
inline constexpr float kTopMenuBarHeight = 28.0F;
inline constexpr float kTopMenuRowHeight = 23.0F;
inline constexpr float kTopMenuDropTop = kTopMenuBarHeight;
inline constexpr float kMainMenuPanelHeight = 356.0F;
inline constexpr float kMainMenuFirstItemY = 52.0F;
inline constexpr float kMainMenuItemRowStep = 32.0F;
inline constexpr float kMainMenuSelectionOffsetY = -5.0F;
inline constexpr float kMainMenuSelectionHeight = 28.0F;
inline constexpr float kGoToFieldWidth = 200.0F;
inline constexpr float kGoToFieldHeight = 36.0F;
inline constexpr std::size_t kGoToInputMaxLength = 10;

// Shared application state passed to platform, input, rendering, audio, and
// persistence code. Game rules remain in the SDL-independent engine module.
struct App {
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* atlas{};
    SDL_Texture* digitalDigits{};
    SDL_Texture* scoreboardPanel{};
    SDL_Texture* appIcon{};
    SDL_Texture* menuBackground{};
    SDL_Texture* gameBackground{};
    SDL_Texture* victoryIllustration{};
    SDL_Texture* originalTransparentSprites{};
    SDL_Texture* polishedPlayerLandPoses{};
    SDL_Texture* polishedPlayerSwimPoses{};
    SDL_Texture* cleanedCreatureSprites{};
    SDL_Texture* cleanedKeyBootIcons{};
    SDL_Gamepad* gamepad{};
    Uint32 levelPackDialogEventType{SDL_EVENT_USER};
    bool levelPackDialogPending{};
    std::atomic_bool levelPackDialogEventPushFailed{};
    bool exitAfterFileDialog{};
    int gamepadStickDirection{-1};
    Uint64 gamepadStickNextRepeat{};
    bool gamepadStickIgnoreHeld{};
    int gamepadDpadDirection{-1};
    Uint64 gamepadDpadNextRepeat{};
    bool gamepadDpadIgnoreHeld{};
    int keyboardDirection{-1};
    int keyboardScancode{-1};
    Uint64 keyboardRepeatAt{};
    chips::AudioSystem audio;
    std::vector<chips::Level> levels;
    std::size_t levelIndex{};
    chips::GameState game;
    chips::CampaignProfile profile;
    std::filesystem::path profilePath;
    std::string profileFileName{"campaign.ccprofile"};
    std::string profileMessage;
    Uint64 lastTick{};
    Uint64 lastPlayerTick{};
    bool running{true};
    bool paused{};
    bool helpVisible{};
    bool debugVisible{};
    bool debugAllowKeypadMovement{};
    std::string debugLastMovement;
    std::string debugLastTickMovement;
    std::filesystem::path debugInputLogPath;
    bool bestTimesVisible{};
    std::size_t bestTimesPage{};
    bool settingsVisible{};
    bool mainMenuVisible{true};
    bool passwordVisible{};
    bool goToDialogVisible{};
    bool goToReturnToLevelSelect{};
    bool goToPasswordFocus{};
    int goToButtonSelection{};
    std::string goToLevelInput;
    std::string goToPasswordInput;
    std::string goToDialogMessage;
    std::size_t passwordTargetLevel{kNoPasswordTarget};
    bool passwordReturnToLevelSelect{};
    std::string passwordInput;
    std::string passwordMessage;
    std::size_t menuSelection{};
    int actionSelection{};
    int topMenuOpen{-1};
    int topMenuSelection{-1};
    int lastMouseX{-1};
    int lastMouseY{-1};
    bool topMenuKeyboardFocus{};
    int waitingForBinding{-1};
    bool levelSelect{};
    std::size_t selectedLevel{};
    bool fullscreen{};
    bool displayProfileDirty{};
    Uint64 displayProfileSaveAt{};
    bool winRecorded{};
    bool levelPlacardVisible{};
    unsigned levelRestartCount{};
    std::uint64_t levelTimeBonus{};
    std::uint64_t levelCompletionBonus{};
    std::uint64_t levelCompletionScore{};
    std::uint64_t totalScore{};
    bool timeRecordEstablished{};
    bool timeRecordImproved{};
    std::uint32_t timeRecordImprovementSeconds{};
    int playerAnimationFrom{-1};
    int playerAnimationTo{-1};
    Uint64 playerAnimationStart{};
    bool playerAnimationTeleported{};
    std::vector<MotionAnimation> creatureAnimations;
};

[[nodiscard]] inline bool levelRequiresPassword(const App& app, std::size_t index) {
    return index < app.levels.size() && index < app.profile.completed.size() &&
           index >= app.profile.highestUnlocked && !app.profile.completed[index] &&
           !app.levels[index].password.empty();
}
