cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(SokolSmokeDemo LANGUAGES CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(NEWAOE_ROOT_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../../../.."
    ABSOLUTE
)

set(NEWAOE_SOKOL_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/sokol")
set(NEWAOE_SMOKE_ASSET_DIR "${NEWAOE_ROOT_DIR}/smoke" CACHE PATH
    "Smoke particle directory produced by ParticleSystemEditor --offscreen"
)

foreach(required_file
    "${NEWAOE_SOKOL_DIR}/sokol_app.h"
    "${NEWAOE_SOKOL_DIR}/sokol_gfx.h"
    "${NEWAOE_SOKOL_DIR}/sokol_glue.h"
    "${NEWAOE_SOKOL_DIR}/sokol_log.h"
    "${NEWAOE_SMOKE_ASSET_DIR}/particle.json"
    "${NEWAOE_SMOKE_ASSET_DIR}/smoke.png"
    "${NEWAOE_SMOKE_ASSET_DIR}/grenade_bright.png"
)
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR "Required smoke-demo file is missing: ${required_file}")
    endif()
endforeach()

# Compile the shipping renderer directly into the small integration demo.
add_subdirectory(
    "${CMAKE_CURRENT_LIST_DIR}/.."
    "${CMAKE_BINARY_DIR}/newaoe-game-render-engine"
)

add_executable(SokolSmokeDemo
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_compile_features(SokolSmokeDemo PRIVATE cxx_std_17)
target_include_directories(SokolSmokeDemo PRIVATE
    "${NEWAOE_ROOT_DIR}"
    "${NEWAOE_SOKOL_DIR}"
)
target_link_libraries(SokolSmokeDemo PRIVATE NewAOE::game_render_engine)

if(EMSCRIPTEN OR ANDROID)
    target_compile_definitions(SokolSmokeDemo PRIVATE SOKOL_GLES3)
elseif(WIN32)
    target_compile_definitions(SokolSmokeDemo PRIVATE
        SOKOL_D3D11
        SOKOL_WIN32_FORCE_MAIN
    )
elseif(APPLE)
    target_compile_definitions(SokolSmokeDemo PRIVATE SOKOL_METAL)
elseif(UNIX)
    target_compile_definitions(SokolSmokeDemo PRIVATE SOKOL_GLCORE)
else()
    message(FATAL_ERROR "SokolSmokeDemo has no backend for this platform")
endif()

if(MSVC)
    target_compile_options(SokolSmokeDemo PRIVATE /utf-8 /W4 /permissive-)
else()
    target_compile_options(SokolSmokeDemo PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(EMSCRIPTEN)
    target_link_options(SokolSmokeDemo PRIVATE
        "-sUSE_WEBGL2=1"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=134217728"
        "--preload-file"
        "${NEWAOE_SMOKE_ASSET_DIR}@/assets/smoke"
    )
    set_target_properties(SokolSmokeDemo PROPERTIES SUFFIX ".js")
    add_custom_command(TARGET SokolSmokeDemo POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_LIST_DIR}/shell.html"
            "$<TARGET_FILE_DIR:SokolSmokeDemo>/SokolSmokeDemo.html"
        VERBATIM
    )
elseif(ANDROID)
    target_link_libraries(SokolSmokeDemo PRIVATE GLESv3 EGL log android)
elseif(WIN32)
    target_link_libraries(SokolSmokeDemo PRIVATE
        kernel32
        user32
        shell32
        d3d11
        dxgi
    )
elseif(APPLE)
    enable_language(OBJCXX)
    set_source_files_properties(
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        PROPERTIES LANGUAGE OBJCXX
    )
    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    target_link_libraries(SokolSmokeDemo PRIVATE
        "${APPKIT_FRAMEWORK}"
        "${QUARTZCORE_FRAMEWORK}"
        "${METAL_FRAMEWORK}"
    )
elseif(UNIX)
    find_package(Threads REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED COMPONENTS Xi Xcursor)
    target_include_directories(SokolSmokeDemo SYSTEM PRIVATE
        "${X11_INCLUDE_DIR}"
    )
    target_link_libraries(SokolSmokeDemo PRIVATE
        OpenGL::GL
        X11::X11
        X11::Xi
        X11::Xcursor
        ${CMAKE_DL_LIBS}
        Threads::Threads
        m
    )
endif()

set_target_properties(SokolSmokeDemo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(NOT EMSCRIPTEN)
    add_custom_command(TARGET SokolSmokeDemo POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "$<TARGET_FILE_DIR:SokolSmokeDemo>/assets/smoke"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${NEWAOE_SMOKE_ASSET_DIR}"
            "$<TARGET_FILE_DIR:SokolSmokeDemo>/assets/smoke"
        VERBATIM
    )
endif()

