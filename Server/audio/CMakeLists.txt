find_package(Threads REQUIRED)

# Resolve the repository root from this module instead of relying on whichever
# parent project includes it. This also lets standalone tests reuse the target.
get_filename_component(NEWAOE_ROOT_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../.."
    ABSOLUTE
)

set(NEWAOE_AUDIO_MANAGER_DIR
    "${CMAKE_CURRENT_LIST_DIR}/audiomanager"
)

add_library(newaoe_audio INTERFACE)
add_library(NewAOE::audio ALIAS newaoe_audio)

# Propagate the module sources to the final consumer. This keeps audio reusable
# without producing a separate .lib/.a archive.
target_sources(newaoe_audio INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/audioInstance.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/audioInstance.h"
    "${NEWAOE_AUDIO_MANAGER_DIR}/audiomanager.cpp"
    "${NEWAOE_AUDIO_MANAGER_DIR}/audiomanager.h"
    "${NEWAOE_AUDIO_MANAGER_DIR}/miniaudio_impl.cpp"
)

target_include_directories(newaoe_audio INTERFACE
    "${NEWAOE_ROOT_DIR}"
)

target_compile_features(newaoe_audio INTERFACE cxx_std_17)

target_link_libraries(newaoe_audio INTERFACE
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

