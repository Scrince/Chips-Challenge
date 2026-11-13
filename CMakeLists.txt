cmake_minimum_required(VERSION 3.24)
project(ChipsChallengeNative VERSION 1.0.1 LANGUAGES CXX)

if(WIN32)
    enable_language(RC)
    configure_file(src/app_icon.rc.in "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc" @ONLY)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep portable MSVC ZIPs independent of a separately installed VC runtime.
# CMP0091 is NEW through the project's CMake 3.24 minimum, so this selects /MT
# for Release-like configurations and /MTd for Debug across SDL and game targets.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)
set(SDL_SHARED ON CACHE BOOL "Build SDL shared library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Build SDL static library" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL test library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Build SDL tests" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "Build SDL examples" FORCE)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    # Pin the exact release commit so a moved tag cannot silently change builds.
    GIT_TAG 829a65d769d935c4852f8159e964312c0957260a
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(SDL3)

set(SDLTTF_VENDORED ON CACHE BOOL "Build SDL3_ttf with vendored dependencies" FORCE)
set(SDLTTF_HARFBUZZ OFF CACHE BOOL "Disable shaping dependency for Latin UI text" FORCE)
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "Disable unused SVG font support" FORCE)
set(SDLTTF_SAMPLES OFF CACHE BOOL "Do not build SDL3_ttf samples" FORCE)
set(SDLTTF_TESTS OFF CACHE BOOL "Do not build SDL3_ttf tests" FORCE)
FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(SDL3_ttf)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")
set(CHIPS_RUNTIME_ASSETS
    assets/ASSET_MANIFEST.json
    assets/help/chips-help.html
    assets/fonts/NotoSans.ttf
    assets/fonts/OFL.txt
    assets/art/tile-atlas.png
    assets/art/menu-background.png
    assets/art/menu-background-hd.png
    assets/art/victory-illustration.png
    assets/art/digital-digits.png
    assets/art/scoreboard-panel.png
    assets/art/app-icon.png
    assets/art/app-icon.ico
    assets/art/app-icon-polished.png
    assets/art/app-icon-polished.ico
    assets/art/original-transparent-sprites.png
    assets/art/player-poses-polished.png
    assets/art/player-swim-poses-polished.png
    assets/art/creature-sprites-polished.png
    assets/art/key-boot-icons-polished.png
    assets/audio/BLIP2.WAV
    assets/audio/BUMMER.WAV
    assets/audio/CHIP01.MID
    assets/audio/CHIP02.MID
    assets/audio/CLICK3.WAV
    assets/audio/DITTY1.WAV
    assets/audio/DOOR.WAV
    assets/audio/OOF3.WAV
    assets/audio/POP2.WAV
    assets/audio/STRIKE.WAV
    assets/audio/TELEPORT.WAV
    assets/audio/WATER2.WAV
)
foreach(asset IN LISTS CHIPS_RUNTIME_ASSETS)
    get_filename_component(asset_directory "${asset}" DIRECTORY)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${asset_directory}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${asset}"
                   "${CMAKE_CURRENT_BINARY_DIR}/${asset}" COPYONLY)
endforeach()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/data/CHIPS.DAT"
               "${CMAKE_CURRENT_BINARY_DIR}/data/CHIPS.DAT" COPYONLY)

