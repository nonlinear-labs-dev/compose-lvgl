cmake_minimum_required(VERSION 3.5...3.30)

project(nl-compose-lvgl)

include(${CMAKE_BINARY_DIR}/cmake/NL.cmake)
include(${CMAKE_BINARY_DIR}/cmake/mallinfo.cmake)

addNLLibrary(
        NAME compose-lvgl
        DESCRIPTION "compose lvgl"
        VERSION 1.0.0
        OUR_LIBS reactive nltools nltools-2 lvgl
        LIB_SOURCES src/*.cpp include/*.h
        INSTALL_HEADERS_DIR include
        CPP_STANDARD 20
)
