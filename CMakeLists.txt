cmake_minimum_required(VERSION 3.5...3.30)

project(nl-compose-lvgl)

include(${CMAKE_BINARY_DIR}/cmake/NL.cmake)
include(${CMAKE_BINARY_DIR}/cmake/mallinfo.cmake)

if (NOT CROSS_BUILD)
    set(LVGL_BACKEND sdl2)
endif ()

addNLLibrary(
    NAME compose-lvgl
    DESCRIPTION "compose lvgl"
    VERSION 1.0.0
    EXTERNAL_LIBS freetype2 glibmm-2.4 ${LVGL_BACKEND}
    OUR_LIBS reactive nltools nltools-2 lvgl lunasvg
    LIB_SOURCES src/*.cpp include/*.h
    TEST_SOURCES tests/*.cpp
    INSTALL_HEADERS_DIR include
    IS_STATIC
    CPP_STANDARD 20
    AUTO_YOCTO_RECIPE
)

if (CROSS_BUILD)
    target_compile_definitions(compose-lvgl PUBLIC LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/my_lv_conf.h")
    target_compile_definitions(compose-lvgl PUBLIC CROSS_BUILD=1)
endif ()