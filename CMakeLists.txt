cmake_minimum_required(VERSION 3.10)
project(Blip)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all third-party libs statically" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL tests" FORCE)
set(SDL_SHARED OFF CACHE BOOL "Build static SDL" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build static SDL" FORCE)
set(SDL2TTF_SAMPLES OFF CACHE BOOL "Disable SDL_ttf samples" FORCE)

if (UNIX AND NOT APPLE)
    set(SDL_AUDIO OFF CACHE BOOL "Disable SDL Audio subsystem" FORCE)
endif()

add_subdirectory(third_party/SDL2)
add_subdirectory(third_party/SDL2_ttf)

# Tree-sitter source definitions
set(TS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter")

set(TS_SOURCES
    "${TS_ROOT}/lib/src/lib.c"
)

set(CORE_SOURCES
    src/config/editor.cpp
    src/core/log.cpp
    src/buffer/table.cpp
    src/buffer/buffer.cpp
    src/text/font_manager.cpp
    src/text/typesetter.cpp
    src/input/vim_engine.cpp
    src/ui/renderer.cpp
    src/ui/window.cpp
    src/platform/system.cpp
    src/ui/glyph_cache.cpp
    src/text/syntax.cpp
    ${TS_SOURCES}
)

if (APPLE)
    list(APPEND CORE_SOURCES src/platform/darwin/watcher.cpp)
    list(APPEND CORE_SOURCES src/platform/darwin/system.cpp)
    find_library(CORETEXT_LIBRARY CoreText)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    set(PLATFORM_LIBS ${CORETEXT_LIBRARY} ${COREFOUNDATION_LIBRARY})
elseif (UNIX AND NOT APPLE)
    list(APPEND CORE_SOURCES src/platform/linux/watcher.cpp)
    list(APPEND CORE_SOURCES src/platform/linux/system.cpp)
    find_package(Fontconfig REQUIRED)
    set(PLATFORM_LIBS Fontconfig::Fontconfig)
endif()

function(add_blip_executable name entry_point)
    add_executable(${name} ${entry_point} ${CORE_SOURCES})

    target_include_directories(${name} PRIVATE
        include
        "${TS_ROOT}/lib/include"
    )

    target_link_libraries(${name}
        PRIVATE
            SDL2-static
            SDL2_ttf
            ${PLATFORM_LIBS}
            ${CMAKE_DL_LIBS}
    )
endfunction()

add_blip_executable(Blip src/app/main.cpp)
add_blip_executable(BlipDev src/app/main.cpp)
target_compile_definitions(BlipDev PRIVATE _DEV_)
add_blip_executable(BlipTests tests/tests.cpp)
