# Each Server module owns its sources, include paths and dependencies. Optional
# modules are not configured at all when disabled, so none of their sources are
# compiled and none of their dependencies reach the Server executable.
option(ENABLE_AUDIO_PLAY "Build the Server audio module" ON)
option(ENABLE_GAME_RENDER "Build the Server render module" ON)

add_subdirectory(core)

set(NEWAOE_SERVER_MODULES
    NewAOE::core
)

if(ENABLE_AUDIO_PLAY)
    add_subdirectory(audio)
    list(APPEND NEWAOE_SERVER_MODULES NewAOE::audio)
endif()

if(ENABLE_GAME_RENDER)
    add_subdirectory(render)
    list(APPEND NEWAOE_SERVER_MODULES NewAOE::render)
endif()

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

add_executable(Server
    main.cpp
    conf.h
    ${NEWAOE_PUBLIC_SOURCES}
)

target_include_directories(Server PRIVATE
    "${PROJECT_SOURCE_DIR}"
)

target_compile_definitions(Server PRIVATE
    ENABLE_AUDIO_PLAY=$<BOOL:${ENABLE_AUDIO_PLAY}>
    ENABLE_GAME_RENDER=$<BOOL:${ENABLE_GAME_RENDER}>
)

target_link_libraries(Server PRIVATE
    NewAOE::boost_headers
    Threads::Threads
    NewAOE::profile
    NewAOE::logger
    ${NEWAOE_SERVER_MODULES}
)

if(WIN32)
    # Boost.Asio auto-links Winsock under MSVC, but MinGW requires the
    # Windows socket import library to be listed explicitly.
    target_link_libraries(Server PRIVATE ws2_32)
endif()

if(EMSCRIPTEN)
    # The Web build keeps the game, audio, and logger/profile workers as real
    # std::threads. Emscripten must see -pthread during both compilation and
    # linking, and workers must be pre-created because main() starts them
    # synchronously before handing rendering to the browser event loop.
    set(NEWAOE_WEB_PTHREAD_POOL_SIZE "6" CACHE STRING
        "Number of Emscripten pthread workers created before main()"
    )

    target_compile_options(Server PRIVATE
        "-pthread"
    )
    target_link_options(Server PRIVATE
        "-pthread"
        "-sPTHREAD_POOL_SIZE=${NEWAOE_WEB_PTHREAD_POOL_SIZE}"
        "-sALLOW_MEMORY_GROWTH=1"
    )
endif()
