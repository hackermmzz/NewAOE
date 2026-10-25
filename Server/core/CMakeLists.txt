# Resolve the repository root locally so this module remains reusable from
# standalone tests or another parent target.
get_filename_component(NEWAOE_ROOT_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../.."
    ABSOLUTE
)

add_library(newaoe_core INTERFACE)
add_library(NewAOE::core ALIAS newaoe_core)

# Compile these sources as part of the final executable instead of creating a
# separate static archive for the module.
target_sources(newaoe_core INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/coreInstance.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/coreInstance.h"
)

target_include_directories(newaoe_core INTERFACE
    "${NEWAOE_ROOT_DIR}"
)

target_compile_features(newaoe_core INTERFACE cxx_std_17)

