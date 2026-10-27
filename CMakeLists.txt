cmake_minimum_required(VERSION 3.20)

project(NewAOE LANGUAGES CXX)

# Configure with:
#   Windows:   cmake -S . -B build-window -DAOE_BUILD=Server
#   Linux/WSL: cmake -S . -B build-linux  -DAOE_BUILD=Server
# Engine builds only the standalone particle-system editor.  Keeping that
# path isolated is important: editor builds must not pull the game server,
# networking, audio, logger or profiler into the tool executable.
set(AOE_BUILD "Both" CACHE STRING
    "Application to build: Server, Client, Both, or Engine"
)
set_property(CACHE AOE_BUILD PROPERTY STRINGS Server Client Both Engine)

string(TOLOWER "${AOE_BUILD}" AOE_BUILD_NORMALIZED)
set(AOE_VALID_BUILD_VALUES server client both engine)
if(NOT AOE_BUILD_NORMALIZED IN_LIST AOE_VALID_BUILD_VALUES)
    message(FATAL_ERROR
        "Invalid AOE_BUILD value '${AOE_BUILD}'. "
        "Expected Server, Client, Both, or Engine."
    )
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Header-only third-party libraries expose namespaced targets so consumers do
# not need to know their on-disk layout.
add_subdirectory(Public/fixed_point)

# Build the browser integration demo without pulling server, networking,
# audio or editor UI dependencies into the WebAssembly bundle. Its asset
# directory is the direct output of ParticleSystemEditor --offscreen.
option(AOE_BUILD_WEB_FIRE_DEMO
    "Build only the Emscripten fire-particle integration demo" OFF)
option(AOE_BUILD_WEB_SMOKE_DEMO
    "Build only the Emscripten smoke-grenade integration demo" OFF)
if(AOE_BUILD_WEB_FIRE_DEMO AND AOE_BUILD_WEB_SMOKE_DEMO)
    message(FATAL_ERROR
        "Select only one Web particle integration demo at a time")
endif()
if(AOE_BUILD_WEB_FIRE_DEMO)
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR
            "AOE_BUILD_WEB_FIRE_DEMO requires the Emscripten toolchain")
    endif()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    add_subdirectory(Server/render/game/engine/WebFireDemo)
    message(STATUS "NewAOE build selection: Web fire-particle demo")
    return()
endif()
if(AOE_BUILD_WEB_SMOKE_DEMO)
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR
            "AOE_BUILD_WEB_SMOKE_DEMO requires the Emscripten toolchain")
    endif()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    add_subdirectory(Server/render/game/engine/WebSmokeDemo)
    message(STATUS "NewAOE build selection: Web smoke-grenade demo")
    return()
endif()

