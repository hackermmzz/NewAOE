cmake_minimum_required(VERSION 3.20)

# This file is intentionally usable both through `-DAOE_BUILD=Engine` at the
# repository root and as a small standalone project for editor development.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(NewAOEParticleSystemEditor LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(NOT WIN32 AND
   NOT (UNIX AND NOT APPLE AND NOT ANDROID AND NOT EMSCRIPTEN))
    message(FATAL_ERROR
        "ParticleSystemEditor is supported on Windows and desktop Linux only"
    )
endif()

get_filename_component(NEWAOE_ROOT_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../../../.."
    ABSOLUTE
)

set(NEWAOE_IMGUI_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/imgui")
set(NEWAOE_SOKOL_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/sokol")
set(NEWAOE_STB_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/stb")
set(NEWAOE_PARTICLE_DIR "${CMAKE_CURRENT_LIST_DIR}/../particle")

foreach(required_file
    "${NEWAOE_IMGUI_DIR}/imgui.cpp"
    "${NEWAOE_SOKOL_DIR}/sokol_app.h"
    "${NEWAOE_SOKOL_DIR}/sokol_gfx.h"
    "${NEWAOE_SOKOL_DIR}/sokol_glue.h"
    "${NEWAOE_SOKOL_DIR}/sokol_log.h"
    "${NEWAOE_SOKOL_DIR}/util/sokol_imgui.h"
    "${NEWAOE_STB_DIR}/stb_image.h"
)
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR "Particle editor dependency is missing: ${required_file}")
    endif()
endforeach()

add_executable(ParticleSystemEditor
    main.cpp
    OffscreenCli.cpp
    OffscreenCli.h
    ParticleEditorApp.cpp
    ParticleEditorApp.h
    ParticlePreview.cpp
    ParticlePreview.h
    PreviewTexture.cpp
    PreviewTexture.h
    "${NEWAOE_PARTICLE_DIR}/ParticleSystem.cpp"
    "${NEWAOE_PARTICLE_DIR}/ParticleSystem.h"
    "${NEWAOE_PARTICLE_DIR}/ParticleJson.cpp"
    "${NEWAOE_IMGUI_DIR}/imgui.cpp"
    "${NEWAOE_IMGUI_DIR}/imgui_draw.cpp"
    "${NEWAOE_IMGUI_DIR}/imgui_tables.cpp"
    "${NEWAOE_IMGUI_DIR}/imgui_widgets.cpp"
)

target_compile_features(ParticleSystemEditor PRIVATE cxx_std_17)
target_include_directories(ParticleSystemEditor PRIVATE
    "${NEWAOE_ROOT_DIR}"
    "${NEWAOE_IMGUI_DIR}"
    "${NEWAOE_SOKOL_DIR}"
    "${NEWAOE_STB_DIR}"
)

if(MSVC)
    target_compile_options(ParticleSystemEditor PRIVATE /utf-8 /W4 /permissive-)
else()
    target_compile_options(ParticleSystemEditor PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

if(WIN32)
    target_compile_definitions(ParticleSystemEditor PRIVATE SOKOL_D3D11)
    target_link_libraries(ParticleSystemEditor PRIVATE
        kernel32
        user32
        shell32
        d3d11
        dxgi
    )
elseif(UNIX AND NOT APPLE AND NOT ANDROID AND NOT EMSCRIPTEN)
    target_compile_definitions(ParticleSystemEditor PRIVATE SOKOL_GLCORE)

    find_package(Threads REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED COMPONENTS Xi Xcursor)
    target_include_directories(ParticleSystemEditor SYSTEM PRIVATE
        "${X11_INCLUDE_DIR}"
    )
    target_link_libraries(ParticleSystemEditor PRIVATE
        OpenGL::GL
        X11::X11
        X11::Xi
        X11::Xcursor
        ${CMAKE_DL_LIBS}
        Threads::Threads
        m
    )
endif()

set_target_properties(ParticleSystemEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

