cmake_minimum_required(VERSION 3.5...3.30)

project(nl-compose-lvgl)

include(${CMAKE_BINARY_DIR}/cmake/NL.cmake)
include(${CMAKE_BINARY_DIR}/cmake/mallinfo.cmake)

addNLLibrary(
        NAME compose-lvgl
        DESCRIPTION "compose lvgl"
        VERSION 1.0.0
        EXTERNAL_LIBS sdl2 freetype2
        OUR_LIBS reactive nltools nltools-2 lvgl lunasvg
        LIB_SOURCES src/*.cpp include/*.h
        TEST_SOURCES tests/*.cpp
        INSTALL_HEADERS_DIR include

        IS_STATIC
        CPP_STANDARD 20
)

target_compile_options(compose-lvgl PRIVATE -fsanitize=address)
target_link_options(compose-lvgl PRIVATE -fsanitize=address)
