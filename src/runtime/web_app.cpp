#include "runtime/web_app.h"

#include "application/app.h"
#include "assets/asset_io.h"
#include "assets/avatars.h"
#include "assets/images.h"
#include "core/diagnostics.h"
#include "generated/fonts/geist_data.h"
#include "ui/controls/morph_slider.h"
#include "ui/effects/glass_cursor.h"
#include "ui/foundation/rounded_panel.h"
#include "ui/foundation/runtime.h"
#include "ui/foundation/theme.h"
#include "ui/foundation/typography/font_cache.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include <GLES3/gl3.h>
#include <SDL.h>
#include <emscripten.h>

#include <algorithm>

namespace solace::runtime
{
namespace
{
constexpr int k_canvas_width = 1120;
constexpr int k_canvas_height = 720;

// clang-format off
EM_JS(int, browser_prefers_dark, (), {
    const saved = window.localStorage.getItem('solace-theme');
    if (saved === 'light')
        return 0;
    if (saved === 'dark')
        return 1;
    return window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches ? 1 : 0;
});

EM_JS(void, browser_set_theme, (int dark), {
    const value = dark ? 'dark' : 'light';
    document.documentElement.dataset.theme = value;
    window.localStorage.setItem('solace-theme', value);
});

EM_JS(void, browser_ready, (), {
    if (window.SolaceBoot)
        window.SolaceBoot.ready();
});

EM_JS(void, browser_failed, (const char* message), {
    const text = UTF8ToString(message);
    if (window.SolaceBoot)
        window.SolaceBoot.fail(text);
});
// clang-format on

void warm_fonts()
{
    fonts.get(geist_regular, 12.f);
    fonts.get(geist_regular, 14.f);
    fonts.get(geist_regular, 16.f);
    fonts.get(geist_medium, 14.f);
    fonts.get(geist_medium, 16.f);
    fonts.get(geist_semibold, 20.f);
}

struct web_state
{
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    bool dark = true;
};

void draw_cursor()
{
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    glass::cursor(ImGui::GetForegroundDrawList(), ImRect(ImVec2(0.f, 0.f), display),
                  glass::settings());
}

void shutdown(web_state& state)
{
    images::shutdown();
    avatars::shutdown();
    glass::cursor_shutdown();
    rounded_panel::shutdown();
    slides::morph_slider_shutdown();
    ui_runtime::clear_animation_states();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    if (state.context)
        SDL_GL_DeleteContext(state.context);
    if (state.window)
        SDL_DestroyWindow(state.window);
    SDL_Quit();
}

void frame(void* argument)
{
    web_state& state = *static_cast<web_state*>(argument);
    SDL_Event event;
    while (SDL_PollEvent(&event))
        ImGui_ImplSDL2_ProcessEvent(&event);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    fonts.update();
    application::render_frame();
    ui_runtime::collect_animation_states();
    draw_cursor();

    if (state.dark != is_dark())
    {
        state.dark = is_dark();
        browser_set_theme(state.dark ? 1 : 0);
    }

    ImGui::Render();
    int drawable_width = 0;
    int drawable_height = 0;
    SDL_GL_GetDrawableSize(state.window, &drawable_width, &drawable_height);
    glViewport(0, 0, drawable_width, drawable_height);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(state.window);
}
} // namespace

int run_web_app()
{
    static web_state state;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        diagnostics::error("web", SDL_GetError());
        browser_failed(SDL_GetError());
        return diagnostics::to_process_exit_code(diagnostics::exit_code::window_initialization);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    state.window = SDL_CreateWindow(
        "Solace", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, k_canvas_width, k_canvas_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!state.window)
    {
        diagnostics::error("web", SDL_GetError());
        browser_failed(SDL_GetError());
        SDL_Quit();
        return diagnostics::to_process_exit_code(diagnostics::exit_code::window_initialization);
    }

    state.context = SDL_GL_CreateContext(state.window);
    if (!state.context)
    {
        diagnostics::error("webgl2", SDL_GetError());
        browser_failed(
            "WebGL2 context creation failed. This browser or GPU may not support WebGL2.");
        SDL_DestroyWindow(state.window);
        SDL_Quit();
        return diagnostics::to_process_exit_code(diagnostics::exit_code::renderer_initialization);
    }
    SDL_GL_MakeCurrent(state.window, state.context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    io.Fonts->TexMinWidth = 1024;
    io.Fonts->TexMinHeight = 1024;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().CircleTessellationMaxError = 0.10f;

    if (!ImGui_ImplSDL2_InitForOpenGL(state.window, state.context) ||
        !ImGui_ImplOpenGL3_Init("#version 300 es"))
    {
        browser_failed("Dear ImGui could not initialize its WebGL2 backend.");
        shutdown(state);
        return diagnostics::to_process_exit_code(diagnostics::exit_code::imgui_initialization);
    }

    ui_runtime::set_scale(1.f, false);
    state.dark = browser_prefers_dark() != 0;
    set_dark(state.dark);
    browser_set_theme(state.dark ? 1 : 0);
    fonts.install_kerning();
    warm_fonts();

    images::options slides_options;
    slides_options.max_edge = 1024;
    slides_options.aspect = 416.f / 650.f;
    slides_options.radius_ratio = 0.f;
    images::load_folder(asset_io::asset_directory(L"slides", L"SLIDES"), slides_options);
    avatars::load(asset_io::asset_directory(L"avatars", L"AVATARS"),
                  asset_io::asset_directory(L"logos", L"LOGOS"),
                  asset_io::asset_directory(L"brands", L"BRANDS"), nullptr, nullptr);
    const bool slider_ready = slides::morph_slider_init(nullptr, nullptr);
    const bool panel_ready = rounded_panel::init(nullptr, nullptr);
    const bool cursor_ready = glass::cursor_init(nullptr, nullptr);
    if (!slider_ready || !panel_ready || !cursor_ready)
        diagnostics::warning("web", "One or more optional visual effects could not initialize.");

    diagnostics::info("runtime", "Solace WebGL2 started.");
    browser_ready();
    emscripten_set_main_loop_arg(frame, &state, 0, true);
    return 0;
}
} // namespace solace::runtime
