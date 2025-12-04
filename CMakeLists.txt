cmake_minimum_required(VERSION 3.5...3.30)

project(compose-lvgl)

include(${CMAKE_BINARY_DIR}/cmake/NL.cmake)
include(${CMAKE_BINARY_DIR}/cmake/mallinfo.cmake)

if (NOT CROSS_BUILD)
    set(LVGL_BACKEND_LIBS sdl2)
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/sdl/*.cpp")
    SET(COMPOSE_FRAMEBUFFER_OFFSET_X 0)
    SET(COMPOSE_FRAMEBUFFER_OFFSET_Y 0)
else ()
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/framebuffer/*.cpp")
    SET(COMPOSE_FRAMEBUFFER_OFFSET_X 15)
    SET(COMPOSE_FRAMEBUFFER_OFFSET_Y 124)
endif ()

addNLLibrary(
    NAME compose-lvgl
    DESCRIPTION "compose lvgl"
    VERSION 1.0.0
    EXTERNAL_LIBS freetype2 glibmm-2.4 ${LVGL_BACKEND_LIBS}
    OUR_LIBS reactive nltools nltools-2 lvgl lunasvg
    PUBLIC_COMPILE_OPTIONS "-DCOMPOSE_FRAMEBUFFER_OFFSET_X=${COMPOSE_FRAMEBUFFER_OFFSET_X}" 
    PUBLIC_COMPILE_OPTIONS "-DCOMPOSE_FRAMEBUFFER_OFFSET_Y=${COMPOSE_FRAMEBUFFER_OFFSET_Y}"
    LIB_SOURCES src/common/*.cpp include/*.h ${LVGL_BACKEND_SPECIFIC_SOURCES}
    TEST_SOURCES tests/*.cpp
    INSTALL_HEADERS_DIR include
    IS_STATIC
    CPP_STANDARD 20
)
