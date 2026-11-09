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
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/CrashBox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/CrashBox.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Object.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Object.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/MoveObject.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/MoveObject.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/BloodHaver.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/BloodHaver.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Attacker.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Attacker.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Resource.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Resource.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Human.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Human.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Farmer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Farmer.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Army.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Army.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Building.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Building.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/BuildingResource.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/BuildingResource.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Missile.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Missile.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Animal.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/Animal.h"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/StaticRes.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/BaseObject/StaticResource.h"
    "${CMAKE_CURRENT_LIST_DIR}/Module/Player/Player.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Module/Player/Player.h"
)

target_include_directories(newaoe_core INTERFACE
    "${NEWAOE_ROOT_DIR}"
)

target_compile_features(newaoe_core INTERFACE cxx_std_17)