add_executable(chips-native src/main.cpp src/renderer.cpp src/input_mapping.cpp src/levelpack.cpp src/engine.cpp src/replay.cpp src/profile.cpp src/platform_file.cpp src/platform_command_line.cpp src/platform_clock.cpp src/audio.cpp src/password.cpp)
if(WIN32)
    target_sources(chips-native PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
endif()
target_link_libraries(chips-native PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)
if(WIN32)
    target_link_libraries(chips-native PRIVATE shell32 winmm)
elseif(APPLE)
    target_link_libraries(chips-native PRIVATE "-framework AudioToolbox" "-framework CoreFoundation")
endif()
target_compile_features(chips-native PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-native PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-native PRIVATE -Wall -Wextra -Wpedantic)
endif()

# A relocatable macOS/Linux install keeps the files beside the executable,
# matching SDL_GetBasePath() resource lookup. The shared SDL runtime is placed
# in the same directory and resolved through an executable-relative RPATH.
if(APPLE OR UNIX)
    if(APPLE)
        set(CHIPS_INSTALL_RPATH "@loader_path")
    else()
        set(CHIPS_INSTALL_RPATH "\$ORIGIN")
    endif()
    set_property(TARGET chips-native PROPERTY INSTALL_RPATH "${CHIPS_INSTALL_RPATH}")
    if(TARGET SDL3-shared)
        set_property(TARGET SDL3-shared PROPERTY INSTALL_RPATH "${CHIPS_INSTALL_RPATH}")
        install(TARGETS chips-native SDL3-shared
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .)
        if(TARGET SDL3_ttf-shared)
            set_property(TARGET SDL3_ttf-shared PROPERTY INSTALL_RPATH "${CHIPS_INSTALL_RPATH}")
            install(TARGETS SDL3_ttf-shared
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .)
        endif()
    else()
        install(TARGETS chips-native RUNTIME DESTINATION .)
    endif()
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION assets)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/data/CHIPS.DAT" DESTINATION data)
endif()

