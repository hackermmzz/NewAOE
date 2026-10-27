file(GLOB_RECURSE NEWAOE_CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)

file(GLOB_RECURSE NEWAOE_PUBLIC_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/Public/network/*.cpp"
    "${PROJECT_SOURCE_DIR}/Public/network/*.cc"
    "${PROJECT_SOURCE_DIR}/Public/network/*.cxx"
    "${PROJECT_SOURCE_DIR}/Public/serializer/*.cpp"
    "${PROJECT_SOURCE_DIR}/Public/serializer/*.cc"
    "${PROJECT_SOURCE_DIR}/Public/serializer/*.cxx"
    "${PROJECT_SOURCE_DIR}/Public/type/*.cpp"
    "${PROJECT_SOURCE_DIR}/Public/type/*.cc"
    "${PROJECT_SOURCE_DIR}/Public/type/*.cxx"
    "${PROJECT_SOURCE_DIR}/Public/util/*.cpp"
    "${PROJECT_SOURCE_DIR}/Public/util/*.cc"
    "${PROJECT_SOURCE_DIR}/Public/util/*.cxx"
    "${PROJECT_SOURCE_DIR}/Public/vec/*.cpp"
    "${PROJECT_SOURCE_DIR}/Public/vec/*.cc"
    "${PROJECT_SOURCE_DIR}/Public/vec/*.cxx"
)

# Shared feature targets contribute profiler/logger sources only when their
# independent build switches are enabled.

if(NOT NEWAOE_CLIENT_SOURCES)
    message(FATAL_ERROR "No C++ source files were found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

add_executable(Client
    ${NEWAOE_CLIENT_SOURCES}
    ${NEWAOE_PUBLIC_SOURCES}
)

target_include_directories(Client PRIVATE
    "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(Client PRIVATE
    NewAOE::boost_headers
    NewAOE::fixed_point
    Threads::Threads
    NewAOE::profile
    NewAOE::logger
)
