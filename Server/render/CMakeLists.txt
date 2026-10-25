# Resolve the repository root locally so this module remains reusable from
# standalone tests or another parent target.
get_filename_component(NEWAOE_ROOT_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../.."
    ABSOLUTE
)

set(NEWAOE_SDL2_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/sdl")
set(NEWAOE_IMGUI_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/imgui")
set(NEWAOE_SOKOL_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/sokol")

if(NOT EXISTS "${NEWAOE_SDL2_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "The vendored SDL2 source tree was not found at ${NEWAOE_SDL2_DIR}"
    )
endif()
if(NOT EXISTS "${NEWAOE_IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR
        "The vendored Dear ImGui source tree was not found at ${NEWAOE_IMGUI_DIR}"
    )
endif()
if(NOT EXISTS "${NEWAOE_SOKOL_DIR}/sokol_gfx.h")
    message(FATAL_ERROR
        "The vendored sokol_gfx header was not found at ${NEWAOE_SOKOL_DIR}"
    )
endif()

# SDL is implemented in C. Configure the vendored copy only when another
# enabled target (for example the standalone SDL demo) has not done so yet.
if(NOT TARGET SDL2::SDL2-static)
    enable_language(C)
    set(SDL_SHARED OFF CACHE BOOL "Build SDL2 as a shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL2 as a static library" FORCE)
    set(SDL_TEST OFF CACHE BOOL "Build the SDL2 test library" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "Build the SDL2 test programs" FORCE)
    set(SDL2_DISABLE_INSTALL ON CACHE BOOL "Disable SDL2 install rules" FORCE)
    set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "Disable SDL2 uninstall target" FORCE)
    set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "Do not build the SDL2main shim" FORCE)
    if(MSVC)
        # Match the project's normal /MD or /MDd runtime selection. A static
        # SDL library does not require the static MSVC runtime.
        set(SDL_FORCE_STATIC_VCRT OFF CACHE BOOL
            "Use the same MSVC runtime as NewAOE" FORCE
        )
    endif()

    add_subdirectory(
        "${NEWAOE_SDL2_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/third-party/sdl2"
        EXCLUDE_FROM_ALL
    )
endif()

if(NOT TARGET SDL2::SDL2-static)
    message(FATAL_ERROR "The SDL2::SDL2-static target is unavailable")
endif()

if(EMSCRIPTEN)
    # SDL is a separate static target, so Server's private -pthread option
    # does not reach its C sources automatically.
    target_compile_options(SDL2-static PRIVATE "-pthread")
endif()

# ImGui is an interface target on purpose: its sources are compiled directly
# into the final executable, just like the rest of the Server modules. Only
# the SDL2 platform backend and SDL_Renderer2 backend are included.
if(NOT TARGET newaoe_imgui)
    add_library(newaoe_imgui INTERFACE)
    add_library(NewAOE::imgui ALIAS newaoe_imgui)

    target_sources(newaoe_imgui INTERFACE
        "${NEWAOE_IMGUI_DIR}/imgui.cpp"
        "${NEWAOE_IMGUI_DIR}/imgui_draw.cpp"
        "${NEWAOE_IMGUI_DIR}/imgui_tables.cpp"
        "${NEWAOE_IMGUI_DIR}/imgui_widgets.cpp"
        "${NEWAOE_IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
        "${NEWAOE_IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp"
    )

    target_include_directories(newaoe_imgui SYSTEM INTERFACE
        "${NEWAOE_IMGUI_DIR}"
        "${NEWAOE_IMGUI_DIR}/backends"
    )
    target_compile_features(newaoe_imgui INTERFACE cxx_std_17)
    target_link_libraries(newaoe_imgui INTERFACE SDL2::SDL2-static)
endif()

add_library(newaoe_render INTERFACE)
add_library(NewAOE::render ALIAS newaoe_render)

# Compile these sources as part of the final executable instead of creating a
# separate static archive for the module.
target_sources(newaoe_render INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/renderInstance.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/renderInstance.h"
)

target_include_directories(newaoe_render INTERFACE
    "${NEWAOE_ROOT_DIR}"
    "${NEWAOE_SOKOL_DIR}"
)

target_compile_features(newaoe_render INTERFACE cxx_std_17)
target_compile_definitions(newaoe_render INTERFACE SDL_MAIN_HANDLED)
target_link_libraries(newaoe_render INTERFACE NewAOE::imgui)

if(EMSCRIPTEN OR ANDROID)
    target_compile_definitions(newaoe_render INTERFACE
        AOE_GFX_GLES=1
        SOKOL_GLES3
    )
else()
    target_compile_definitions(newaoe_render INTERFACE
        AOE_GFX_GLES=0
        SOKOL_GLCORE
    )
endif()

if(EMSCRIPTEN)
    target_link_options(newaoe_render INTERFACE "-sUSE_WEBGL2=1")
elseif(ANDROID)
    target_link_libraries(newaoe_render INTERFACE GLESv3 EGL log android)
elseif(APPLE)
    enable_language(OBJCXX)
    set_source_files_properties(
        "${CMAKE_CURRENT_LIST_DIR}/renderInstance.cpp"
        PROPERTIES LANGUAGE OBJCXX
    )
    find_library(NEWAOE_OPENGL_FRAMEWORK OpenGL REQUIRED)
    target_link_libraries(newaoe_render INTERFACE
        "${NEWAOE_OPENGL_FRAMEWORK}"
    )
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(newaoe_render INTERFACE OpenGL::GL)
endif()

