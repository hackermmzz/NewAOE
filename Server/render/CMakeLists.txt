# Resolve the repository root locally so this module remains reusable from
# standalone tests or another parent target.
get_filename_component(NEWAOE_ROOT_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../.."
    ABSOLUTE
)

set(NEWAOE_IMGUI_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/imgui")
set(NEWAOE_SOKOL_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/sokol")

foreach(required_file
    "${NEWAOE_IMGUI_DIR}/imgui.cpp"
    "${NEWAOE_SOKOL_DIR}/sokol_app.h"
    "${NEWAOE_SOKOL_DIR}/sokol_gfx.h"
    "${NEWAOE_SOKOL_DIR}/sokol_glue.h"
    "${NEWAOE_SOKOL_DIR}/sokol_log.h"
    "${NEWAOE_SOKOL_DIR}/util/sokol_imgui.h"
)
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR "Required render dependency is missing: ${required_file}")
    endif()
endforeach()

# Dear ImGui is compiled directly into the final executable. sokol_imgui.h
# supplies both its Sokol renderer and sokol_app event integration, so no
# separate platform backend is needed.
if(NOT TARGET newaoe_imgui)
    add_library(newaoe_imgui INTERFACE)
    add_library(NewAOE::imgui ALIAS newaoe_imgui)

    target_sources(newaoe_imgui INTERFACE
        "${NEWAOE_IMGUI_DIR}/imgui.cpp"
        "${NEWAOE_IMGUI_DIR}/imgui_draw.cpp"
        "${NEWAOE_IMGUI_DIR}/imgui_tables.cpp"
        "${NEWAOE_IMGUI_DIR}/imgui_widgets.cpp"
    )
    target_include_directories(newaoe_imgui SYSTEM INTERFACE
        "${NEWAOE_IMGUI_DIR}"
    )
    target_compile_features(newaoe_imgui INTERFACE cxx_std_17)
endif()

add_subdirectory(game/engine)

add_library(newaoe_render INTERFACE)
add_library(NewAOE::render ALIAS newaoe_render)

target_sources(newaoe_render INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/renderInstance.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/renderInstance.h"
)
target_include_directories(newaoe_render INTERFACE
    "${NEWAOE_ROOT_DIR}"
    "${NEWAOE_SOKOL_DIR}"
)
target_compile_features(newaoe_render INTERFACE cxx_std_17)
target_link_libraries(newaoe_render INTERFACE
    NewAOE::imgui
    NewAOE::game_render_engine
)

if(EMSCRIPTEN OR ANDROID)
    target_compile_definitions(newaoe_render INTERFACE SOKOL_GLES3)
elseif(WIN32)
    target_compile_definitions(newaoe_render INTERFACE SOKOL_D3D11)
elseif(APPLE)
    target_compile_definitions(newaoe_render INTERFACE SOKOL_METAL)
elseif(UNIX)
    target_compile_definitions(newaoe_render INTERFACE SOKOL_GLCORE)
else()
    message(FATAL_ERROR "The render module has no Sokol backend for this platform")
endif()

if(EMSCRIPTEN)
    target_link_options(newaoe_render INTERFACE
        "-sUSE_WEBGL2=1"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
    )
elseif(ANDROID)
    message(FATAL_ERROR
        "The Server executable currently owns main(), but sapp_run() with "
        "SOKOL_NO_ENTRY is not supported on Android"
    )
elseif(WIN32)
    target_link_libraries(newaoe_render INTERFACE
        kernel32
        user32
        shell32
        d3d11
        dxgi
    )
elseif(APPLE)
    enable_language(OBJCXX)
    set_source_files_properties(
        "${CMAKE_CURRENT_LIST_DIR}/renderInstance.cpp"
        PROPERTIES LANGUAGE OBJCXX
    )
    find_library(NEWAOE_QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    find_library(NEWAOE_METAL_FRAMEWORK Metal REQUIRED)
    if(IOS)
        find_library(NEWAOE_UIKIT_FRAMEWORK UIKit REQUIRED)
        find_library(NEWAOE_FOUNDATION_FRAMEWORK Foundation REQUIRED)
        find_library(NEWAOE_COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
        target_link_libraries(newaoe_render INTERFACE
            "${NEWAOE_UIKIT_FRAMEWORK}"
            "${NEWAOE_FOUNDATION_FRAMEWORK}"
            "${NEWAOE_COREGRAPHICS_FRAMEWORK}"
            "${NEWAOE_QUARTZCORE_FRAMEWORK}"
            "${NEWAOE_METAL_FRAMEWORK}"
        )
    else()
        find_library(NEWAOE_APPKIT_FRAMEWORK AppKit REQUIRED)
        target_link_libraries(newaoe_render INTERFACE
            "${NEWAOE_APPKIT_FRAMEWORK}"
            "${NEWAOE_QUARTZCORE_FRAMEWORK}"
            "${NEWAOE_METAL_FRAMEWORK}"
        )
    endif()
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED COMPONENTS Xi Xcursor)
    target_include_directories(newaoe_render SYSTEM INTERFACE
        "${X11_INCLUDE_DIR}"
    )
    target_link_libraries(newaoe_render INTERFACE
        OpenGL::GL
        X11::X11
        X11::Xi
        X11::Xcursor
        ${CMAKE_DL_LIBS}
        Threads::Threads
        m
    )
endif()
