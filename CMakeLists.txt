cmake_minimum_required(VERSION 3.5...3.30)

project(compose-lvgl)

include(${CMAKE_BINARY_DIR}/cmake/NL.cmake)
include(${CMAKE_BINARY_DIR}/cmake/mallinfo.cmake)

if (NOT CROSS_BUILD)
    set(LVGL_BACKEND_LIBS sdl2)
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/sdl/*.cpp")
else ()
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/framebuffer/*.cpp")
endif ()

addNLLibrary(
        NAME compose-lvgl
        DESCRIPTION "compose lvgl"
        VERSION 1.0.0
        EXTERNAL_LIBS freetype2 glibmm-2.4 ${LVGL_BACKEND_LIBS}
        OUR_LIBS reactive nltools nltools-2 lvgl lunasvg
        LIB_SOURCES src/common/*.cpp include/*.h ${LVGL_BACKEND_SPECIFIC_SOURCES}
        TEST_SOURCES tests/test_basewidget.cpp tests/test_stylesheet_json.cpp
        INSTALL_HEADERS_DIR include
        IS_STATIC
        CPP_STANDARD 20
)

if (NOT CROSS_BUILD)
    addNLExecutable(
            NAME compose-lvgl-virtual-list-demo
            DESCRIPTION "compose-lvgl virtual list demo"
            VERSION 0.0.1
            EXTERNAL_LIBS sdl2
            OUR_LIBS compose-lvgl
            SUBMODULE_LIBS reactive
            LIB_SOURCES tests/manual/VirtualListDemoApp.h tests/manual/VirtualListDemoApp.cpp
            EXECUTABLE_SOURCES tests/manual/VirtualListDemoMain.cpp
            CPP_STANDARD 20
            USE_PTHREADS
    )
endif ()