# The editor deliberately takes the short path through the root build.  Its
# own CMakeLists owns Sokol, Dear ImGui, stb_image and platform link details.
# In particular, an Engine configure does not even validate server-only
# dependencies such as Boost.
if(AOE_BUILD_NORMALIZED STREQUAL "engine")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    foreach(config Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER "${config}" config_upper)
        set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${CMAKE_BINARY_DIR}/bin")
    endforeach()

    add_subdirectory(Server/render/game/engine/ParticleSystemEditor)
    message(STATUS "NewAOE build selection: ${AOE_BUILD}")
    message(STATUS "Executable output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    return()
endif()

# Logging and profiling are independent build features. Their interface
# targets below always exist so every consumer receives the same 0/1 macro,
# while disabled features contribute no source files or link dependencies.
option(ENABLE_LOGGER "Build and enable the logger" ON)
option(ENABLE_PROFILE_TRACE "Build and enable the profiler" ON)

set(NEWAOE_BOOST_ROOT "${PROJECT_SOURCE_DIR}/Public/third-party/boost")
if(NOT EXISTS "${NEWAOE_BOOST_ROOT}/boost/version.hpp")
    message(FATAL_ERROR
        "Vendored Boost headers were not found at ${NEWAOE_BOOST_ROOT}"
    )
endif()

add_library(newaoe_boost_headers INTERFACE)
add_library(NewAOE::boost_headers ALIAS newaoe_boost_headers)
target_include_directories(newaoe_boost_headers SYSTEM INTERFACE
    "${NEWAOE_BOOST_ROOT}"
)
# Every Boost component used by NewAOE is header-only. Disable MSVC's
# auto-link pragmas so a platform-specific Boost binary is never requested.
target_compile_definitions(newaoe_boost_headers INTERFACE BOOST_ALL_NO_LIB)

find_package(Threads REQUIRED)

# Debug keeps the normal unstripped build. Release and MinSizeRel are both
# built at minimum size: -Os, one section per function, then discard anything
# the program never reaches.
if(MSVC)
    string(REPLACE "/O2" "/O1" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
else()
    string(REPLACE "-O3" "-Os" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT NEWAOE_IPO_SUPPORTED OUTPUT NEWAOE_IPO_ERROR)
if(NEWAOE_IPO_SUPPORTED)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto")
        set(CMAKE_CXX_LINK_OPTIONS_IPO "-flto=auto")
    endif()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)
else()
    message(STATUS "Link-time optimization is unavailable: ${NEWAOE_IPO_ERROR}")
endif()

if(MSVC)
    # /MP compiles translation units in parallel. /Gy is the section split that
    # lets /OPT:REF drop unused functions, and it stays off in Debug.
    add_compile_options(/utf-8 /MP)
    add_compile_options("$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:/Gy>")
else()
    add_compile_options(
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-ffunction-sections>"
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-fdata-sections>"
    )
endif()

add_library(newaoe_profile INTERFACE)
add_library(NewAOE::profile ALIAS newaoe_profile)
target_include_directories(newaoe_profile INTERFACE "${PROJECT_SOURCE_DIR}")
target_compile_features(newaoe_profile INTERFACE cxx_std_17)
if(ENABLE_PROFILE_TRACE)
    target_sources(newaoe_profile INTERFACE
        "${PROJECT_SOURCE_DIR}/Public/profile/profile.cpp"
        "${PROJECT_SOURCE_DIR}/Public/profile/profile_worker.cpp"
    )
    target_compile_definitions(newaoe_profile INTERFACE ENABLE_PROFILE_TRACE=1)
    target_link_libraries(newaoe_profile INTERFACE
        Threads::Threads
        NewAOE::boost_headers
    )
    if(WIN32)
        target_link_libraries(newaoe_profile INTERFACE ws2_32 mswsock)
    endif()
else()
    target_compile_definitions(newaoe_profile INTERFACE
        ENABLE_PROFILE_TRACE=0
        AOE_DISABLE_PROFILING=1
    )
endif()

add_library(newaoe_logger INTERFACE)
add_library(NewAOE::logger ALIAS newaoe_logger)
target_include_directories(newaoe_logger INTERFACE "${PROJECT_SOURCE_DIR}")
target_compile_features(newaoe_logger INTERFACE cxx_std_17)
if(ENABLE_LOGGER)
    target_sources(newaoe_logger INTERFACE
        "${PROJECT_SOURCE_DIR}/Public/logger/logger_worker.cpp"
    )
    target_compile_definitions(newaoe_logger INTERFACE ENABLE_LOGGER=1)
    target_link_libraries(newaoe_logger INTERFACE Threads::Threads)
else()
    target_compile_definitions(newaoe_logger INTERFACE ENABLE_LOGGER=0)
endif()

option(AOE_BUILD_PROFILE_TESTS "Build the profile unit test" OFF)
if(AOE_BUILD_PROFILE_TESTS)
    if(NOT ENABLE_PROFILE_TRACE)
        message(FATAL_ERROR
            "AOE_BUILD_PROFILE_TESTS requires ENABLE_PROFILE_TRACE=ON"
        )
    endif()
    enable_testing()
    add_executable(ProfileTests
        "${PROJECT_SOURCE_DIR}/test/profile_test.cpp"
        "${PROJECT_SOURCE_DIR}/Public/util/util.cpp"
    )
    target_link_libraries(ProfileTests PRIVATE NewAOE::profile)
    add_test(NAME ProfileTests COMMAND ProfileTests)
endif()

option(AOE_BUILD_PROFILE_BENCHMARKS "Build the profile microbenchmark" OFF)
if(AOE_BUILD_PROFILE_BENCHMARKS)
    if(NOT ENABLE_PROFILE_TRACE)
        message(FATAL_ERROR
            "AOE_BUILD_PROFILE_BENCHMARKS requires ENABLE_PROFILE_TRACE=ON"
        )
    endif()
    add_executable(ProfileBenchmark
        "${PROJECT_SOURCE_DIR}/test/profile_benchmark.cpp"
        "${PROJECT_SOURCE_DIR}/Public/util/util.cpp"
    )
    target_link_libraries(ProfileBenchmark PRIVATE NewAOE::profile)
endif()

option(AOE_BUILD_SDL_DEMO "Build the vendored SDL2 demo" OFF)
if(AOE_BUILD_SDL_DEMO)
    enable_language(C)
    add_subdirectory(test/sdl_demo)
endif()

option(AOE_BUILD_RENDER_TESTS "Build the SDL2 + Dear ImGui render smoke tests" OFF)
option(AOE_BUILD_HUD_DEMO "Build the native SDL2 + ImGui HUD demo" OFF)
option(AOE_BUILD_WEB_HUD_DEMO "Build the Emscripten SDL2 + ImGui HUD demo" OFF)

if(MSVC)
    add_link_options(
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:/OPT:REF>"
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:/OPT:ICF>"
    )
elseif(APPLE)
    add_link_options(
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Wl,-dead_strip>"
    )
else()
    add_link_options(
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Wl,--gc-sections>"
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-s>"
    )
endif()

# Keep executable locations consistent for single- and multi-config generators.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" config_upper)
    set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${CMAKE_BINARY_DIR}/bin")
endforeach()

if(AOE_BUILD_NORMALIZED STREQUAL "server" OR AOE_BUILD_NORMALIZED STREQUAL "both")
    add_subdirectory(Server)
endif()

if(AOE_BUILD_NORMALIZED STREQUAL "client" OR AOE_BUILD_NORMALIZED STREQUAL "both")
    add_subdirectory(Client)
endif()

if(AOE_BUILD_RENDER_TESTS)
    if(ANDROID OR EMSCRIPTEN)
        message(STATUS
            "AOE_BUILD_RENDER_TESTS is enabled, but native render tests are "
            "not run for Android or Emscripten targets"
        )
    elseif(NOT TARGET NewAOE::render)
        message(FATAL_ERROR
            "AOE_BUILD_RENDER_TESTS requires AOE_BUILD=Server or Both and "
            "ENABLE_GAME_RENDER=ON"
        )
    else()
        enable_testing()
        add_subdirectory(test/imgui_sdl2)
    endif()
endif()

if(AOE_BUILD_HUD_DEMO OR AOE_BUILD_WEB_HUD_DEMO)
    if(AOE_BUILD_WEB_HUD_DEMO AND NOT EMSCRIPTEN)
        message(FATAL_ERROR
            "AOE_BUILD_WEB_HUD_DEMO requires the Emscripten toolchain"
        )
    elseif(AOE_BUILD_HUD_DEMO AND EMSCRIPTEN)
        message(FATAL_ERROR
            "Use AOE_BUILD_WEB_HUD_DEMO for an Emscripten HUD build"
        )
    elseif(NOT TARGET NewAOE::render)
        message(FATAL_ERROR
            "The HUD demo requires AOE_BUILD=Server or Both and "
            "ENABLE_GAME_RENDER=ON"
        )
    else()
        add_subdirectory(test/hud_sdl_web)
    endif()
endif()

message(STATUS "NewAOE build selection: ${AOE_BUILD}")
message(STATUS "Executable output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
