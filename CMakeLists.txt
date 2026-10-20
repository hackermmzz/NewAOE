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

if(MSVC)
    add_compile_options(/utf-8)
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
