get_filename_component(NEWAOE_ROOT_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../../.."
    ABSOLUTE
)

set(NEWAOE_SOKOL_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/sokol")
set(NEWAOE_STB_DIR "${NEWAOE_ROOT_DIR}/Public/third-party/stb")

foreach(required_file
    "${NEWAOE_SOKOL_DIR}/sokol_gfx.h"
    "${NEWAOE_STB_DIR}/stb_image.h"
    "${CMAKE_CURRENT_LIST_DIR}/shaders/engine.glsl.h"
)
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR "Required engine dependency is missing: ${required_file}")
    endif()
endforeach()

add_library(newaoe_game_render_engine INTERFACE)
add_library(NewAOE::game_render_engine ALIAS newaoe_game_render_engine)

target_sources(newaoe_game_render_engine INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/Camera2D.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Camera2D.h"
    "${CMAKE_CURRENT_LIST_DIR}/DeferredRenderer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/DeferredRenderer.h"
    "${CMAKE_CURRENT_LIST_DIR}/ParticleManager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ParticleManager.h"
    "${CMAKE_CURRENT_LIST_DIR}/RenderEngine.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/RenderEngine.h"
    "${CMAKE_CURRENT_LIST_DIR}/RenderTypes.h"
    "${CMAKE_CURRENT_LIST_DIR}/SpriteBatcher.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SpriteBatcher.h"
    "${CMAKE_CURRENT_LIST_DIR}/TextureCache.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/TextureCache.h"
    "${CMAKE_CURRENT_LIST_DIR}/particle/ParticleJson.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/particle/ParticleSystem.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/particle/ParticleSystem.h"
    "${CMAKE_CURRENT_LIST_DIR}/shaders/engine.glsl"
    "${CMAKE_CURRENT_LIST_DIR}/shaders/engine.glsl.h"
)

target_include_directories(newaoe_game_render_engine INTERFACE
    "${NEWAOE_ROOT_DIR}"
    "${NEWAOE_SOKOL_DIR}"
    "${NEWAOE_STB_DIR}"
)
target_compile_features(newaoe_game_render_engine INTERFACE cxx_std_17)

