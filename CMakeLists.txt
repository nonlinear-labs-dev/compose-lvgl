cmake_minimum_required(VERSION 3.5...3.30)

project(compose-lvgl)

set(CMAKE_CXX_STANDARD 23)

if (NOT COMMAND addNLLibrary AND EXISTS ${CMAKE_BINARY_DIR}/cmake/NL.cmake)
    include(${CMAKE_BINARY_DIR}/cmake/NL.cmake)
    include(${CMAKE_BINARY_DIR}/cmake/mallinfo.cmake)
endif ()

if (NOT CROSS_BUILD)
    set(LVGL_BACKEND_LIBS sdl2)
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/sdl/*.cpp")
else ()
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/framebuffer/*.cpp")
endif ()

set(COMPOSE_DEFAULT_FONT_BASE_NAME "Nonlegible-Rubik" CACHE STRING "Default Font::baseName")
set(COMPOSE_DEFAULT_FONT_FALLBACK_BASE_NAMES "" CACHE STRING "Default Font::fallbackBaseNames, semicolon separated")

set(COMPOSE_DEFAULT_FONT_FALLBACK_INITIALIZER "\"\"")
if (NOT COMPOSE_DEFAULT_FONT_FALLBACK_BASE_NAMES STREQUAL "")
    set(quotedFallbacks ${COMPOSE_DEFAULT_FONT_FALLBACK_BASE_NAMES})
    list(TRANSFORM quotedFallbacks PREPEND "\"")
    list(TRANSFORM quotedFallbacks APPEND "\"")
    list(JOIN quotedFallbacks ", " COMPOSE_DEFAULT_FONT_FALLBACK_INITIALIZER)
endif ()
configure_file(include/compose/FontDefaults.h.in ${CMAKE_CURRENT_BINARY_DIR}/configured/compose/FontDefaults.h)

if (COMMAND addNLLibrary)
    addNLLibrary(
            NAME compose-lvgl
            DESCRIPTION "compose lvgl"
            VERSION 1.0.0
            EXTERNAL_LIBS freetype2 glibmm-2.4 giomm-2.4 yaml-cpp nlohmann_json ${LVGL_BACKEND_LIBS}
            OUR_LIBS reactive lvgl lunasvg
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
else ()
    find_package(PkgConfig REQUIRED)
    find_package(yaml-cpp REQUIRED)
    if (NOT TARGET nlohmann_json::nlohmann_json)
        find_package(nlohmann_json REQUIRED)
    endif ()
    pkg_check_modules(LIBS REQUIRED freetype2 glibmm-2.4 giomm-2.4 ${LVGL_BACKEND_LIBS})

    file(GLOB_RECURSE SOURCES src/common/*.cpp include/*.h ${LVGL_BACKEND_SPECIFIC_SOURCES})
    add_library(${PROJECT_NAME} STATIC ${SOURCES})

    target_include_directories(${PROJECT_NAME} PUBLIC ./include ${LIBS_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC reactive lvgl lunasvg nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp ${LIBS_LIBRARIES})
endif ()

target_include_directories(compose-lvgl PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/configured)
