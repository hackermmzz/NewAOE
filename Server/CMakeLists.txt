file(GLOB_RECURSE NEWAOE_SERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)

file(GLOB_RECURSE NEWAOE_PUBLIC_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/Public/*.cpp"
    "${PROJECT_SOURCE_DIR}/Public/*.cc"
    "${PROJECT_SOURCE_DIR}/Public/*.cxx"
)

# Vendored sources stay out of this executable.
list(FILTER NEWAOE_PUBLIC_SOURCES EXCLUDE REGEX "/third-party/")
# The profiler and logger are built once as shared project dependencies.
list(FILTER NEWAOE_PUBLIC_SOURCES EXCLUDE REGEX "/profile/")
list(FILTER NEWAOE_PUBLIC_SOURCES EXCLUDE REGEX "/logger/")

if(NOT NEWAOE_SERVER_SOURCES)
    message(FATAL_ERROR "No C++ source files were found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

add_executable(Server
    ${NEWAOE_SERVER_SOURCES}
    ${NEWAOE_PUBLIC_SOURCES}
)

target_include_directories(Server PRIVATE
    "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(Server PRIVATE
    Boost::headers
    Threads::Threads
    NewAOE::profile
    NewAOE::logger
)
