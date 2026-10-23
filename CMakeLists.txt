cmake_minimum_required(VERSION 3.20)

project(NewAOE LANGUAGES CXX)

# Configure with: cmake -S . -B build -DAOE_BUILD=Server
# Valid values are Server, Client, and Both (case-insensitive).
set(AOE_BUILD "Both" CACHE STRING "Application to build: Server, Client, or Both")
set_property(CACHE AOE_BUILD PROPERTY STRINGS Server Client Both)

string(TOLOWER "${AOE_BUILD}" AOE_BUILD_NORMALIZED)
set(AOE_VALID_BUILD_VALUES server client both)
if(NOT AOE_BUILD_NORMALIZED IN_LIST AOE_VALID_BUILD_VALUES)
    message(FATAL_ERROR
        "Invalid AOE_BUILD value '${AOE_BUILD}'. "
        "Expected Server, Client, or Both."
    )
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED CONFIG)
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

add_library(newaoe_profile STATIC
    "${PROJECT_SOURCE_DIR}/Public/profile/profile_worker.cpp"
)

add_library(NewAOE::profile ALIAS newaoe_profile)
target_include_directories(newaoe_profile PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(newaoe_profile PUBLIC cxx_std_17)
target_link_libraries(newaoe_profile PUBLIC Threads::Threads Boost::headers)
if(WIN32)
    target_link_libraries(newaoe_profile PUBLIC ws2_32 mswsock)
endif()

add_library(newaoe_logger STATIC
    "${PROJECT_SOURCE_DIR}/Public/logger/logger_worker.cpp"
)
add_library(NewAOE::logger ALIAS newaoe_logger)
target_include_directories(newaoe_logger PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(newaoe_logger PUBLIC cxx_std_17)
target_link_libraries(newaoe_logger PUBLIC Threads::Threads)

option(AOE_BUILD_PROFILE_TESTS "Build the profile unit test" OFF)
if(AOE_BUILD_PROFILE_TESTS)
    enable_testing()
    add_executable(ProfileTests "${PROJECT_SOURCE_DIR}/test/profile_test.cpp")
    target_link_libraries(ProfileTests PRIVATE NewAOE::profile)
    add_test(NAME ProfileTests COMMAND ProfileTests)
endif()

option(AOE_BUILD_PROFILE_BENCHMARKS "Build the profile microbenchmark" OFF)
if(AOE_BUILD_PROFILE_BENCHMARKS)
    add_executable(ProfileBenchmark "${PROJECT_SOURCE_DIR}/test/profile_benchmark.cpp")
    target_link_libraries(ProfileBenchmark PRIVATE NewAOE::profile)
endif()

if(MSVC)
    add_link_options(
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:/OPT:REF>"
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:/OPT:ICF>"
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

message(STATUS "NewAOE build selection: ${AOE_BUILD}")
message(STATUS "Executable output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
