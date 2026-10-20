file(GLOB_RECURSE NEWAOE_CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)

file(GLOB_RECURSE NEWAOE_PUBLIC_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/Public/*.cpp"
    "${PROJECT_SOURCE_DIR}/Public/*.cc"
    "${PROJECT_SOURCE_DIR}/Public/*.cxx"
)

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
    Boost::headers
)
