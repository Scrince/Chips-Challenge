#include "levelpack.hpp"
#include "engine.hpp"
#include "profile.hpp"
#include "audio.hpp"
#include "password.hpp"
#include "app_state.hpp"
#include "renderer.hpp"
#include "input_mapping.hpp"
#include "platform_command_line.hpp"
#include "platform_clock.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using chips_native::draw;
using chips_native::hasAtlasMapping;
using chips_native::boardPositionAtOutput;
using chips_native::levelGridLayout;
using chips_native::settingsOverlayLayout;
using chips_native::bestTimesOverlayLayout;
using chips_native::uiScaleForOutput;
constexpr std::size_t kLevelSelectorColumns = 13;
constexpr std::size_t kLevelSelectorRows = 12;
constexpr std::size_t kLevelSelectorPageSize = kLevelSelectorColumns * kLevelSelectorRows;

std::string pathToUtf8(const std::filesystem::path& path) {
    const auto encoded = path.u8string();
    return {reinterpret_cast<const char*>(encoded.data()), encoded.size()};
}

std::filesystem::path pathFromUtf8(std::string_view encoded) {
    std::u8string pathBytes;
    pathBytes.reserve(encoded.size());
    for (const unsigned char byte : encoded)
        pathBytes.push_back(static_cast<char8_t>(byte));
    return std::filesystem::path(pathBytes);
}

const std::array<std::uint16_t, 4> kArrowScancodes{
    SDL_SCANCODE_UP, SDL_SCANCODE_RIGHT, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT};
const std::array<std::uint16_t, 25> kReservedMovementScancodes{
    SDL_SCANCODE_ESCAPE, SDL_SCANCODE_RETURN, SDL_SCANCODE_F1, SDL_SCANCODE_F2,
    SDL_SCANCODE_F3, SDL_SCANCODE_F4, SDL_SCANCODE_F5, SDL_SCANCODE_F6,
    SDL_SCANCODE_F7, SDL_SCANCODE_F8, SDL_SCANCODE_F9, SDL_SCANCODE_F10,
    SDL_SCANCODE_F11, SDL_SCANCODE_P, SDL_SCANCODE_R, SDL_SCANCODE_L,
    SDL_SCANCODE_O, SDL_SCANCODE_M, SDL_SCANCODE_MINUS, SDL_SCANCODE_EQUALS,
    SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET, SDL_SCANCODE_PAGEUP,
    SDL_SCANCODE_PAGEDOWN, SDL_SCANCODE_UNKNOWN};

int movementIndexForScancode(const App& app, SDL_Scancode scancode) {
    return chips::movementIndexForKey(static_cast<std::uint16_t>(scancode),
                                      app.profile.movementBindings, kArrowScancodes);
}

void resetPlayer(App& app) {
    const auto tickPhase = app.game.gameTicks;
    const auto randomPhase = app.game.randomState;
    app.game = chips::GameState::start(app.levels[app.levelIndex], tickPhase, randomPhase);
    app.winRecorded = false;
    app.levelPlacardVisible = true;
    app.levelTimeBonus = 0;
    app.levelCompletionBonus = 0;
    app.levelCompletionScore = 0;
    app.timeRecordEstablished = false;
    app.timeRecordImproved = false;
    app.timeRecordImprovementSeconds = 0;
    app.actionSelection = 0;
    app.playerAnimationFrom = -1;
    app.playerAnimationTo = -1;
    app.playerAnimationTeleported = false;
    app.creatureAnimations.clear();
}

void setPaused(App& app, bool paused) {
    if (app.paused != paused) app.lastTick = SDL_GetTicks();
    if (paused && !app.paused) {
        // A pause is a hard input boundary. Do not carry queued movement or a
        // held controller direction across Resume.
        app.game.pendingDirection.reset();
        app.actionSelection = 0;
        constexpr int releaseThreshold = 8000;
        const int stickX = app.gamepad ? SDL_GetGamepadAxis(app.gamepad, SDL_GAMEPAD_AXIS_LEFTX) : 0;
        const int stickY = app.gamepad ? SDL_GetGamepadAxis(app.gamepad, SDL_GAMEPAD_AXIS_LEFTY) : 0;
        app.gamepadStickIgnoreHeld = std::abs(stickX) >= releaseThreshold ||
                                     std::abs(stickY) >= releaseThreshold;
        app.gamepadDpadIgnoreHeld = app.gamepad &&
            (SDL_GetGamepadButton(app.gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP) ||
             SDL_GetGamepadButton(app.gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT) ||
             SDL_GetGamepadButton(app.gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN) ||
             SDL_GetGamepadButton(app.gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT));
        app.gamepadStickDirection = -1;
        app.gamepadStickNextRepeat = 0;
        app.gamepadDpadDirection = -1;
        app.gamepadDpadNextRepeat = 0;
    }
    app.paused = paused;
}

void togglePause(App& app) {
    setPaused(app, !app.paused);
}

void persistProfile(App& app, bool announce = true);

void selectLevel(App& app, std::size_t index) {
    if (index >= app.levels.size()) return;
    app.levelIndex = index;
    app.levelRestartCount = 0;
    const auto title = app.levels[index].title.empty() ? std::string("Untitled") : app.levels[index].title;
    SDL_SetWindowTitle(app.window, (std::string("Chip's Challenge: ") + title).c_str());
    app.levelSelect = false;
    app.mainMenuVisible = false;
    app.paused = false;
    app.helpVisible = false;
    resetPlayer(app);
    if (chips::recordLevelSelection(app.profile, index, app.levels.size())) {
        persistProfile(app);
    }
}

void requestLevelSelection(App& app, std::size_t index) {
    if (index >= app.levels.size()) return;
    if (!levelRequiresPassword(app, index)) {
        selectLevel(app, index);
        return;
    }
    app.selectedLevel = index;
    app.passwordTargetLevel = index;
    app.passwordReturnToLevelSelect = true;
    app.passwordInput.clear();
    app.passwordMessage = "PASSWORD REQUIRED FOR LEVEL " +
        std::to_string(index + 1) + ". ESC CANCELS.";
    app.levelSelect = false;
    app.passwordVisible = true;
    (void)SDL_StartTextInput(app.window);
}

void activateMenuSelection(App& app) {
    switch (app.menuSelection) {
        case 0:
            app.mainMenuVisible = false;
            app.levelPlacardVisible = true;
            break;
        case 1:
            app.bestTimesVisible = false;
            app.paused = false;
            app.totalScore = 0;
            selectLevel(app, 0);
            break;
        case 2:
            app.selectedLevel = app.levelIndex;
            app.levelSelect = true;
            break;
        case 3:
            app.passwordInput.clear();
            app.passwordMessage.clear();
            app.passwordTargetLevel = kNoPasswordTarget;
            app.passwordReturnToLevelSelect = false;
            app.passwordVisible = true;
            (void)SDL_StartTextInput(app.window);
            break;
        case 4:
            app.settingsVisible = true;
            app.waitingForBinding = -1;
            break;
        case 5: app.helpVisible = true; break;
        case 6: app.running = false; break;
        default: break;
    }
}

void closePasswordEntry(App& app) {
    const bool returnToLevelSelect = app.passwordReturnToLevelSelect;
    app.passwordVisible = false;
    app.passwordTargetLevel = kNoPasswordTarget;
    app.passwordReturnToLevelSelect = false;
    if (returnToLevelSelect) app.levelSelect = true;
    (void)SDL_StopTextInput(app.window);
}

void submitPassword(App& app) {
    std::optional<std::size_t> levelIndex;
    if (app.passwordTargetLevel != kNoPasswordTarget) {
        if (app.passwordTargetLevel < app.levels.size() &&
            chips::passwordMatches(app.passwordInput,
                                   app.levels[app.passwordTargetLevel].password)) {
            levelIndex = app.passwordTargetLevel;
        } else {
            app.passwordMessage = "THAT PASSWORD DOES NOT MATCH THE SELECTED LEVEL.";
            return;
        }
    } else {
        levelIndex = chips::findLevelByPassword(app.passwordInput, app.levels);
    }

    if (levelIndex) {
        app.profile.highestUnlocked = static_cast<std::uint16_t>(std::max<std::size_t>(
            app.profile.highestUnlocked, *levelIndex + 1));
        closePasswordEntry(app);
        selectLevel(app, *levelIndex);
        app.profileMessage = "Password accepted; level loaded.";
        return;
    }
    app.passwordMessage = "Password not found. Try again or press Escape.";
}

void changeLevel(App& app, int delta) {
    const auto count = static_cast<int>(app.levels.size());
    const auto current = static_cast<int>(app.levelIndex);
    requestLevelSelection(app, static_cast<std::size_t>((current + delta + count) % count));
}

bool isCampaignComplete(const App& app) {
    return app.game.outcome == chips::Outcome::won && !app.levels.empty() &&
           app.levelIndex + 1 >= app.levels.size();
}

void advanceAfterWin(App& app) {
    if (!isCampaignComplete(app)) {
        changeLevel(app, 1);
        return;
    }
    app.paused = false;
    app.levelSelect = false;
    app.mainMenuVisible = true;
    app.menuSelection = 0;
}

void changeLevelSelectPage(App& app, int delta) {
    if (app.levels.empty()) return;
    const auto currentPage = app.selectedLevel / kLevelSelectorPageSize;
    const auto pageCount = (app.levels.size() + kLevelSelectorPageSize - 1) /
                          kLevelSelectorPageSize;
    const auto targetPage = delta < 0
        ? (currentPage > 0 ? currentPage - 1 : 0)
        : std::min(currentPage + 1, pageCount - 1);
    const auto withinPage = app.selectedLevel % kLevelSelectorPageSize;
    app.selectedLevel = std::min(targetPage * kLevelSelectorPageSize + withinPage,
                                 app.levels.size() - 1);
}

std::size_t bestTimesPageCount(const App& app) {
    std::size_t timedLevels = 0;
    const auto count = std::min(app.profile.bestTimes.size(), app.levels.size());
    for (std::size_t i = 0; i < count; ++i)
        if (app.profile.bestTimes[i] != 0xFFFFFFFFU) ++timedLevels;
    return std::max<std::size_t>(1, (timedLevels + kBestTimesPerPage - 1) / kBestTimesPerPage);
}

void changeBestTimesPage(App& app, int delta) {
    const auto pageCount = bestTimesPageCount(app);
    if (delta < 0) {
        if (app.bestTimesPage > 0) --app.bestTimesPage;
    } else if (app.bestTimesPage + 1 < pageCount) {
        ++app.bestTimesPage;
    }
}

void handleBestTimesPointer(App& app, float windowX, float windowY) {
    int physicalWidth = 0, physicalHeight = 0, windowWidth = 0, windowHeight = 0;
    SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
    SDL_GetWindowSize(app.window, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0) return;
    const float scale = uiScaleForOutput(physicalWidth, physicalHeight, app.profile.uiScalePercent);
    const float x = windowX * static_cast<float>(physicalWidth) / static_cast<float>(windowWidth) / scale;
    const float y = windowY * static_cast<float>(physicalHeight) / static_cast<float>(windowHeight) / scale;
    const int outputWidth = std::max(1, static_cast<int>(static_cast<float>(physicalWidth) / scale));
    const int outputHeight = std::max(1, static_cast<int>(static_cast<float>(physicalHeight) / scale));
    const auto layout = bestTimesOverlayLayout(outputWidth, outputHeight);
    const auto inside = [x, y](const SDL_FRect& bounds) {
        return x >= bounds.x && x < bounds.x + bounds.w && y >= bounds.y && y < bounds.y + bounds.h;
    };
    if (inside(layout.previousPageButton)) changeBestTimesPage(app, -1);
    else if (inside(layout.nextPageButton)) changeBestTimesPage(app, 1);
}

