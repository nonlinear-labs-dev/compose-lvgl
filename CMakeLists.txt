cmake_minimum_required(VERSION 3.5...3.30)

project(compose-lvgl)

set(CMAKE_CXX_STANDARD 23)

if (NOT CROSS_BUILD)
    set(LVGL_BACKEND_LIBS sdl2)
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/sdl/*.cpp")
else ()
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/framebuffer/*.cpp")
endif ()

find_package(PkgConfig REQUIRED)
find_package(yaml-cpp REQUIRED)
pkg_check_modules(LIBS REQUIRED freetype2 glibmm-2.4 giomm-2.4 ${LVGL_BACKEND_LIBS})

file(GLOB_RECURSE SOURCES src/common/*.cpp include/*.h ${LVGL_BACKEND_SPECIFIC_SOURCES})
list(APPEND SOURCES src/common/widgets/DragDrop.cpp)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ./include ${LIBS_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC reactive lvgl lunasvg nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp ${LIBS_LIBRARIES})
