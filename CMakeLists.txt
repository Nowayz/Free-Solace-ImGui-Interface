cmake_minimum_required(VERSION 3.24)

project(SolaceWeb LANGUAGES CXX)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "The web target requires Emscripten. Configure it with emcmake cmake.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

set(SOLACE_SOURCES
    src/main.cpp
    src/application/app.cpp
    src/assets/asset_io.cpp
    src/assets/avatars_web.cpp
    src/assets/images_web.cpp
    src/auth/auth_layout.cpp
    src/auth/legal.cpp
    src/auth/signin_form.cpp
    src/auth/signup_form.cpp
    src/auth/validation.cpp
    src/core/diagnostics.cpp
    src/core/environment.cpp
    src/generated/fonts/geist_data.cpp
    src/graphics/gl_texture.cpp
    src/graphics/snapshot_web.cpp
    src/runtime/web_app.cpp
    src/ui/controls/caret.cpp
    src/ui/controls/form_controls.cpp
    src/ui/controls/loader.cpp
    src/ui/controls/morph_slider_web.cpp
    src/ui/controls/scroll.cpp
    src/ui/controls/theme_toggle.cpp
    src/ui/controls/widgets.cpp
    src/ui/effects/glass_cursor_web.cpp
    src/ui/foundation/draw.cpp
    src/ui/foundation/icons.cpp
    src/ui/foundation/primitives.cpp
    src/ui/foundation/rounded_panel_web.cpp
    src/ui/foundation/runtime.cpp
    src/ui/foundation/svg.cpp
    src/ui/foundation/theme.cpp
    src/ui/foundation/motion/motion.cpp
    src/ui/foundation/typography/font_cache.cpp
    src/ui/foundation/typography/kerning.cpp
    src/ui/screens/page_renderer.cpp
    src/ui/screens/search_overlay.cpp
    src/ui/screens/shell_layout.cpp
    src/ui/screens/shell_menus.cpp
    src/ui/screens/shell_screen.cpp
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/backends/imgui_impl_sdl2.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(solace-web ${SOLACE_SOURCES})
set_target_properties(solace-web PROPERTIES
    OUTPUT_NAME index
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist"
)

target_include_directories(solace-web PRIVATE
    src
    thirdparty/imgui
    thirdparty/imgui/backends
)

target_compile_definitions(solace-web PRIVATE
    IMGUI_DEFINE_MATH_OPERATORS
    IMGUI_IMPL_OPENGL_ES3
    SOLACE_WEBGL2
)

target_compile_options(solace-web PRIVATE
    -O3
    -sUSE_SDL=2
    -Wall
    -Wextra
    -Wpedantic
)

target_link_options(solace-web PRIVATE
    -O3
    -sUSE_SDL=2
    -sMIN_WEBGL_VERSION=2
    -sMAX_WEBGL_VERSION=2
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=67108864
    -sSTACK_SIZE=5242880
    -sENVIRONMENT=web
    -sFILESYSTEM=1
    -sEXIT_RUNTIME=0
    -sASSERTIONS=0
    -sMALLOC=emmalloc
    --preload-file
    "${CMAKE_SOURCE_DIR}/assets@/assets"
    --shell-file
    "${CMAKE_SOURCE_DIR}/web/shell.html"
)

add_custom_command(TARGET solace-web POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_SOURCE_DIR}/LICENSE"
            "$<TARGET_FILE_DIR:solace-web>/LICENSE.txt"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
            "$<TARGET_FILE_DIR:solace-web>/THIRD_PARTY_NOTICES.md"
)