enable_testing()
add_executable(chips-engine-tests tests/engine_tests.cpp src/engine.cpp src/replay.cpp src/levelpack.cpp)
target_include_directories(chips-engine-tests PRIVATE src)
target_compile_definitions(chips-engine-tests PRIVATE
    CHIPS_TEST_PACK_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/CHIPS.DAT")
target_compile_features(chips-engine-tests PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-engine-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-engine-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME engine-unit-checks COMMAND chips-engine-tests)

add_executable(chips-viewport-tests tests/viewport_tests.cpp)
target_include_directories(chips-viewport-tests PRIVATE src)
target_compile_features(chips-viewport-tests PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-viewport-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-viewport-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME player-viewport-checks COMMAND chips-viewport-tests)

add_executable(chips-render-assets-tests tests/render_assets_tests.cpp)
target_include_directories(chips-render-assets-tests PRIVATE src)
target_compile_features(chips-render-assets-tests PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-render-assets-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-render-assets-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME render-tile-selection-checks COMMAND chips-render-assets-tests)

add_executable(chips-input-mapping-tests tests/input_mapping_tests.cpp src/input_mapping.cpp)
target_include_directories(chips-input-mapping-tests PRIVATE src)
target_compile_features(chips-input-mapping-tests PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-input-mapping-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-input-mapping-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME input-mapping-checks COMMAND chips-input-mapping-tests)

add_executable(chips-levelpack-tests tests/levelpack_tests.cpp src/levelpack.cpp)
target_include_directories(chips-levelpack-tests PRIVATE src)
target_compile_features(chips-levelpack-tests PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-levelpack-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-levelpack-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME levelpack-format-checks COMMAND chips-levelpack-tests)

add_executable(chips-profile-tests tests/profile_tests.cpp src/profile.cpp src/platform_file.cpp)
target_include_directories(chips-profile-tests PRIVATE src)
target_compile_features(chips-profile-tests PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-profile-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-profile-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME profile-persistence-checks COMMAND chips-profile-tests)

add_executable(chips-password-tests tests/password_tests.cpp src/password.cpp src/levelpack.cpp)
target_include_directories(chips-password-tests PRIVATE src)
target_compile_features(chips-password-tests PRIVATE cxx_std_20)
target_compile_definitions(chips-password-tests PRIVATE
    CHIPS_TEST_PACK_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/CHIPS.DAT")
if(MSVC)
    target_compile_options(chips-password-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-password-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME campaign-password-checks COMMAND chips-password-tests)

add_executable(chips-audio-tests tests/audio_tests.cpp src/audio.cpp)
target_include_directories(chips-audio-tests PRIVATE src)
target_link_libraries(chips-audio-tests PRIVATE SDL3::SDL3)
if(APPLE)
    target_link_libraries(chips-audio-tests PRIVATE "-framework AudioToolbox" "-framework CoreFoundation")
endif()
target_compile_definitions(chips-audio-tests PRIVATE
    CHIPS_TEST_RESOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
if(WIN32)
    target_link_libraries(chips-audio-tests PRIVATE winmm)
endif()
target_compile_features(chips-audio-tests PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-audio-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-audio-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME original-audio-checks COMMAND chips-audio-tests)
set_tests_properties(original-audio-checks PROPERTIES ENVIRONMENT "SDL_AUDIODRIVER=dummy")

add_executable(chips-campaign-validation
    tests/campaign_validation.cpp src/engine.cpp src/levelpack.cpp src/replay.cpp)
target_include_directories(chips-campaign-validation PRIVATE src)
target_compile_features(chips-campaign-validation PRIVATE cxx_std_20)
target_compile_definitions(chips-campaign-validation PRIVATE
    CHIPS_TEST_PACK_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/CHIPS.DAT")
if(MSVC)
    target_compile_options(chips-campaign-validation PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-campaign-validation PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME original-campaign-validation COMMAND chips-campaign-validation)

add_executable(chips-levelpack-dump tools/levelpack_dump.cpp src/levelpack.cpp)
target_include_directories(chips-levelpack-dump PRIVATE src)
target_compile_features(chips-levelpack-dump PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-levelpack-dump PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-levelpack-dump PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(chips-replay tools/replay_runner.cpp src/engine.cpp src/levelpack.cpp src/replay.cpp)
target_include_directories(chips-replay PRIVATE src)
target_compile_features(chips-replay PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-replay PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-replay PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Diagnostic shortest-route search over the actual native engine state. This
# tool is for campaign QA and is not part of the packaged game runtime.
add_executable(chips-route-search tools/native_route_search.cpp src/engine.cpp src/levelpack.cpp src/replay.cpp)
target_include_directories(chips-route-search PRIVATE src)
target_compile_features(chips-route-search PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(chips-route-search PRIVATE /W4 /permissive-)
else()
    target_compile_options(chips-route-search PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(Python3 3.10 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(NAME reference-rle-format-checks
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/reference_rle_test.py")
    add_test(NAME reference-optional-metadata-checks
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/reference_optional_metadata_test.py")
    add_test(NAME recorded-key-replay-timing-checks
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/recorded_key_replay_test.py")
    add_test(NAME recorded-level2-source-timing-parity
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/replay_recorded_key_events.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/level2-key-capture.json"
            --runner "$<TARGET_FILE:chips-replay>"
            --pack "${CMAKE_CURRENT_SOURCE_DIR}/data/CHIPS.DAT"
            --level 2 --seed 1 --pre-record-ticks 18 --expect-outcome won
            --expect-player 392 --expect-chips 0)
    add_test(NAME decoded-campaign-crosscheck
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/compare_campaign.py"
            --native "$<TARGET_FILE:chips-levelpack-dump>"
            --pack "${CMAKE_CURRENT_SOURCE_DIR}/data/CHIPS.DAT"
            --report "${CMAKE_CURRENT_BINARY_DIR}/campaign-comparison.json")
    add_test(NAME solved-campaign-route-replays
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_known_routes.py"
            --runner "$<TARGET_FILE:chips-replay>"
            --pack "${CMAKE_CURRENT_SOURCE_DIR}/data/CHIPS.DAT"
            --routes "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/native-known-routes.json"
            --report "${CMAKE_CURRENT_BINARY_DIR}/native-known-route-validation.json"
            --seed 1 --ticks-per-input 2 --expect-wins 1,2,3,6,7,8,10,13)
endif()

if(WIN32)
    add_custom_command(TARGET chips-native POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:chips-native>/data"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/data/CHIPS.DAT"
            "$<TARGET_FILE_DIR:chips-native>/data/CHIPS.DAT"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:chips-native>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3>"
            "$<TARGET_FILE_DIR:chips-native>"
    )
    if(TARGET SDL3_ttf-shared)
        add_custom_command(TARGET chips-native POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:SDL3_ttf-shared>"
                "$<TARGET_FILE_DIR:chips-native>"
        )
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET chips-native POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
            "$<TARGET_FILE_DIR:chips-native>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_BIN_DIR}/libstdc++-6.dll"
            "$<TARGET_FILE_DIR:chips-native>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_BIN_DIR}/libwinpthread-1.dll"
            "$<TARGET_FILE_DIR:chips-native>"
        )
    endif()
endif()