void movePlayer(App& app, chips::Direction direction) {
    if (app.paused || app.helpVisible || app.debugVisible || app.settingsVisible ||
        app.levelSelect || app.passwordVisible || app.mainMenuVisible ||
        app.game.outcome != chips::Outcome::playing)
        return;

    if (app.levelPlacardVisible) {
        app.levelPlacardVisible = false;
        app.lastTick = SDL_GetTicks();
    }

    const int oldPosition = app.game.playerPosition;
    const auto oldOutcome = app.game.outcome;
    if (app.game.pressMove(direction) == chips::MoveResult::ignored) return;

    const auto now = SDL_GetTicks();
    if (app.game.playerPosition != oldPosition) {
        app.playerAnimationFrom = oldPosition;
        app.playerAnimationTo = app.game.playerPosition;
        app.playerAnimationStart = now;
        app.playerAnimationTeleported = app.game.playerTeleportedThisTick;
    }

    if (oldOutcome == chips::Outcome::playing && app.game.outcome != oldOutcome) {
        app.audio.play(app.game.outcome == chips::Outcome::won
            ? chips::AudioCue::victory : chips::AudioCue::death);
    } else if (app.game.playerPosition != oldPosition) {
        const auto pickupCue = app.game.playerPickupThisTick == chips::PickupKind::key
            ? chips::AudioCue::keyPickup
            : app.game.playerPickupThisTick == chips::PickupKind::boot ? chips::AudioCue::bootPickup
            : chips::AudioCue::pickup;
        app.audio.play(app.game.playerTeleportedThisTick ? chips::AudioCue::teleport
            : app.game.switchActivatedThisTick ? chips::AudioCue::switchTile
            : app.game.playerPickupThisTick != chips::PickupKind::none ? pickupCue
            : chips::AudioCue::move);
    } else {
        app.audio.play(chips::AudioCue::blocked);
    }
}

void openFirstGamepad(App& app) {
    if (app.gamepad) return;
    int count = 0;
    SDL_JoystickID* gamepadIds = SDL_GetGamepads(&count);
    if (!gamepadIds) return;
    for (int i = 0; i < count && !app.gamepad; ++i) {
        app.gamepad = SDL_OpenGamepad(gamepadIds[i]);
        if (!app.gamepad)
            SDL_Log("Unable to open gamepad %u: %s", static_cast<unsigned>(gamepadIds[i]), SDL_GetError());
    }
    SDL_free(gamepadIds);
}

