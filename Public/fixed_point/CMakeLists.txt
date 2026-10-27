add_library(newaoe_fixed_point INTERFACE)
add_library(NewAOE::fixed_point ALIAS newaoe_fixed_point)

target_sources(newaoe_fixed_point INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/fixed.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/macro.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/math.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/type.hpp"
)

# Headers use repository-root-relative includes such as
# <Public/fixed_point/include.h> and <Public/util/util.h>.
target_include_directories(newaoe_fixed_point SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)
target_compile_features(newaoe_fixed_point INTERFACE cxx_std_17)
