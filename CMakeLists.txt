cmake_minimum_required(VERSION 3.21)
project(engine2d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # lets VS Code IntelliSense find headers

# ---- dependencies (installed automatically by vcpkg in manifest mode) ----
find_package(SDL2 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm  CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")   # stb is header-only

# ---- the engine executable ----
# As you add files in later phases, list them here.
add_executable(engine2d
    src/main.cpp
)

target_include_directories(engine2d PRIVATE
    src
    ${STB_INCLUDE_DIRS}
)

target_link_libraries(engine2d PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    glad::glad
    glm::glm
)

# ---- copy the assets/ folder next to the .exe after every build ----
# (Phase 2+ needs this so the engine can find shaders/textures at runtime.)
add_custom_command(TARGET engine2d POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:engine2d>/assets
    COMMENT "Copying assets to build output"
)