bool initialize(App& app) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }
    app.game.randomState = chips::platform::sessionRandomSeed();
    app.levelPackDialogEventType = SDL_RegisterEvents(1);
    if (app.levelPackDialogEventType == static_cast<Uint32>(-1)) {
        SDL_Log("Unable to reserve a level-pack dialog event type: %s", SDL_GetError());
        app.levelPackDialogEventType = SDL_EVENT_USER;
    }
    (void)app.audio.initialize();
    if (!SDL_CreateWindowAndRenderer("Chip's Challenge — Native Remake",
                                     1280, 820,
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
                                     &app.window, &app.renderer)) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }
    openFirstGamepad(app);
    // The menu and campaign grid need this much logical client area to remain
    // fully visible while the window is resized. SDL applies these dimensions
    // in window coordinates, allowing high-DPI output to scale them normally.
    if (!SDL_SetWindowMinimumSize(app.window, 640, 480))
        SDL_Log("Unable to set minimum window size: %s", SDL_GetError());
    const auto* basePath = SDL_GetBasePath();
    const auto resourceRoot = pathFromUtf8(basePath ? basePath : "");
    const auto iconPath = resourceRoot / "assets" / "art" / "app-icon-polished.png";
    const auto iconPathUtf8 = pathToUtf8(iconPath);
    SDL_Surface* iconSurface = SDL_LoadPNG(iconPathUtf8.c_str());
    if (iconSurface) {
        SDL_SetWindowIcon(app.window, iconSurface);
        app.appIcon = SDL_CreateTextureFromSurface(app.renderer, iconSurface);
        SDL_DestroySurface(iconSurface);
        if (!app.appIcon) SDL_Log("Unable to create polished app icon texture: %s", SDL_GetError());
        else {
            SDL_SetTextureScaleMode(app.appIcon, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(app.appIcon, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("Unable to load polished app icon %s: %s", iconPathUtf8.c_str(), SDL_GetError());
    }
    const auto menuBackgroundPath = resourceRoot / "assets" / "art" / "menu-background.png";
    const auto menuBackgroundPathUtf8 = pathToUtf8(menuBackgroundPath);
    SDL_Surface* menuBackgroundSurface = SDL_LoadPNG(menuBackgroundPathUtf8.c_str());
    if (!menuBackgroundSurface) {
        SDL_Log("Unable to load original menu background %s: %s",
                menuBackgroundPathUtf8.c_str(), SDL_GetError());
    } else {
        app.menuBackground = SDL_CreateTextureFromSurface(app.renderer, menuBackgroundSurface);
        SDL_DestroySurface(menuBackgroundSurface);
        if (!app.menuBackground) {
            SDL_Log("Unable to create original menu background texture: %s", SDL_GetError());
        } else {
            SDL_SetTextureScaleMode(app.menuBackground, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(app.menuBackground, SDL_BLENDMODE_BLEND);
        }
    }
    const auto victoryIllustrationPath = resourceRoot / "assets" / "art" / "victory-illustration.png";
    const auto victoryIllustrationPathUtf8 = pathToUtf8(victoryIllustrationPath);
    SDL_Surface* victoryIllustrationSurface = SDL_LoadPNG(victoryIllustrationPathUtf8.c_str());
    if (!victoryIllustrationSurface) {
        SDL_Log("Unable to load original victory illustration %s: %s",
                victoryIllustrationPathUtf8.c_str(), SDL_GetError());
    } else {
        app.victoryIllustration = SDL_CreateTextureFromSurface(app.renderer, victoryIllustrationSurface);
        SDL_DestroySurface(victoryIllustrationSurface);
        if (!app.victoryIllustration) {
            SDL_Log("Unable to create original victory illustration texture: %s", SDL_GetError());
        } else {
            SDL_SetTextureScaleMode(app.victoryIllustration, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(app.victoryIllustration, SDL_BLENDMODE_BLEND);
        }
    }
    const auto originalSpritesPath = resourceRoot / "assets" / "art" / "original-transparent-sprites.png";
    const auto originalSpritesPathUtf8 = pathToUtf8(originalSpritesPath);
    SDL_Surface* originalSpritesSurface = SDL_LoadPNG(originalSpritesPathUtf8.c_str());
    if (!originalSpritesSurface) {
        SDL_Log("Unable to load original transparent sprite sheet %s: %s",
                originalSpritesPathUtf8.c_str(), SDL_GetError());
    } else {
        app.originalTransparentSprites = SDL_CreateTextureFromSurface(app.renderer, originalSpritesSurface);
        SDL_DestroySurface(originalSpritesSurface);
        if (!app.originalTransparentSprites) {
            SDL_Log("Unable to create original transparent sprite texture: %s", SDL_GetError());
        } else {
            SDL_SetTextureScaleMode(app.originalTransparentSprites, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(app.originalTransparentSprites, SDL_BLENDMODE_BLEND);
        }
    }
    const auto loadOptionalSpriteSheet = [&](const char* filename, SDL_Texture*& destination) {
        const auto path = resourceRoot / "assets" / "art" / filename;
        const auto utf8 = pathToUtf8(path);
        SDL_Surface* surface = SDL_LoadPNG(utf8.c_str());
        if (!surface) {
            SDL_Log("Optional cleaned sprite sheet %s unavailable; original art will be used: %s",
                    utf8.c_str(), SDL_GetError());
            return;
        }
        destination = SDL_CreateTextureFromSurface(app.renderer, surface);
        SDL_DestroySurface(surface);
        if (!destination) {
            SDL_Log("Unable to create cleaned sprite texture %s; original art will be used: %s",
                    utf8.c_str(), SDL_GetError());
            return;
        }
        SDL_SetTextureScaleMode(destination, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(destination, SDL_BLENDMODE_BLEND);
    };
    loadOptionalSpriteSheet("player-poses-polished.png", app.polishedPlayerLandPoses);
    loadOptionalSpriteSheet("player-swim-poses-polished.png", app.polishedPlayerSwimPoses);
    loadOptionalSpriteSheet("creature-sprites-polished.png", app.cleanedCreatureSprites);
    loadOptionalSpriteSheet("key-boot-icons-polished.png", app.cleanedKeyBootIcons);
    loadOptionalSpriteSheet("scoreboard-panel.png", app.scoreboardPanel);
    SDL_SetRenderVSync(app.renderer, 1);
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
    const auto atlasPath = resourceRoot / "assets" / "art" / "tile-atlas.png";
    const auto atlasPathUtf8 = pathToUtf8(atlasPath);
    SDL_Surface* atlasSurface = SDL_LoadPNG(atlasPathUtf8.c_str());
    if (!atlasSurface) {
        SDL_Log("Unable to load original tile atlas %s: %s",
                atlasPathUtf8.c_str(), SDL_GetError());
    } else {
        app.atlas = SDL_CreateTextureFromSurface(app.renderer, atlasSurface);
        SDL_DestroySurface(atlasSurface);
        if (!app.atlas) SDL_Log("Unable to create tile atlas texture: %s", SDL_GetError());
        else {
            SDL_SetTextureScaleMode(app.atlas, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(app.atlas, SDL_BLENDMODE_BLEND);
        }
    }
    const auto digitsPath = resourceRoot / "assets" / "art" / "digital-digits.png";
    const auto digitsPathUtf8 = pathToUtf8(digitsPath);
    SDL_Surface* digitsSurface = SDL_LoadPNG(digitsPathUtf8.c_str());
    if (!digitsSurface) {
        SDL_Log("Unable to load original digital scoreboard digits %s: %s",
                digitsPathUtf8.c_str(), SDL_GetError());
    } else {
        app.digitalDigits = SDL_CreateTextureFromSurface(app.renderer, digitsSurface);
        SDL_DestroySurface(digitsSurface);
        if (!app.digitalDigits) SDL_Log("Unable to create digital digit texture: %s", SDL_GetError());
        else {
            SDL_SetTextureScaleMode(app.digitalDigits, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(app.digitalDigits, SDL_BLENDMODE_BLEND);
        }
    }
    app.lastTick = SDL_GetTicks();
    return true;
}

void loadCampaignProfile(App& app) {
    char* preferencePath = SDL_GetPrefPath("ChipsChallenge", "NativeRemake");
    if (!preferencePath) {
        app.profileMessage = "Campaign progress storage is unavailable.";
        SDL_Log("Cannot locate per-user save directory: %s", SDL_GetError());
        app.profile.completed.assign(app.levels.size(), 0);
        app.profile.bestTimes.assign(app.levels.size(), 0xFFFFFFFFU);
        return;
    }
    app.profilePath = pathFromUtf8(preferencePath) / app.profileFileName;
    SDL_free(preferencePath);
    app.profile = chips::loadProfile(app.profilePath, &app.profileMessage, app.levels.size());
    app.audio.setVolume(static_cast<float>(app.profile.effectsVolumePercent) / 100.0F);
    app.audio.setMuted(app.profile.effectsMuted);
    app.audio.setMusicVolume(static_cast<float>(app.profile.musicVolumePercent) / 100.0F);
    app.audio.setMusicMuted(app.profile.musicMuted);
    const bool requestedFullscreen = app.profile.fullscreen;
    if (!SDL_SetWindowFullscreen(app.window, requestedFullscreen)) {
        SDL_Log("Unable to restore saved display mode: %s", SDL_GetError());
        app.fullscreen = (SDL_GetWindowFlags(app.window) & SDL_WINDOW_FULLSCREEN) != 0;
        app.profile.fullscreen = app.fullscreen;
        app.profileMessage = "Saved display mode could not be restored.";
    } else {
        app.fullscreen = requestedFullscreen;
    }
    if (!app.fullscreen)
        SDL_SetWindowSize(app.window, app.profile.windowWidth, app.profile.windowHeight);
    app.levelIndex = std::min<std::size_t>(app.profile.lastSelectedLevel - 1, app.levels.size() - 1);
    resetPlayer(app);
    const auto& activeTitle = app.levels[app.levelIndex].title;
    SDL_SetWindowTitle(app.window, (std::string("Chip's Challenge: ") +
        (activeTitle.empty() ? "Untitled" : activeTitle)).c_str());
    if (!app.profileMessage.empty()) SDL_Log("%s", app.profileMessage.c_str());
}

std::string customPackProfileFileName(const std::filesystem::path& packPath) {
    std::ifstream file(packPath, std::ios::binary);
    if (!file) throw chips::LevelPackError("Cannot open level pack: " + packPath.string());
    // FNV-1a gives a stable, dependency-free identity for the exact pack bytes.
    std::uint64_t hash = 14695981039346656037ULL;
    char byte{};
    while (file.get(byte)) {
        hash ^= static_cast<std::uint8_t>(static_cast<unsigned char>(byte));
        hash *= 1099511628211ULL;
    }
    if (!file.eof()) throw chips::LevelPackError("Could not read level pack: " + packPath.string());
    std::ostringstream name;
    name << "pack-" << std::hex << std::nouppercase << std::setw(16) << std::setfill('0')
         << hash << ".ccprofile";
    return name.str();
}

void loadLevelPackIntoApp(App& app, const std::filesystem::path& packPath) {
    auto levels = chips::loadLevelPack(packPath);
    if (levels.empty()) throw chips::LevelPackError("Level pack contains no levels");
    auto profileFileName = customPackProfileFileName(packPath);
    if (!app.profilePath.empty()) {
        chips::saveProfile(app.profilePath, app.profile);
        app.displayProfileDirty = false;
    }
    const auto tickPhase = app.game.gameTicks;
    const auto randomPhase = app.game.randomState;
    app.levels = std::move(levels);
    app.profileFileName = std::move(profileFileName);
    app.profilePath.clear();
    app.profileMessage.clear();
    app.profile = {};
    app.levelIndex = 0;
    app.selectedLevel = 0;
    app.game = chips::GameState::start(app.levels.front(), tickPhase, randomPhase);
    app.mainMenuVisible = false;
    app.paused = false;
    app.helpVisible = false;
    app.debugVisible = false;
    app.bestTimesVisible = false;
    app.settingsVisible = false;
    app.levelSelect = false;
    app.passwordVisible = false;
    app.winRecorded = false;
    app.playerAnimationFrom = -1;
    app.playerAnimationTo = -1;
    app.playerAnimationTeleported = false;
    app.creatureAnimations.clear();
    loadCampaignProfile(app);
    if (app.profileMessage.empty())
        app.profileMessage = "Loaded level pack (" + std::to_string(app.levels.size()) + " levels).";
    app.lastTick = SDL_GetTicks();
}

void SDLCALL levelPackDialogCallback(void* userdata, const char* const* filelist, int) {
    auto* app = static_cast<App*>(userdata);
    constexpr Sint32 selectedFile = 0;
    constexpr Sint32 dialogError = 1;
    constexpr Sint32 dialogCancelled = 2;
    char* result = nullptr;
    Sint32 resultCode = dialogCancelled;
    if (!filelist) {
        resultCode = dialogError;
        result = SDL_strdup(SDL_GetError());
    } else if (filelist[0]) {
        resultCode = selectedFile;
        result = SDL_strdup(filelist[0]);
    }
    SDL_Event event{};
    event.type = app->levelPackDialogEventType;
    event.user.code = resultCode;
    event.user.data1 = result;
    if (!SDL_PushEvent(&event)) {
        SDL_free(result);
        app->levelPackDialogEventPushFailed.store(true, std::memory_order_release);
    }
}

void requestLevelPackOpen(App& app) {
    if (app.levelPackDialogPending) return;
    static constexpr std::array<SDL_DialogFileFilter, 2> filters{{
        {"Chip's Challenge level packs", "dat;DAT"},
        {"All files", "*"}
    }};
    app.levelPackDialogPending = true;
    app.exitAfterFileDialog = false;
    app.lastTick = SDL_GetTicks();
    SDL_ShowOpenFileDialog(levelPackDialogCallback, &app, app.window,
                           filters.data(), static_cast<int>(filters.size()), nullptr, false);
}

void persistProfile(App& app, bool announce) {
    if (app.profilePath.empty()) return;
    try {
        chips::saveProfile(app.profilePath, app.profile);
        if (announce) app.profileMessage = "Progress saved.";
    } catch (const std::exception& error) {
        app.profileMessage = "Progress could not be saved.";
        SDL_Log("Campaign progress save failed: %s", error.what());
    }
}

void toggleFullscreen(App& app) {
    const bool requestedFullscreen = !app.fullscreen;
    if (!SDL_SetWindowFullscreen(app.window, requestedFullscreen)) {
        SDL_Log("Unable to change fullscreen mode: %s", SDL_GetError());
        app.profileMessage = "Unable to change display mode.";
        return;
    }
    app.fullscreen = requestedFullscreen;
    app.profile.fullscreen = requestedFullscreen;
    persistProfile(app);
}

void recordLevelWin(App& app) {
    if (app.winRecorded || app.game.outcome != chips::Outcome::won) return;
    app.winRecorded = true;
    const auto index = app.levelIndex;
    if (index >= app.levels.size()) return;
    const auto& level = app.levels[index];
    app.levelTimeBonus = static_cast<std::uint64_t>(app.game.secondsLeft) * 10U;
    auto completionBonus = static_cast<std::uint64_t>(level.number) * 500U;
    for (unsigned restart = 0; restart < app.levelRestartCount; ++restart) {
        completionBonus = completionBonus * 4U / 5U;
        if (completionBonus < 500U) break;
    }
    app.levelCompletionBonus = completionBonus;
    app.levelCompletionScore = app.levelTimeBonus + app.levelCompletionBonus;
    app.totalScore += app.levelCompletionScore;
    const auto previousBest = app.profile.bestTimes[index];
    app.timeRecordEstablished = level.timeLimit > 0 && previousBest == 0xFFFFFFFFU;
    app.timeRecordImproved = level.timeLimit > 0 && previousBest != 0xFFFFFFFFU &&
                             app.game.secondsElapsed < previousBest;
    if (app.timeRecordImproved)
        app.timeRecordImprovementSeconds = previousBest - app.game.secondsElapsed;
    if (chips::recordLevelCompletion(app.profile, index, app.levels.size(),
                                     level.timeLimit > 0, app.game.secondsElapsed))
        persistProfile(app);
}

void handleLevelSelectClick(App& app, float windowX, float windowY) {
    int physicalWidth = 0, physicalHeight = 0, windowWidth = 0, windowHeight = 0;
    SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
    SDL_GetWindowSize(app.window, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0) return;
    const float uiScale = uiScaleForOutput(physicalWidth, physicalHeight, app.profile.uiScalePercent);
    const int outputWidth = std::max(1, static_cast<int>(static_cast<float>(physicalWidth) / uiScale));
    const int outputHeight = std::max(1, static_cast<int>(static_cast<float>(physicalHeight) / uiScale));
    const float x = windowX * static_cast<float>(physicalWidth) /
                    static_cast<float>(windowWidth) / uiScale;
    const float y = windowY * static_cast<float>(physicalHeight) /
                    static_cast<float>(windowHeight) / uiScale;
    const auto grid = levelGridLayout(outputWidth, outputHeight);
    const auto pageStart = (app.selectedLevel / kLevelSelectorPageSize) * kLevelSelectorPageSize;
    const auto pageCount = (app.levels.size() + kLevelSelectorPageSize - 1) / kLevelSelectorPageSize;
    if (pageCount > 1) {
        if (x >= grid.previousPageButton.x && x < grid.previousPageButton.x + grid.previousPageButton.w &&
            y >= grid.previousPageButton.y && y < grid.previousPageButton.y + grid.previousPageButton.h) {
            changeLevelSelectPage(app, -1);
            return;
        }
        if (x >= grid.nextPageButton.x && x < grid.nextPageButton.x + grid.nextPageButton.w &&
            y >= grid.nextPageButton.y && y < grid.nextPageButton.y + grid.nextPageButton.h) {
            changeLevelSelectPage(app, 1);
            return;
        }
    }
    const int column = static_cast<int>((x - grid.gridLeft) / grid.cell);
    const int row = static_cast<int>((y - grid.gridTop) / grid.cell);
    if (column < 0 || column >= 13 || row < 0 || row >= 12) return;
    const auto index = pageStart + static_cast<std::size_t>(row) * kLevelSelectorColumns +
                       static_cast<std::size_t>(column);
    if (index < app.levels.size())
        requestLevelSelection(app, index);
}

void handleMainMenuPointer(App& app, float windowX, float windowY, bool activate) {
    int physicalWidth = 0, physicalHeight = 0, windowWidth = 0, windowHeight = 0;
    SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
    SDL_GetWindowSize(app.window, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0) return;
    const float uiScale = uiScaleForOutput(physicalWidth, physicalHeight, app.profile.uiScalePercent);
    const int outputWidth = std::max(1, static_cast<int>(static_cast<float>(physicalWidth) / uiScale));
    const int outputHeight = std::max(1, static_cast<int>(static_cast<float>(physicalHeight) / uiScale));
    const float x = windowX * static_cast<float>(physicalWidth) /
                    static_cast<float>(windowWidth) / uiScale;
    const float y = windowY * static_cast<float>(physicalHeight) /
                    static_cast<float>(windowHeight) / uiScale;
    const float panelWidth = std::min(520.0F, static_cast<float>(outputWidth) - 40.0F);
    const float panelHeight = 356.0F;
    const float panelLeft = (static_cast<float>(outputWidth) - panelWidth) * 0.5F;
    const float panelTop = (static_cast<float>(outputHeight) - panelHeight) * 0.5F;
    if (x < panelLeft + 20.0F || x >= panelLeft + panelWidth - 20.0F) return;

    for (std::size_t i = 0; i < 7; ++i) {
        const float rowTop = panelTop + 100.0F + static_cast<float>(i) * 32.0F - 5.0F;
        if (y >= rowTop && y < rowTop + 28.0F) {
            app.menuSelection = i;
            if (activate) activateMenuSelection(app);
            return;
        }
    }
}

void handleSettingsPointer(App& app, float windowX, float windowY) {
    if (app.waitingForBinding >= 0) return;
    int physicalWidth = 0, physicalHeight = 0, windowWidth = 0, windowHeight = 0;
    SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
    SDL_GetWindowSize(app.window, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0) return;
    const float scale = uiScaleForOutput(physicalWidth, physicalHeight, app.profile.uiScalePercent);
    const float x = windowX * static_cast<float>(physicalWidth) /
                    static_cast<float>(windowWidth) / scale;
    const float y = windowY * static_cast<float>(physicalHeight) /
                    static_cast<float>(windowHeight) / scale;
    const int outputWidth = std::max(1, static_cast<int>(static_cast<float>(physicalWidth) / scale));
    const int outputHeight = std::max(1, static_cast<int>(static_cast<float>(physicalHeight) / scale));
    const auto layout = settingsOverlayLayout(outputWidth, outputHeight);
    if (x >= layout.effectsToggle.x && x <= layout.effectsToggle.x + layout.effectsToggle.w &&
        y >= layout.effectsToggle.y && y <= layout.effectsToggle.y + layout.effectsToggle.h) {
        app.profile.effectsMuted = !app.profile.effectsMuted;
        app.audio.setMuted(app.profile.effectsMuted);
        persistProfile(app);
        return;
    }
    if (x >= layout.musicToggle.x && x <= layout.musicToggle.x + layout.musicToggle.w &&
        y >= layout.musicToggle.y && y <= layout.musicToggle.y + layout.musicToggle.h) {
        app.profile.musicMuted = !app.profile.musicMuted;
        app.audio.setMusicMuted(app.profile.musicMuted);
        persistProfile(app);
        return;
    }
    if (x >= layout.motionToggle.x && x <= layout.motionToggle.x + layout.motionToggle.w &&
        y >= layout.motionToggle.y && y <= layout.motionToggle.y + layout.motionToggle.h) {
        app.profile.reducedMotion = !app.profile.reducedMotion;
        persistProfile(app);
        return;
    }
    if (x >= layout.fullscreenToggle.x && x < layout.fullscreenToggle.x + layout.fullscreenToggle.w &&
        y >= layout.fullscreenToggle.y && y < layout.fullscreenToggle.y + layout.fullscreenToggle.h) {
        toggleFullscreen(app);
        return;
    }
    const auto insideSlider = [x, y](const SDL_FRect& slider) {
        return x >= slider.x - 4.0F && x <= slider.x + slider.w + 4.0F &&
               y >= slider.y - 6.0F && y <= slider.y + slider.h + 6.0F;
    };
    if (insideSlider(layout.effectsSlider)) {
        const float fraction = std::clamp((x - layout.effectsSlider.x) / layout.effectsSlider.w,
                                          0.0F, 1.0F);
        const auto percent = static_cast<unsigned>(std::lround(fraction * 20.0F)) * 5U;
        app.profile.effectsVolumePercent = static_cast<std::uint8_t>(std::min(percent, 100U));
        app.audio.setVolume(static_cast<float>(app.profile.effectsVolumePercent) / 100.0F);
        persistProfile(app);
        return;
    }
    if (insideSlider(layout.musicSlider)) {
        const float fraction = std::clamp((x - layout.musicSlider.x) / layout.musicSlider.w,
                                          0.0F, 1.0F);
        const auto percent = static_cast<unsigned>(std::lround(fraction * 20.0F)) * 5U;
        app.profile.musicVolumePercent = static_cast<std::uint8_t>(std::min(percent, 100U));
        app.audio.setMusicVolume(static_cast<float>(app.profile.musicVolumePercent) / 100.0F);
        persistProfile(app);
        return;
    }
    if (insideSlider(layout.scaleSlider)) {
        const float fraction = std::clamp((x - layout.scaleSlider.x) / layout.scaleSlider.w,
                                          0.0F, 1.0F);
        const auto step = static_cast<unsigned>(std::lround(fraction * 4.0F));
        app.profile.uiScalePercent = static_cast<std::uint16_t>(100U + step * 25U);
        persistProfile(app);
    }
}

bool pauseActionsVisible(const App& app) {
    return app.paused && !app.helpVisible && !app.settingsVisible && !app.bestTimesVisible;
}

bool resultActionsVisible(const App& app) {
    return app.game.outcome != chips::Outcome::playing && !app.paused && !app.helpVisible &&
           !app.bestTimesVisible && !app.settingsVisible && !app.levelSelect && !app.mainMenuVisible;
}

std::size_t resultActionCount(const App& app) {
    return app.game.outcome == chips::Outcome::won ? 1U : 2U;
}

void activatePauseAction(App& app, std::size_t selection) {
    if (selection == 0) {
        setPaused(app, false);
    } else if (selection == 1) {
        ++app.levelRestartCount;
        resetPlayer(app);
        setPaused(app, false);
    } else if (selection == 2) {
        app.helpVisible = true;
    }
}

void activateResultAction(App& app, std::size_t selection) {
    if (!resultActionsVisible(app)) return;
    const bool won = app.game.outcome == chips::Outcome::won;
    if (won && selection == 0) {
        advanceAfterWin(app);
    } else if (!won && selection == 0) {
        ++app.levelRestartCount;
        resetPlayer(app);
    } else if (!won && selection == 1) {
        app.paused = false;
        app.helpVisible = false;
        app.mainMenuVisible = true;
        app.menuSelection = 0;
    }
}

void handlePauseAndResultPointer(App& app, float windowX, float windowY, bool activate) {
    int physicalWidth = 0, physicalHeight = 0, windowWidth = 0, windowHeight = 0;
    SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
    SDL_GetWindowSize(app.window, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0) return;
    const float scale = uiScaleForOutput(physicalWidth, physicalHeight, app.profile.uiScalePercent);
    const float x = windowX * static_cast<float>(physicalWidth) /
                    static_cast<float>(windowWidth) / scale;
    const float y = windowY * static_cast<float>(physicalHeight) /
                    static_cast<float>(windowHeight) / scale;
    const int outputWidth = std::max(1, static_cast<int>(static_cast<float>(physicalWidth) / scale));
    const int outputHeight = std::max(1, static_cast<int>(static_cast<float>(physicalHeight) / scale));
    const auto inside = [x, y](const SDL_FRect& bounds) {
        return x >= bounds.x && x < bounds.x + bounds.w &&
               y >= bounds.y && y < bounds.y + bounds.h;
    };
    if (pauseActionsVisible(app)) {
        const auto layout = chips_native::pauseOverlayLayout(outputWidth, outputHeight);
        for (std::size_t i = 0; i < layout.buttonCount; ++i) {
            if (!inside(layout.buttons[i])) continue;
            app.actionSelection = static_cast<int>(i);
            if (activate) activatePauseAction(app, i);
            return;
        }
        return;
    }
    if (!resultActionsVisible(app)) return;
    const bool won = app.game.outcome == chips::Outcome::won;
    const auto layout = chips_native::resultOverlayLayout(outputWidth, outputHeight, won);
    for (std::size_t i = 0; i < layout.buttonCount; ++i) {
        if (!inside(layout.buttons[i])) continue;
        app.actionSelection = static_cast<int>(i);
        if (activate) activateResultAction(app, i);
        return;
    }
}

void activateTopMenuItem(App& app, int menu, int row) {
    if (menu < 0 || menu > 3 || row < 0 || row >= topMenuItemCount(menu)) return;
    app.topMenuOpen = -1;
    app.topMenuSelection = -1;
    app.levelSelect = false;
    app.helpVisible = false;
    app.bestTimesVisible = false;
    app.settingsVisible = false;
    app.passwordVisible = false;
    if (menu == 0) {
        if (row == 0) {
            app.paused = false;
            app.totalScore = 0;
            selectLevel(app, 0);
        } else if (row == 1) {
            app.mainMenuVisible = false;
            app.bestTimesVisible = false;
            togglePause(app);
        } else if (row == 2) {
            app.mainMenuVisible = false;
            app.paused = false;
            app.bestTimesVisible = true;
        } else {
            app.running = false;
        }
    } else if (menu == 1) {
        app.mainMenuVisible = false;
        app.settingsVisible = true;
    } else if (menu == 2) {
        app.mainMenuVisible = false;
        if (row == 0) {
            app.levelSelect = true;
            app.selectedLevel = app.levelIndex;
        } else if (row == 1) {
            app.passwordVisible = true;
            app.passwordInput.clear();
            app.passwordMessage.clear();
            (void)SDL_StartTextInput(app.window);
        } else {
            requestLevelPackOpen(app);
        }
    } else {
        app.mainMenuVisible = false;
        app.helpVisible = true;
    }
}

void handleGamepadDirection(App& app, chips::Direction direction) {
    if (app.debugVisible || app.levelPackDialogPending) return;
    if (app.passwordVisible || app.bestTimesVisible) return;
    if (app.mainMenuVisible && !app.helpVisible && !app.settingsVisible && !app.levelSelect) {
        if (direction == chips::Direction::north)
            app.menuSelection = (app.menuSelection + 6) % 7;
        else if (direction == chips::Direction::south)
            app.menuSelection = (app.menuSelection + 1) % 7;
        return;
    }
    if (app.levelSelect) {
        switch (direction) {
            case chips::Direction::north:
                app.selectedLevel = app.selectedLevel >= 13 ? app.selectedLevel - 13 : app.selectedLevel;
                break;
            case chips::Direction::east:
                app.selectedLevel = std::min(app.selectedLevel + 1, app.levels.size() - 1);
                break;
            case chips::Direction::south:
                app.selectedLevel = std::min(app.selectedLevel + 13, app.levels.size() - 1);
                break;
            case chips::Direction::west:
                app.selectedLevel = app.selectedLevel > 0 ? app.selectedLevel - 1 : 0;
                break;
        }
        return;
    }
    if (pauseActionsVisible(app)) {
        const int count = 3;
        const int delta = direction == chips::Direction::north || direction == chips::Direction::west ? -1 : 1;
        app.actionSelection = (app.actionSelection + delta + count) % count;
        return;
    }
    if (resultActionsVisible(app)) {
        const int count = static_cast<int>(resultActionCount(app));
        const int delta = direction == chips::Direction::north || direction == chips::Direction::west ? -1 : 1;
        app.actionSelection = (app.actionSelection + delta + count) % count;
        return;
    }
    if (!app.paused && !app.helpVisible && !app.settingsVisible && !app.mainMenuVisible)
        movePlayer(app, direction);
}

void handleGamepadAxis(App& app, SDL_GamepadAxis axis) {
    if (!app.gamepad || (axis != SDL_GAMEPAD_AXIS_LEFTX && axis != SDL_GAMEPAD_AXIS_LEFTY)) return;
    const int x = SDL_GetGamepadAxis(app.gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    const int y = SDL_GetGamepadAxis(app.gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    const int magnitudeX = std::abs(x);
    const int magnitudeY = std::abs(y);
    constexpr int activationThreshold = 16000;
    constexpr int releaseThreshold = 8000;
    constexpr int axisSwitchMargin = 6000;
    if (app.paused && !pauseActionsVisible(app)) {
        app.gamepadStickDirection = -1;
        app.gamepadStickNextRepeat = 0;
        app.gamepadStickIgnoreHeld = std::max(magnitudeX, magnitudeY) >= releaseThreshold;
        return;
    }
    if (app.gamepadStickIgnoreHeld) {
        if (std::max(magnitudeX, magnitudeY) < releaseThreshold)
            app.gamepadStickIgnoreHeld = false;
        app.gamepadStickDirection = -1;
        app.gamepadStickNextRepeat = 0;
        return;
    }
    int direction = -1;
    if (std::max(magnitudeX, magnitudeY) >= activationThreshold) {
        if (magnitudeX > magnitudeY)
            direction = x > 0 ? static_cast<int>(chips::Direction::east)
                              : static_cast<int>(chips::Direction::west);
        else
            direction = y > 0 ? static_cast<int>(chips::Direction::south)
                              : static_cast<int>(chips::Direction::north);
    } else if (std::max(magnitudeX, magnitudeY) < releaseThreshold) {
        app.gamepadStickDirection = -1;
        return;
    }
    if (direction >= 0 && app.gamepadStickDirection >= 0) {
        const auto current = static_cast<chips::Direction>(app.gamepadStickDirection);
        const bool currentVertical = current == chips::Direction::north || current == chips::Direction::south;
        const bool candidateVertical = direction == static_cast<int>(chips::Direction::north) ||
                                       direction == static_cast<int>(chips::Direction::south);
        const int currentAxis = currentVertical ? y : x;
        const int currentMagnitude = currentVertical ? magnitudeY : magnitudeX;
        const bool matchingPolarity = currentVertical
            ? ((current == chips::Direction::south) == (currentAxis > 0))
            : ((current == chips::Direction::east) == (currentAxis > 0));
        const int candidateMagnitude = candidateVertical ? magnitudeY : magnitudeX;
        if (currentVertical != candidateVertical && matchingPolarity &&
            currentMagnitude >= releaseThreshold &&
            candidateMagnitude < currentMagnitude + axisSwitchMargin)
            direction = app.gamepadStickDirection;
    }
    if (direction < 0 || direction == app.gamepadStickDirection) return;
    app.gamepadStickDirection = direction;
    handleGamepadDirection(app, static_cast<chips::Direction>(direction));
    app.gamepadStickNextRepeat = SDL_GetTicks() + 300;
}

int heldGamepadDpadDirection(SDL_Gamepad* gamepad) {
    if (!gamepad) return -1;
    if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP))
        return static_cast<int>(chips::Direction::north);
    if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
        return static_cast<int>(chips::Direction::east);
    if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
        return static_cast<int>(chips::Direction::south);
    if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
        return static_cast<int>(chips::Direction::west);
    return -1;
}

void handleGamepadButton(App& app, SDL_GamepadButton button) {
    if (app.debugVisible) {
        if (button == SDL_GAMEPAD_BUTTON_EAST) app.debugVisible = false;
        return;
    }
    if (app.levelPlacardVisible && !app.mainMenuVisible) {
        app.levelPlacardVisible = false;
        app.lastTick = SDL_GetTicks();
    }
    if (app.bestTimesVisible && button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) {
        changeBestTimesPage(app, -1);
        return;
    }
    if (app.bestTimesVisible && button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
        changeBestTimesPage(app, 1);
        return;
    }
    if (app.levelSelect && button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) {
        changeLevelSelectPage(app, -1);
        return;
    }
    if (app.levelSelect && button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
        changeLevelSelectPage(app, 1);
        return;
    }
    if (button == SDL_GAMEPAD_BUTTON_SOUTH) {
        if (app.bestTimesVisible) app.bestTimesVisible = false;
        else if (app.mainMenuVisible && !app.levelSelect && !app.helpVisible && !app.settingsVisible)
            activateMenuSelection(app);
        else if (app.levelSelect) requestLevelSelection(app, app.selectedLevel);
        else if (pauseActionsVisible(app))
            activatePauseAction(app, static_cast<std::size_t>(std::clamp(app.actionSelection, 0, 2)));
        else if (resultActionsVisible(app))
            activateResultAction(app, static_cast<std::size_t>(std::clamp(
                app.actionSelection, 0, static_cast<int>(resultActionCount(app)) - 1)));
        else if (!app.paused && !app.helpVisible && !app.settingsVisible && !app.mainMenuVisible) {
            if (app.game.outcome == chips::Outcome::won) advanceAfterWin(app);
            else if (app.game.outcome != chips::Outcome::playing) {
                ++app.levelRestartCount;
                resetPlayer(app);
            }
        }
        return;
    }
    if (button == SDL_GAMEPAD_BUTTON_EAST) {
        if (app.bestTimesVisible) app.bestTimesVisible = false;
        else if (app.passwordVisible) closePasswordEntry(app);
        else if (app.helpVisible) app.helpVisible = false;
        else if (app.levelSelect) app.levelSelect = false;
        else if (app.settingsVisible && app.waitingForBinding >= 0) {
            app.waitingForBinding = -1;
            app.profileMessage = "Movement key binding canceled.";
        } else if (app.settingsVisible) app.settingsVisible = false;
        else if (app.paused) setPaused(app, false);
        else if (!app.mainMenuVisible) setPaused(app, true);
        return;
    }
    if (button == SDL_GAMEPAD_BUTTON_START && !app.mainMenuVisible && !app.levelSelect &&
        !app.helpVisible && !app.bestTimesVisible && !app.settingsVisible && !app.passwordVisible)
        togglePause(app);
}

void run(App& app) {
    while (app.running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == app.levelPackDialogEventType) {
                const auto resultCode = event.user.code;
                auto* result = static_cast<char*>(event.user.data1);
                app.levelPackDialogPending = false;
                if (!app.exitAfterFileDialog && resultCode == 0 && result) {
                    try {
                        loadLevelPackIntoApp(app, pathFromUtf8(result));
                    } catch (const std::exception& error) {
                        app.profileMessage = std::string("Could not load level pack: ") + error.what();
                        SDL_Log("Level-pack load failed: %s", error.what());
                    }
                } else if (!app.exitAfterFileDialog && resultCode == 1) {
                    app.profileMessage = std::string("File dialog failed: ") +
                        (result ? result : "unknown error");
                    SDL_Log("Level-pack file dialog failed: %s", result ? result : "unknown error");
                }
                SDL_free(result);
                app.lastTick = SDL_GetTicks();
                if (app.exitAfterFileDialog) app.running = false;
                continue;
            }
            if (event.type == SDL_EVENT_QUIT) {
                if (app.levelPackDialogPending) app.exitAfterFileDialog = true;
                else app.running = false;
            } else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                if (app.levelPackDialogPending) app.exitAfterFileDialog = true;
                else app.running = false;
            } else if (app.levelPackDialogPending) {
                continue;
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED && !app.fullscreen &&
                       event.window.data1 >= 640 && event.window.data1 <= 8192 &&
                       event.window.data2 >= 480 && event.window.data2 <= 8192) {
                if (app.profile.windowWidth != event.window.data1 ||
                    app.profile.windowHeight != event.window.data2) {
                    app.profile.windowWidth = static_cast<std::uint16_t>(event.window.data1);
                    app.profile.windowHeight = static_cast<std::uint16_t>(event.window.data2);
                    app.displayProfileDirty = true;
                    app.displayProfileSaveAt = SDL_GetTicks() + 500;
                }
            } else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST ||
                       event.type == SDL_EVENT_WINDOW_MINIMIZED) {
                // Do not let alt-tab or another native window consume board
                // time. Stay paused after focus returns so gameplay resumes
                // only after an explicit player action.
                if (!app.mainMenuVisible && !app.paused && !app.helpVisible &&
                    !app.bestTimesVisible && !app.settingsVisible && !app.levelSelect &&
                    !app.passwordVisible)
                    setPaused(app, true);
                app.topMenuOpen = -1;
                app.topMenuSelection = -1;
                app.topMenuKeyboardFocus = false;
                app.gamepadStickDirection = -1;
                app.gamepadStickNextRepeat = 0;
                constexpr int releaseThreshold = 8000;
                const int stickX = app.gamepad ? SDL_GetGamepadAxis(app.gamepad, SDL_GAMEPAD_AXIS_LEFTX) : 0;
                const int stickY = app.gamepad ? SDL_GetGamepadAxis(app.gamepad, SDL_GAMEPAD_AXIS_LEFTY) : 0;
                app.gamepadStickIgnoreHeld = std::abs(stickX) >= releaseThreshold ||
                                             std::abs(stickY) >= releaseThreshold;
                app.gamepadDpadDirection = -1;
                app.gamepadDpadNextRepeat = 0;
                app.gamepadDpadIgnoreHeld = heldGamepadDpadDirection(app.gamepad) >= 0;
                app.game.pendingDirection.reset();
                app.lastTick = SDL_GetTicks();
            } else if (event.type == SDL_EVENT_TEXT_INPUT && app.passwordVisible) {
                if (app.passwordInput.size() < 24) app.passwordInput += event.text.text;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (app.levelPlacardVisible && !app.mainMenuVisible) {
                    app.levelPlacardVisible = false;
                    app.lastTick = SDL_GetTicks();
                }
                if (app.topMenuOpen >= 0) {
                    const int itemCount = topMenuItemCount(app.topMenuOpen);
                    const auto focusKeyboardMenu = [&app]() {
                        float mouseX = 0.0F;
                        float mouseY = 0.0F;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        app.lastMouseX = static_cast<int>(std::lround(mouseX));
                        app.lastMouseY = static_cast<int>(std::lround(mouseY));
                        app.topMenuKeyboardFocus = true;
                    };
                    if (event.key.key == SDLK_ESCAPE) {
                        app.topMenuOpen = -1;
                        app.topMenuSelection = -1;
                    } else if (event.key.key == SDLK_UP) {
                        focusKeyboardMenu();
                        app.topMenuSelection = app.topMenuSelection <= 0
                            ? itemCount - 1 : app.topMenuSelection - 1;
                    } else if (event.key.key == SDLK_DOWN) {
                        focusKeyboardMenu();
                        app.topMenuSelection = (app.topMenuSelection + 1) % itemCount;
                    } else if (event.key.key == SDLK_LEFT || event.key.key == SDLK_RIGHT) {
                        focusKeyboardMenu();
                        app.topMenuOpen = (app.topMenuOpen + (event.key.key == SDLK_LEFT ? 3 : 1)) % 4;
                        app.topMenuSelection = -1;
                    } else if ((event.key.key == SDLK_RETURN || event.key.key == SDLK_SPACE) &&
                               !event.key.repeat) {
                        activateTopMenuItem(app, app.topMenuOpen,
                            app.topMenuSelection < 0 ? 0 : app.topMenuSelection);
                    }
                    continue;
                }
                if (app.bestTimesVisible && event.key.key == SDLK_ESCAPE) {
                    app.bestTimesVisible = false;
                    continue;
                }
                if (app.bestTimesVisible) {
                    if (event.key.key == SDLK_PAGEUP || event.key.key == SDLK_LEFT)
                        changeBestTimesPage(app, -1);
                    else if (event.key.key == SDLK_PAGEDOWN || event.key.key == SDLK_RIGHT)
                        changeBestTimesPage(app, 1);
                    continue;
                }
                if (app.passwordVisible) {
                    if (event.key.key == SDLK_ESCAPE) closePasswordEntry(app);
                    else if (event.key.key == SDLK_BACKSPACE && !app.passwordInput.empty())
                        app.passwordInput.pop_back();
                    else if (event.key.key == SDLK_RETURN && !event.key.repeat)
                        submitPassword(app);
                    continue;
                }
                if (app.debugVisible) {
                    if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_F8)
                        app.debugVisible = false;
                    continue;
                }
                if (app.settingsVisible && app.waitingForBinding >= 0 &&
                    event.key.key != SDLK_ESCAPE && !event.key.repeat) {
                    const auto scancode = event.key.scancode;
                    const auto rejection = chips::validateMovementBinding(
                        static_cast<std::uint16_t>(scancode),
                        static_cast<std::size_t>(app.waitingForBinding),
                        app.profile.movementBindings, kReservedMovementScancodes, kArrowScancodes);
                    if (scancode == SDL_SCANCODE_UNKNOWN ||
                        rejection == chips::BindingRejection::reserved) {
                        app.profileMessage = "That key is reserved; choose another movement key.";
                    } else if (rejection == chips::BindingRejection::duplicate) {
                        app.profileMessage = "That key is already assigned to another direction.";
                    } else {
                        app.profile.movementBindings[static_cast<std::size_t>(app.waitingForBinding)] =
                            static_cast<std::uint16_t>(scancode);
                        app.waitingForBinding = -1;
                        app.profileMessage = "Movement key binding saved.";
                        persistProfile(app);
                    }
                    continue;
                }
                if (app.mainMenuVisible && !app.levelSelect && !app.helpVisible && !app.settingsVisible) {
                    bool handled = true;
                    switch (event.key.key) {
                        case SDLK_UP: case SDLK_W:
                            app.menuSelection = (app.menuSelection + 6) % 7;
                            break;
                        case SDLK_DOWN: case SDLK_S:
                            app.menuSelection = (app.menuSelection + 1) % 7;
                            break;
                        case SDLK_RETURN: activateMenuSelection(app); break;
                        case SDLK_ESCAPE: app.running = false; break;
                        case SDLK_F2: app.menuSelection = 1; activateMenuSelection(app); break;
                        case SDLK_L: app.menuSelection = 2; activateMenuSelection(app); break;
                        case SDLK_P: app.menuSelection = 3; activateMenuSelection(app); break;
                        case SDLK_O: app.menuSelection = 4; activateMenuSelection(app); break;
                        case SDLK_F1: app.menuSelection = 5; activateMenuSelection(app); break;
                        default: handled = false; break;
                    }
                    if (handled) continue;
                }
                if (app.levelSelect) {
                    bool handled = true;
                    switch (event.key.key) {
                        case SDLK_UP: case SDLK_W:
                            app.selectedLevel = app.selectedLevel >= 13 ? app.selectedLevel - 13 : app.selectedLevel;
                            break;
                        case SDLK_RIGHT: case SDLK_D:
                            app.selectedLevel = std::min(app.selectedLevel + 1, app.levels.size() - 1);
                            break;
                        case SDLK_DOWN: case SDLK_S:
                            app.selectedLevel = std::min(app.selectedLevel + 13, app.levels.size() - 1);
                            break;
                        case SDLK_LEFT: case SDLK_A:
                            app.selectedLevel = app.selectedLevel > 0 ? app.selectedLevel - 1 : 0;
                            break;
                        case SDLK_PAGEUP:
                            changeLevelSelectPage(app, -1);
                            break;
                        case SDLK_PAGEDOWN:
                            changeLevelSelectPage(app, 1);
                            break;
                        default: handled = false; break;
                    }
                    if (handled) continue;
                } else if (pauseActionsVisible(app)) {
                    bool handled = true;
                    switch (event.key.key) {
                        case SDLK_UP: case SDLK_W: case SDLK_LEFT: case SDLK_A:
                            handleGamepadDirection(app, chips::Direction::north);
                            break;
                        case SDLK_DOWN: case SDLK_S: case SDLK_RIGHT: case SDLK_D:
                            handleGamepadDirection(app, chips::Direction::south);
                            break;
                        case SDLK_RETURN: case SDLK_SPACE:
                            activatePauseAction(app, static_cast<std::size_t>(
                                std::clamp(app.actionSelection, 0, 2)));
                            break;
                        default: handled = false; break;
                    }
                    if (handled) continue;
                } else if (resultActionsVisible(app)) {
                    bool handled = true;
                    switch (event.key.key) {
                        case SDLK_LEFT: case SDLK_A: case SDLK_UP: case SDLK_W:
                            handleGamepadDirection(app, chips::Direction::west);
                            break;
                        case SDLK_RIGHT: case SDLK_D: case SDLK_DOWN: case SDLK_S:
                            handleGamepadDirection(app, chips::Direction::east);
                            break;
                        case SDLK_RETURN: case SDLK_SPACE:
                            activateResultAction(app, static_cast<std::size_t>(std::clamp(
                                app.actionSelection, 0, static_cast<int>(resultActionCount(app)) - 1)));
                            break;
                        default: handled = false; break;
                    }
                    if (handled) continue;
                } else if (!app.paused && !app.helpVisible && !app.bestTimesVisible &&
                           !app.settingsVisible && !app.mainMenuVisible) {
                    const int direction = movementIndexForScancode(app, event.key.scancode);
                    if (direction >= 0) {
                        movePlayer(app, static_cast<chips::Direction>(direction));
                        continue;
                    }
                }
                switch (event.key.key) {
                    case SDLK_ESCAPE:
                        if (app.helpVisible) app.helpVisible = false;
                        else if (app.levelSelect) app.levelSelect = false;
                        else if (app.settingsVisible) {
                            if (app.waitingForBinding >= 0) {
                                app.waitingForBinding = -1;
                                app.profileMessage = "Movement key binding canceled.";
                            } else {
                                app.settingsVisible = false;
                            }
                        }
                        else if (app.paused) setPaused(app, false);
                        else if (app.mainMenuVisible) app.running = false;
                        else app.mainMenuVisible = true;
                        break;
                    case SDLK_P:
                        if (!app.helpVisible && !app.settingsVisible && !app.levelSelect && !app.mainMenuVisible)
                            togglePause(app);
                        break;
                    case SDLK_F1:
                        if (app.settingsVisible) break;
                        app.levelSelect = false;
                        app.helpVisible = !app.helpVisible;
                        break;
                    case SDLK_F2: case SDLK_F3: case SDLK_F4: case SDLK_F5:
                        if (app.settingsVisible) {
                            app.waitingForBinding = static_cast<int>(event.key.key - SDLK_F2);
                            app.profileMessage = "Press a new key; Escape cancels.";
                        } else if (event.key.key == SDLK_F2) {
                            app.bestTimesVisible = false;
                            app.paused = false;
                            app.totalScore = 0;
                            selectLevel(app, 0);
                        } else if (event.key.key == SDLK_F3) {
                            app.mainMenuVisible = false;
                            togglePause(app);
                        }
                        break;
                    case SDLK_F6:
                        if (app.settingsVisible && !event.key.repeat) {
                            app.profile.reducedMotion = !app.profile.reducedMotion;
                            persistProfile(app);
                        }
                        break;
                    case SDLK_F8:
                        if (!event.key.repeat && !app.paused && !app.helpVisible &&
                            !app.bestTimesVisible && !app.settingsVisible && !app.levelSelect &&
                            !app.mainMenuVisible && !app.passwordVisible && app.topMenuOpen < 0)
                            app.debugVisible = true;
                        break;
                    case SDLK_PAGEUP: case SDLK_PAGEDOWN:
                        if (app.settingsVisible && !event.key.repeat) {
                            if (event.key.key == SDLK_PAGEUP && app.profile.uiScalePercent < 200)
                                app.profile.uiScalePercent = static_cast<std::uint16_t>(app.profile.uiScalePercent + 25);
                            else if (event.key.key == SDLK_PAGEDOWN && app.profile.uiScalePercent > 100)
                                app.profile.uiScalePercent = static_cast<std::uint16_t>(app.profile.uiScalePercent - 25);
                            persistProfile(app);
                        }
                        break;
                    case SDLK_L:
                        if (!app.paused && !app.helpVisible && !app.settingsVisible) {
                            app.levelSelect = !app.levelSelect;
                            app.selectedLevel = app.levelIndex;
                        }
                        break;
                    case SDLK_O:
                        if (!app.paused && !app.helpVisible && !app.levelSelect) {
                            app.settingsVisible = !app.settingsVisible;
                            app.waitingForBinding = -1;
                        }
                        break;
                    case SDLK_F11:
                        toggleFullscreen(app);
                        break;
                    case SDLK_M:
                        if (!event.key.repeat) {
                            app.profile.effectsMuted = !app.profile.effectsMuted;
                            app.audio.setMuted(app.profile.effectsMuted);
                            persistProfile(app);
                        }
                        break;
                    case SDLK_MINUS:
                        if (app.profile.effectsVolumePercent >= 5) {
                            app.profile.effectsVolumePercent = static_cast<std::uint8_t>(
                                app.profile.effectsVolumePercent - 5);
                            app.audio.setVolume(static_cast<float>(app.profile.effectsVolumePercent) / 100.0F);
                            persistProfile(app);
                        }
                        break;
                    case SDLK_EQUALS:
                        if (app.profile.effectsVolumePercent <= 95) {
                            app.profile.effectsVolumePercent = static_cast<std::uint8_t>(
                                app.profile.effectsVolumePercent + 5);
                            app.audio.setVolume(static_cast<float>(app.profile.effectsVolumePercent) / 100.0F);
                            persistProfile(app);
                        }
                        break;
                    case SDLK_LEFTBRACKET:
                        if (app.levelSelect) app.selectedLevel = app.selectedLevel > 0 ? app.selectedLevel - 1 : 0;
                        else if (!app.paused && !app.helpVisible && !app.settingsVisible && !app.mainMenuVisible) changeLevel(app, -1);
                        break;
                    case SDLK_RIGHTBRACKET:
                        if (app.levelSelect) app.selectedLevel = std::min(app.selectedLevel + 1, app.levels.size() - 1);
                        else if (!app.paused && !app.helpVisible && !app.settingsVisible && !app.mainMenuVisible) changeLevel(app, 1);
                        break;
                    case SDLK_R:
                        if (app.levelSelect || app.helpVisible || app.settingsVisible || app.mainMenuVisible) break;
                        ++app.levelRestartCount;
                        resetPlayer(app);
                        app.paused = false;
                        app.helpVisible = false;
                        break;
                    case SDLK_RETURN:
                        if (app.levelSelect) requestLevelSelection(app, app.selectedLevel);
                        else if (!app.paused && !app.helpVisible && !app.settingsVisible && !app.mainMenuVisible &&
                                 app.game.outcome == chips::Outcome::won)
                            advanceAfterWin(app);
                        else if (!app.paused && !app.helpVisible && !app.settingsVisible && !app.mainMenuVisible &&
                                 app.game.outcome != chips::Outcome::playing) {
                            ++app.levelRestartCount;
                            resetPlayer(app);
                        }
                        break;
                    default: break;
                }
            } else if (app.debugVisible &&
                       (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                        event.type == SDL_EVENT_MOUSE_BUTTON_UP || event.type == SDL_EVENT_MOUSE_WHEEL)) {
                // The inspector is read-only; do not let mouse input reach the board or menu.
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                const int mouseX = static_cast<int>(std::lround(event.motion.x));
                const int mouseY = static_cast<int>(std::lround(event.motion.y));
                const bool pointerMoved = mouseX != app.lastMouseX || mouseY != app.lastMouseY;
                app.lastMouseX = mouseX;
                app.lastMouseY = mouseY;
                if (app.topMenuOpen >= 0 && app.topMenuKeyboardFocus && !pointerMoved)
                    continue;
                if (pointerMoved) app.topMenuKeyboardFocus = false;
                int physicalWidth = 0, physicalHeight = 0, windowWidth = 0, windowHeight = 0;
                SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
                SDL_GetWindowSize(app.window, &windowWidth, &windowHeight);
                if (windowWidth > 0 && windowHeight > 0 && !app.passwordVisible && !app.levelSelect &&
                    !app.helpVisible) {
                    const float scale = uiScaleForOutput(physicalWidth, physicalHeight,
                                                         app.profile.uiScalePercent);
                    const float y = event.motion.y * static_cast<float>(physicalHeight) /
                                    static_cast<float>(windowHeight) / scale;
                    const float x = event.motion.x * static_cast<float>(physicalWidth) /
                                    static_cast<float>(windowWidth) / scale;
                    if (y >= 0.0F && y < 26.0F) {
                        if (app.topMenuOpen >= 0) {
                            const int hoveredMenu = x < 80.0F ? 0 : x < 170.0F ? 1 :
                                                    x < 260.0F ? 2 : 3;
                            app.topMenuOpen = hoveredMenu;
                            app.topMenuSelection = -1;
                        }
                    } else if (app.topMenuOpen >= 0) {
                        static constexpr std::array<float, 4> menuLeft{0.0F, 84.0F, 194.0F, 278.0F};
                        static constexpr std::array<float, 4> menuWidth{188.0F, 196.0F, 190.0F, 212.0F};
                        const auto menuIndex = static_cast<std::size_t>(app.topMenuOpen);
                        const std::size_t itemCount = static_cast<std::size_t>(topMenuItemCount(app.topMenuOpen));
                        if (y >= 26.0F && y < 26.0F + static_cast<float>(itemCount) * 24.0F &&
                            x >= menuLeft[menuIndex] && x < menuLeft[menuIndex] + menuWidth[menuIndex])
                            app.topMenuSelection = static_cast<int>((y - 26.0F) / 24.0F);
                        else
                            app.topMenuSelection = -1;
                    } else if (app.settingsVisible) {
                        // Slider hover does not alter settings; the value changes on click.
                    } else if (app.mainMenuVisible) {
                        handleMainMenuPointer(app, event.motion.x, event.motion.y, false);
                    } else if (pauseActionsVisible(app) || resultActionsVisible(app)) {
                        handlePauseAndResultPointer(app, event.motion.x, event.motion.y, false);
                    }
                }
            } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                float vertical = event.wheel.y;
                if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) vertical = -vertical;
                if (vertical != 0.0F) {
                    const int pageDelta = vertical < 0.0F ? 1 : -1;
                    if (app.levelSelect) changeLevelSelectPage(app, pageDelta);
                    else if (app.bestTimesVisible) changeBestTimesPage(app, pageDelta);
                }
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (app.levelPlacardVisible && !app.mainMenuVisible) {
                    app.levelPlacardVisible = false;
                    app.lastTick = SDL_GetTicks();
                }
                app.lastMouseX = static_cast<int>(std::lround(event.button.x));
                app.lastMouseY = static_cast<int>(std::lround(event.button.y));
                app.topMenuKeyboardFocus = false;
                int physicalWidth = 0, physicalHeight = 0, windowWidth = 0, windowHeight = 0;
                SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
                SDL_GetWindowSize(app.window, &windowWidth, &windowHeight);
                if (windowWidth > 0 && windowHeight > 0) {
                    const float scale = uiScaleForOutput(physicalWidth, physicalHeight, app.profile.uiScalePercent);
                    const float y = event.button.y * static_cast<float>(physicalHeight) / windowHeight / scale;
                    const float x = event.button.x * static_cast<float>(physicalWidth) / windowWidth / scale;
                    if (y >= 0.0F && y < 26.0F) {
                        const int selected = x < 80.0F ? 0 : x < 170.0F ? 1 : x < 260.0F ? 2 : 3;
                        app.topMenuOpen = app.topMenuOpen == selected ? -1 : selected;
                        app.topMenuSelection = -1;
                        continue;
                    }
                    static constexpr std::array<float, 4> menuLeft{0.0F, 84.0F, 194.0F, 278.0F};
                    static constexpr std::array<float, 4> menuWidth{188.0F, 196.0F, 190.0F, 212.0F};
                    const float openMenuBottom = app.topMenuOpen >= 0
                        ? 26.0F + static_cast<float>(topMenuItemCount(app.topMenuOpen)) * 24.0F : 26.0F;
                    if (app.topMenuOpen >= 0 && y >= 26.0F && y < openMenuBottom &&
                        x >= menuLeft[static_cast<std::size_t>(app.topMenuOpen)] &&
                        x < menuLeft[static_cast<std::size_t>(app.topMenuOpen)] + menuWidth[static_cast<std::size_t>(app.topMenuOpen)]) {
                        const int row = static_cast<int>((y - 26.0F) / 24.0F);
                        activateTopMenuItem(app, app.topMenuOpen, row);
                        continue;
                    }
                    if (app.topMenuOpen >= 0 && y >= 26.0F) {
                        app.topMenuOpen = -1;
                        app.topMenuSelection = -1;
                        continue;
                    }
                }
                if (app.mainMenuVisible && !app.passwordVisible && !app.levelSelect &&
                    !app.helpVisible && !app.settingsVisible)
                    handleMainMenuPointer(app, event.button.x, event.button.y, true);
                else if (app.levelSelect)
                    handleLevelSelectClick(app, event.button.x, event.button.y);
                else if (app.settingsVisible)
                    handleSettingsPointer(app, event.button.x, event.button.y);
                else if (app.bestTimesVisible)
                    handleBestTimesPointer(app, event.button.x, event.button.y);
                else if (!app.paused && !app.helpVisible && !app.passwordVisible &&
                         !app.mainMenuVisible && app.topMenuOpen < 0 &&
                         app.game.outcome == chips::Outcome::playing) {
                    int physicalWidth = 0, physicalHeight = 0, windowWidth = 0, windowHeight = 0;
                    SDL_GetCurrentRenderOutputSize(app.renderer, &physicalWidth, &physicalHeight);
                    SDL_GetWindowSize(app.window, &windowWidth, &windowHeight);
                    if (physicalWidth > 0 && physicalHeight > 0 && windowWidth > 0 && windowHeight > 0) {
                        const float scale = uiScaleForOutput(physicalWidth, physicalHeight,
                                                             app.profile.uiScalePercent);
                        const float x = event.button.x * static_cast<float>(physicalWidth) /
                                        static_cast<float>(windowWidth) / scale;
                        const float y = event.button.y * static_cast<float>(physicalHeight) /
                                        static_cast<float>(windowHeight) / scale;
                        const int outputWidth = std::max(1, static_cast<int>(
                            static_cast<float>(physicalWidth) / scale));
                        const int outputHeight = std::max(1, static_cast<int>(
                            static_cast<float>(physicalHeight) / scale));
                        if (const auto target = boardPositionAtOutput(app, x, y,
                                                                       outputWidth, outputHeight))
                            app.game.setMouseTarget(*target);
                    }
                }
                else
                    handlePauseAndResultPointer(app, event.button.x, event.button.y, true);
            } else if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                if (!app.gamepad) {
                    app.gamepad = SDL_OpenGamepad(event.gdevice.which);
                    app.gamepadStickDirection = -1;
                    app.gamepadStickIgnoreHeld = false;
                    app.gamepadDpadDirection = -1;
                    app.gamepadDpadIgnoreHeld = false;
                    if (!app.gamepad) openFirstGamepad(app);
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                if (app.gamepad && SDL_GetGamepadID(app.gamepad) == event.gdevice.which) {
                    SDL_CloseGamepad(app.gamepad);
                    app.gamepad = nullptr;
                    app.gamepadStickDirection = -1;
                    app.gamepadStickNextRepeat = 0;
                    app.gamepadStickIgnoreHeld = false;
                    app.gamepadDpadDirection = -1;
                    app.gamepadDpadNextRepeat = 0;
                    app.gamepadDpadIgnoreHeld = false;
                    openFirstGamepad(app);
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && app.gamepad &&
                       SDL_GetGamepadID(app.gamepad) == event.gaxis.which) {
                handleGamepadAxis(app, static_cast<SDL_GamepadAxis>(event.gaxis.axis));
            } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && app.gamepad &&
                       SDL_GetGamepadID(app.gamepad) == event.gbutton.which) {
                handleGamepadButton(app, static_cast<SDL_GamepadButton>(event.gbutton.button));
            }
        }
        if (app.levelPackDialogEventPushFailed.exchange(false, std::memory_order_acq_rel)) {
            app.levelPackDialogPending = false;
            app.profileMessage = "Could not finish the file dialog. Please try again.";
            app.lastTick = SDL_GetTicks();
            if (app.exitAfterFileDialog) app.running = false;
        }
        const Uint64 now = SDL_GetTicks();
        if (app.displayProfileDirty && now >= app.displayProfileSaveAt) {
            persistProfile(app, false);
            app.displayProfileDirty = false;
        }
        if (app.gamepad && app.gamepadStickDirection >= 0 && now >= app.gamepadStickNextRepeat) {
            handleGamepadDirection(app, static_cast<chips::Direction>(app.gamepadStickDirection));
            app.gamepadStickNextRepeat = now + 180;
        }
        const int heldDpadDirection = heldGamepadDpadDirection(app.gamepad);
        if (app.paused && !pauseActionsVisible(app)) {
            app.gamepadDpadDirection = -1;
            app.gamepadDpadNextRepeat = 0;
            app.gamepadDpadIgnoreHeld = heldDpadDirection >= 0;
        } else if (app.gamepadDpadIgnoreHeld) {
            if (heldDpadDirection < 0) app.gamepadDpadIgnoreHeld = false;
            app.gamepadDpadDirection = -1;
            app.gamepadDpadNextRepeat = 0;
        } else if (heldDpadDirection != app.gamepadDpadDirection) {
            app.gamepadDpadDirection = heldDpadDirection;
            if (heldDpadDirection >= 0) {
                handleGamepadDirection(app, static_cast<chips::Direction>(heldDpadDirection));
                app.gamepadDpadNextRepeat = now + 300;
            } else {
                app.gamepadDpadNextRepeat = 0;
            }
        } else if (heldDpadDirection >= 0 && now >= app.gamepadDpadNextRepeat) {
            handleGamepadDirection(app, static_cast<chips::Direction>(heldDpadDirection));
            app.gamepadDpadNextRepeat = now + 180;
        }
        if (app.paused || app.helpVisible || app.debugVisible || app.settingsVisible || app.levelSelect ||
            app.levelPlacardVisible || app.passwordVisible || app.mainMenuVisible || app.levelPackDialogPending)
            app.lastTick = now;
        app.audio.update();
        // The original board timer uses 110 ms while a level is active.
        while (now - app.lastTick >= 110) {
            if (!app.paused && !app.helpVisible && !app.debugVisible && !app.settingsVisible && !app.levelSelect &&
                !app.levelPlacardVisible && !app.passwordVisible && !app.mainMenuVisible &&
                !app.levelPackDialogPending) {
                const int oldPosition = app.game.playerPosition;
                std::vector<int> oldCreaturePositions;
                oldCreaturePositions.reserve(app.game.creatures.size());
                for (const auto& creature : app.game.creatures)
                    oldCreaturePositions.push_back(creature.alive ? creature.position : -1);
                const auto oldOutcome = app.game.outcome;
                app.game.advanceTicks();
                const bool playerAttempt = app.game.playerAttemptedThisTick;
                const bool playerTeleported = app.game.playerTeleportedThisTick;
                const bool switchActivated = app.game.switchActivatedThisTick;
                const auto pickup = app.game.playerPickupThisTick;
                if (oldOutcome == chips::Outcome::playing &&
                    app.game.outcome == chips::Outcome::won &&
                    !app.levels.empty() && app.levelIndex + 1 >= app.levels.size())
                    app.actionSelection = 2;
                if (app.game.playerPosition != oldPosition) {
                    app.playerAnimationFrom = oldPosition;
                    app.playerAnimationTo = app.game.playerPosition;
                    app.playerAnimationStart = now;
                    app.playerAnimationTeleported = playerTeleported;
                }
                if (app.creatureAnimations.size() < app.game.creatures.size())
                    app.creatureAnimations.resize(app.game.creatures.size());
                for (std::size_t i = 0; i < oldCreaturePositions.size(); ++i) {
                    const int oldCreaturePosition = oldCreaturePositions[i];
                    const auto& creature = app.game.creatures[i];
                    if (oldCreaturePosition >= 0 && creature.alive &&
                        creature.position != oldCreaturePosition) {
                        app.creatureAnimations[i] = {oldCreaturePosition, creature.position, now,
                                                     creature.teleportedThisTick};
                    }
                }
                if (oldOutcome == chips::Outcome::playing && app.game.outcome != oldOutcome) {
                    app.audio.play(app.game.outcome == chips::Outcome::won
                        ? chips::AudioCue::victory : chips::AudioCue::death);
                } else if (playerAttempt) {
                    if (app.game.playerPosition != oldPosition) {
                        const auto pickupCue = pickup == chips::PickupKind::key
                            ? chips::AudioCue::keyPickup
                            : pickup == chips::PickupKind::boot ? chips::AudioCue::bootPickup
                            : chips::AudioCue::pickup;
                        app.audio.play(playerTeleported ? chips::AudioCue::teleport
                            : switchActivated ? chips::AudioCue::switchTile
                            : pickup != chips::PickupKind::none ? pickupCue : chips::AudioCue::move);
                    } else {
                        app.audio.play(chips::AudioCue::blocked);
                    }
                } else if (switchActivated) {
                    app.audio.play(chips::AudioCue::switchTile);
                }
            }
            app.lastTick += 110;
        }
        recordLevelWin(app);
        draw(app);
    }
    if (app.displayProfileDirty) {
        persistProfile(app, false);
        app.displayProfileDirty = false;
    }
}

int validatePack(const std::filesystem::path& path) {
    try {
        const auto levels = chips::loadLevelPack(path);
        if (levels.empty()) throw chips::LevelPackError("Level pack contains no levels");
        bool ordered = true;
        std::size_t trapLinks = 0;
        std::size_t cloneLinks = 0;
        std::size_t teleports = 0;
        for (std::size_t i = 0; i < levels.size(); ++i) {
            ordered = ordered && levels[i].number == i + 1;
            const auto state = chips::GameState::start(levels[i]);
            if (state.outcome != chips::Outcome::playing) {
                throw chips::LevelPackError("Level " + std::to_string(i + 1) +
                                            " does not produce a playable initial state");
            }
            trapLinks += state.traps.size();
            cloneLinks += state.cloneLinks.size();
            teleports += state.teleports.size();
        }
        std::cout << "Levels: " << levels.size() << "\n"
                  << "Sequential numbering: " << (ordered ? "yes" : "no") << "\n"
                  << "Each layer: 32 x 32 tiles\n"
                  << "First: " << levels.front().title << "\n"
                  << "Last: " << levels.back().title << "\n"
                  << "Trap links: " << trapLinks << "\n"
                  << "Clone links: " << cloneLinks << "\n"
                  << "Teleport tiles: " << teleports << "\n"
                  << "All levels initialize: yes\n";
        return levels.size() == 149 && ordered ? 0 : 2;
    } catch (const std::exception& error) {
        std::cerr << "Level-pack error: " << error.what() << '\n';
        return 1;
    }
}

int validateAtlas(const std::filesystem::path& path) {
    const auto pathUtf8 = pathToUtf8(path);
    SDL_Surface* surface = SDL_LoadPNG(pathUtf8.c_str());
    if (!surface) {
        std::cerr << "Atlas error: " << SDL_GetError() << '\n';
        return 1;
    }
    constexpr int columns = 13;
    constexpr int rows = 16;
    constexpr int cellSize = 64;
    constexpr int sourceCellSize = cellSize / 2;
    constexpr int expectedWidth = columns * cellSize;
    constexpr int expectedHeight = rows * cellSize;
    if (surface->w != expectedWidth || surface->h != expectedHeight) {
        std::cerr << "Atlas error: expected " << expectedWidth << 'x' << expectedHeight
                  << " pixels (" << columns << 'x' << rows << " source cells enlarged 2x), got "
                  << surface->w << 'x' << surface->h << '\n';
        SDL_DestroySurface(surface);
        return 2;
    }
    unsigned mappedTiles = 0;
    unsigned nonRenderedTiles = 0;
    for (unsigned tile = 0; tile <= 0x6F; ++tile) {
        const bool intentionallyInvisible = tile == 0x05 || tile == 0x20 || tile == 0x2C ||
            tile == 0x2E || (tile >= 0x33 && tile <= 0x38);
        if (intentionallyInvisible) {
            ++nonRenderedTiles;
            continue;
        }
        if (!hasAtlasMapping(static_cast<std::uint8_t>(tile))) {
            std::cerr << "Atlas error: visible gameplay tile 0x" << std::hex << tile
                      << std::dec << " has no source tile-sheet cell\n";
            SDL_DestroySurface(surface);
            return 2;
        }
        ++mappedTiles;
    }
    std::cout << "Original tile sheet: " << surface->w << " x " << surface->h
              << " RGBA PNG, " << sourceCellSize << "x" << sourceCellSize
              << " source cells enlarged 2x\n"
              << "Source grid: " << columns << " columns x " << rows << " rows\n"
              << "Visible gameplay tile mappings: " << mappedTiles << " of "
              << (0x70U - nonRenderedTiles) << " renderable IDs\n"
              << "Hidden or non-rendered IDs: " << nonRenderedTiles << " of 112\n";
    SDL_DestroySurface(surface);
    return 0;
}

int validateIcon(const std::filesystem::path& path) {
    const auto pathUtf8 = pathToUtf8(path);
    SDL_Surface* surface = SDL_LoadPNG(pathUtf8.c_str());
    if (!surface) {
        std::cerr << "Icon error: " << SDL_GetError() << '\n';
        return 1;
    }
    if (surface->w != surface->h || (surface->w != 128 && surface->w != 256)) {
        std::cerr << "Icon error: expected a 128x128 or 256x256 square PNG, got "
                  << surface->w << 'x' << surface->h << '\n';
        SDL_DestroySurface(surface);
        return 2;
    }
    std::cout << "App icon: " << surface->w << " x " << surface->h << " RGBA-compatible PNG\n";
    SDL_DestroySurface(surface);
    return 0;
}

int validateCleanedSprites(const std::filesystem::path& creaturePath,
                           const std::filesystem::path& itemPath) {
    const auto validateSheet = [](const std::filesystem::path& path,
                                  int expectedWidth, int expectedHeight,
                                  const char* label) {
        const auto pathUtf8 = pathToUtf8(path);
        SDL_Surface* surface = SDL_LoadPNG(pathUtf8.c_str());
        if (!surface) {
            std::cerr << label << " sheet error: " << SDL_GetError() << '\n';
            return false;
        }
        const bool valid = surface->w == expectedWidth && surface->h == expectedHeight;
        if (!valid) {
            std::cerr << label << " sheet error: expected " << expectedWidth << 'x'
                      << expectedHeight << " pixels, got " << surface->w << 'x'
                      << surface->h << '\n';
        } else {
            std::cout << label << " sheet: " << surface->w << " x " << surface->h
                      << " RGBA-compatible PNG\n";
        }
        SDL_DestroySurface(surface);
        return valid;
    };
    const bool creaturesValid = validateSheet(creaturePath, 1536, 1536, "Cleaned creature (6x6, 256px frames)");
    const bool itemsValid = validateSheet(itemPath, 1774, 887, "Cleaned key/boot (4x2, 443.5px frames)");
    return creaturesValid && itemsValid ? 0 : 2;
}

int validatePlayerSprites(const std::filesystem::path& landPath,
                          const std::filesystem::path& swimPath) {
    const auto validateSheet = [](const std::filesystem::path& path, const char* label) {
        const auto pathUtf8 = pathToUtf8(path);
        SDL_Surface* surface = SDL_LoadPNG(pathUtf8.c_str());
        if (!surface) {
            std::cerr << label << " sheet error: " << SDL_GetError() << '\n';
            return false;
        }
        constexpr int expectedWidth = 2172;  // 4 horizontal frames, 543px each.
        constexpr int expectedHeight = 724;
        const bool valid = surface->w == expectedWidth && surface->h == expectedHeight;
        if (!valid) {
            std::cerr << label << " sheet error: expected " << expectedWidth << 'x'
                      << expectedHeight << " pixels, got " << surface->w << 'x'
                      << surface->h << '\n';
        } else {
            std::cout << label << " sheet: " << surface->w << " x " << surface->h
                      << " RGBA-compatible PNG (4 directional frames)\n";
        }
        SDL_DestroySurface(surface);
        return valid;
    };
    const bool landValid = validateSheet(landPath, "Polished Chip land-pose");
    const bool swimValid = validateSheet(swimPath, "Polished Chip swimming-pose");
    return landValid && swimValid ? 0 : 2;
}

int validateDigits(const std::filesystem::path& path) {
    const auto pathUtf8 = pathToUtf8(path);
    SDL_Surface* surface = SDL_LoadPNG(pathUtf8.c_str());
    if (!surface) {
        std::cerr << "Digit-sheet error: " << SDL_GetError() << '\n';
        return 1;
    }
    constexpr int expectedWidth = 34;
    constexpr int expectedHeight = 1104; // 24 original 17x23 glyphs, enlarged 2x
    if (surface->w != expectedWidth || surface->h != expectedHeight) {
        std::cerr << "Digit-sheet error: expected " << expectedWidth << 'x' << expectedHeight
                  << " pixels, got " << surface->w << 'x' << surface->h << '\n';
        SDL_DestroySurface(surface);
        return 2;
    }
    std::cout << "Original scoreboard digits: " << surface->w << " x " << surface->h
              << " RGBA-compatible PNG\n";
    SDL_DestroySurface(surface);
    return 0;
}
}

