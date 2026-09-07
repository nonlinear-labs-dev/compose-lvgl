cmake_minimum_required(VERSION 3.5...3.30)

project(compose-lvgl)

if (NOT DEFINED LVGL_BACKEND_SPECIFIC_SOURCES)
    set(LVGL_BACKEND_LIBS sdl2)
    set(LVGL_BACKEND_SPECIFIC_SOURCES "src/sdl/*.cpp")
endif ()

set(COMPOSE_DEFAULT_FONT_BASE_NAME "Nonlegible-Rubik" CACHE STRING "Default Font::baseName")
set(COMPOSE_DEFAULT_FONT_FALLBACK_BASE_NAMES "" CACHE STRING "Default Font::fallbackBaseNames, semicolon separated")

set(COMPOSE_DEFAULT_FONT_FALLBACK_INITIALIZER "")
if (NOT COMPOSE_DEFAULT_FONT_FALLBACK_BASE_NAMES STREQUAL "")
    set(quotedFallbacks ${COMPOSE_DEFAULT_FONT_FALLBACK_BASE_NAMES})
    list(TRANSFORM quotedFallbacks PREPEND "\"")
    list(TRANSFORM quotedFallbacks APPEND "\"")
    list(JOIN quotedFallbacks ", " COMPOSE_DEFAULT_FONT_FALLBACK_INITIALIZER)
endif ()
configure_file(include/compose/FontDefaults.h.in ${CMAKE_CURRENT_BINARY_DIR}/configured/compose/FontDefaults.h)

find_package(PkgConfig REQUIRED)
find_package(yaml-cpp REQUIRED)
if (NOT TARGET nlohmann_json::nlohmann_json)
    find_package(nlohmann_json REQUIRED)
endif ()
pkg_check_modules(LIBS REQUIRED freetype2 glibmm-2.4 giomm-2.4 ${LVGL_BACKEND_LIBS})

file(GLOB_RECURSE SOURCES src/common/*.cpp include/*.h ${LVGL_BACKEND_SPECIFIC_SOURCES})
add_library(compose-lvgl STATIC ${SOURCES})
target_compile_features(compose-lvgl PRIVATE cxx_std_23)
target_include_directories(compose-lvgl PUBLIC ./include ${LIBS_INCLUDE_DIRS})
target_link_libraries(compose-lvgl PUBLIC reactive lvgl lunasvg nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp ${LIBS_LIBRARIES})

target_include_directories(compose-lvgl PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/configured)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/configured/compose/FontDefaults.h DESTINATION include/compose)

option(COMPOSE_BUILD_EXAMPLES "build examples" Off)
if (COMPOSE_BUILD_EXAMPLES)
    add_subdirectory(example)
endif ()
