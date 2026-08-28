#include "ui/effects/glass_cursor.h"

#include "ui/foundation/theme.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace solace::glass
{
namespace
{
struct trail_state
{
    std::vector<ImVec2> points;
    ImVec2 smoothed{};
    float presence = 0.f;
    bool seeded = false;
};

trail_state& state()
{
    static trail_state value;
    return value;
}

ImU32 alpha_colour(ImU32 colour, float alpha)
{
    const ImVec4 value = ImGui::ColorConvertU32ToFloat4(colour);
    return ImGui::ColorConvertFloat4ToU32(
        ImVec4(value.x, value.y, value.z, value.w * std::clamp(alpha, 0.f, 1.f)));
}
} // namespace

cursor_options& settings()
{
    static cursor_options options;
    return options;
}

bool& enabled()
{
    static bool value = true;
    return value;
}

bool cursor_live()
{
    return enabled() && state().seeded && state().presence > 0.05f;
}

bool cursor_init(ID3D11Device*, ID3D11DeviceContext*)
{
    return true;
}

void cursor_shutdown()
{
    state() = {};
}

void cursor(ImDrawList* draw_list, const ImRect& viewport, const cursor_options& options)
{
    if (!draw_list || !enabled() || options.opacity <= 0.001f)
        return;

    ImGuiIO& io = ImGui::GetIO();
    trail_state& trail = state();
    const ImVec2 pointer = options.follow_pointer ? io.MousePos : options.pointer;
    const bool present = pointer.x > -FLT_MAX * 0.5f && viewport.Contains(pointer);
    const float dt = std::clamp(io.DeltaTime, 0.f, 0.1f);
    trail.presence += ((present ? 1.f : 0.f) - trail.presence) * std::min(1.f, dt * 8.f);

    if (!present && trail.presence < 0.01f)
    {
        trail = {};
        return;
    }
    if (present)
    {
        if (!trail.seeded)
        {
            trail.seeded = true;
            trail.smoothed = pointer;
            trail.points.assign(2, pointer);
        }
        const float dampening = std::clamp(options.dampening, 0.f, 0.999f);
        const float response = dampening <= 0.f ? 1.f : 1.f - std::pow(dampening, dt * 60.f);
        trail.smoothed.x += (pointer.x - trail.smoothed.x) * response;
        trail.smoothed.y += (pointer.y - trail.smoothed.y) * response;
        trail.points.insert(trail.points.begin(), trail.smoothed);
    }

    const int wanted = std::clamp(options.trail_length / 4, 3, 16);
    if (static_cast<int>(trail.points.size()) > wanted)
        trail.points.resize(static_cast<std::size_t>(wanted));

    const float base_radius = std::max(solace::px(options.blob_radius) * 0.48f, 4.f);
    for (int index = static_cast<int>(trail.points.size()) - 1; index >= 0; --index)
    {
        const float progress =
            trail.points.size() > 1 ? static_cast<float>(index) / (trail.points.size() - 1) : 0.f;
        const float fade = (1.f - progress) * options.opacity * trail.presence;
        const float radius = base_radius * (1.f - options.tail_fade * progress * 0.7f);
        draw_list->AddCircleFilled(trail.points[static_cast<std::size_t>(index)], radius,
                                   alpha_colour(c_foreground, fade * 0.14f), 24);
        draw_list->AddCircle(trail.points[static_cast<std::size_t>(index)], radius,
                             alpha_colour(c_foreground, fade * 0.36f), 0, solace::px(1.f));
    }

    if (!trail.points.empty())
        draw_list->AddCircleFilled(trail.points.front(), std::max(solace::px(2.2f), 1.5f),
                                   alpha_colour(c_foreground, trail.presence * 0.78f), 16);
}
} // namespace solace::glass