int main(int argc, char** argv) {
    auto nativeArguments = chips::platform::Utf8Arguments::fromNative(argc, argv);
    if (!nativeArguments) {
        std::cerr << "Unable to convert native command-line arguments to UTF-8.\n";
        return 1;
    }
    argc = nativeArguments->argc();
    argv = nativeArguments->argv();
    if (argc >= 2 && std::string(argv[1]) == "--validate") {
        if (argc < 3) {
            std::cerr << "Usage: chips-native --validate <CHIPS.DAT>\n";
            return 2;
        }
        return validatePack(pathFromUtf8(argv[2]));
    }
    if (argc >= 2 && std::string(argv[1]) == "--validate-atlas") {
        if (argc < 3) {
            std::cerr << "Usage: chips-native --validate-atlas <tile-atlas.png>\n";
            return 2;
        }
        return validateAtlas(pathFromUtf8(argv[2]));
    }
    if (argc >= 2 && std::string(argv[1]) == "--validate-icon") {
        if (argc < 3) {
            std::cerr << "Usage: chips-native --validate-icon <app-icon.png>\n";
            return 2;
        }
        return validateIcon(pathFromUtf8(argv[2]));
    }
    if (argc >= 2 && std::string(argv[1]) == "--validate-cleaned-sprites") {
        if (argc < 4) {
            std::cerr << "Usage: chips-native --validate-cleaned-sprites <creatures.png> <keys-boots.png>\n";
            return 2;
        }
        return validateCleanedSprites(pathFromUtf8(argv[2]), pathFromUtf8(argv[3]));
    }
    if (argc >= 2 && std::string(argv[1]) == "--validate-player-sprites") {
        if (argc < 4) {
            std::cerr << "Usage: chips-native --validate-player-sprites <land.png> <swim.png>\n";
            return 2;
        }
        return validatePlayerSprites(pathFromUtf8(argv[2]), pathFromUtf8(argv[3]));
    }
    if (argc >= 2 && std::string(argv[1]) == "--validate-digits") {
        if (argc < 3) {
            std::cerr << "Usage: chips-native --validate-digits <digital-digits.png>\n";
            return 2;
        }
        return validateDigits(pathFromUtf8(argv[2]));
    }

    try {
        const auto* basePath = SDL_GetBasePath();
        const std::filesystem::path packPath = argc >= 2
            ? pathFromUtf8(argv[1])
            : pathFromUtf8(basePath ? basePath : "") / "data" / "CHIPS.DAT";
        App app;
        if (argc >= 2) app.profileFileName = customPackProfileFileName(packPath);
        app.levels = chips::loadLevelPack(packPath);
        if (app.levels.empty()) throw chips::LevelPackError("Level pack contains no levels");
        if (!initialize(app)) return 1;
        // The original seeds its process-wide CRT generator once from the
        // platform's milliseconds-since-startup clock. The adapter uses the
        // matching Windows clock and a monotonic equivalent elsewhere.
        resetPlayer(app);
        loadCampaignProfile(app);
        run(app);
        if (app.gamepad) SDL_CloseGamepad(app.gamepad);
        app.audio.shutdown();
        SDL_DestroyTexture(app.originalTransparentSprites);
        SDL_DestroyTexture(app.polishedPlayerLandPoses);
        SDL_DestroyTexture(app.polishedPlayerSwimPoses);
        SDL_DestroyTexture(app.cleanedCreatureSprites);
        SDL_DestroyTexture(app.cleanedKeyBootIcons);
        SDL_DestroyTexture(app.menuBackground);
        SDL_DestroyTexture(app.victoryIllustration);
        SDL_DestroyTexture(app.appIcon);
        SDL_DestroyTexture(app.digitalDigits);
        SDL_DestroyTexture(app.scoreboardPanel);
        SDL_DestroyTexture(app.atlas);
        SDL_DestroyRenderer(app.renderer);
        SDL_DestroyWindow(app.window);
        SDL_Quit();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Unable to start game: " << error.what() << '\n';
        return 1;
    }
}
