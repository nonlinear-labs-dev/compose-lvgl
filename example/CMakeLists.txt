option(COMPOSE_EXAMPLES_DOWNLOAD_MISSING_DEPENDENCIES "download dependencies if missing" Off)
if (COMPOSE_EXAMPLES_DOWNLOAD_MISSING_DEPENDENCIES)
    if (NOT TARGET lvgl)
        include(FetchContent)
        FetchContent_Declare(
                lvgl_imported
                GIT_REPOSITORY https://github.com/lvgl/lvgl.git
                GIT_TAG v9.5.0
                EXCLUDE_FROM_ALL
        )
        FetchContent_MakeAvailable(lvgl_imported)
        target_compile_definitions(lvgl PRIVATE LV_BUILD_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../sdl_lv_config.h")
    endif ()
    if (NOT TARGET lunasvg)
        include(FetchContent)
        FetchContent_Declare(
                lunasvg_imported
                GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
                GIT_TAG v3.5.0
                EXCLUDE_FROM_ALL
        )
        FetchContent_MakeAvailable(lunasvg_imported)
    endif()
    if (NOT TARGET reactive)
        include(FetchContent)
        FetchContent_Declare(
                reactive_imported
                GIT_REPOSITORY https://github.com/nonlinear-labs-dev/reactive.git
                GIT_TAG f851f9d606bdad13ae3145ffbc3b8364a09c4f99
                EXCLUDE_FROM_ALL
        )
        FetchContent_MakeAvailable(reactive_imported)
    endif()
endif ()

add_executable(compose-lvgl-virtual-list-demo src/virtual-list-demo/VirtualListDemoMain.cpp)
target_link_libraries(compose-lvgl-virtual-list-demo PRIVATE compose-lvgl)
target_sources(compose-lvgl-virtual-list-demo PRIVATE src/virtual-list-demo/VirtualListDemoApp.cpp)
target_compile_features(compose-lvgl-virtual-list-demo PRIVATE cxx_std_23)

add_executable(compose-svg-demo src/svg_demo.cpp)
target_link_libraries(compose-svg-demo PRIVATE compose-lvgl)
target_compile_features(compose-svg-demo PRIVATE cxx_std_23)
