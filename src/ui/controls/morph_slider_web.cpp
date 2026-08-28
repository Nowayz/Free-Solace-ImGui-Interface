#include "ui/controls/morph_slider.h"

#include "assets/images.h"
#include "ui/foundation/motion/motion.h"
#include "ui/foundation/primitives.h"
#include "ui/foundation/theme.h"

#include <algorithm>
#include <cmath>

namespace solace::slides
{
namespace
{
struct slider_state
{
    int current = 0;
    int next = 0;
    int direction = 1;
    float progress = 1.f;
    float since_change = 0.f;
    float caption = 1.f;
    bool animating = false;
    bool hovered = false;
};

slider_state& state()
{
    static slider_state value;
    return value;
}

morph_slider_status& status()
{
    static morph_slider_status value;
    return value;
}

float ease(float value)
{
    value = ImClamp(value, 0.f, 1.f);
    return value < 0.5f ? 4.f * value * value * value : 1.f - powf(-2.f * value + 2.f, 3.f) * 0.5f;
}

ImU32 faded(ImU32 colour, float alpha)
{
    return mo::with_alpha(colour, ImClamp(alpha, 0.f, 1.f));
}
} // namespace

bool morph_slider_init(ID3D11Device*, ID3D11DeviceContext*)
{
    return true;
}

void morph_slider_shutdown()
{
    state() = {};
    status() = {};
}

const morph_slider_status& morph_slider_state()
{
    return status();
}

void morph_slider(ImDrawList* draw_list, const ImRect& rect, const ImRect&, float card_rounding,
                  const morph_slider_options& options)
{
    const std::vector<images::texture>& textures = images::ready();
    const int count = static_cast<int>(textures.size());
    if (!draw_list || count == 0 || rect.GetWidth() <= 1.f || rect.GetHeight() <= 1.f)
        return;

    slider_state& slider = state();
    slider.current = ImClamp(slider.current, 0, count - 1);
    slider.next = ImClamp(slider.next, 0, count - 1);
    const float dt = ImGui::GetIO().DeltaTime;

    auto go = [&](int direction, int requested = -1)
    {
        if (slider.animating || count < 2)
            return;
        int target = requested >= 0 ? requested : slider.next + direction;
        if (!options.loop && (target < 0 || target >= count))
            return;
        target = ((target % count) + count) % count;
        if (target == slider.next)
            return;
        slider.current = slider.next;
        slider.next = target;
        slider.direction = direction;
        slider.progress = 0.f;
        slider.since_change = 0.f;
        slider.caption = 0.f;
        slider.animating = true;
    };

    if (slider.animating)
    {
        slider.progress += dt / ImMax(options.duration, 0.05f);
        if (slider.progress >= 1.f)
        {
            slider.progress = 1.f;
            slider.animating = false;
            slider.current = slider.next;
        }
    }
    else if (!(options.pause_on_hover && slider.hovered))
    {
        slider.since_change += dt;
        if (options.autoplay && count > 1 &&
            slider.since_change >= ImMax(options.autoplay_delay, 1.f))
            go(1);
    }
    slider.caption = ImMin(slider.caption + dt / ImMax(options.duration * 0.66f, 0.05f), 1.f);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGui::PushID("morph-slider-web");
    const ImGuiID stage_id = window->GetID("stage");
    ImGui::SetCursorScreenPos(rect.Min);
    ImGui::ItemSize(ImVec2(0.f, 0.f));
    ImGui::ItemAdd(rect, stage_id);
    bool held = false;
    ImGui::ButtonBehavior(rect, stage_id, &slider.hovered, &held, ImGuiButtonFlags_MouseButtonLeft);

    if (options.keyboard && slider.hovered)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, true))
            go(1);
        else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true))
            go(-1);
    }
    if (held)
    {
        const float drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
        if (ImFabs(drag) > px(48.f))
        {
            go(drag < 0.f ? 1 : -1);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
    }

    const float transition = slider.animating ? ease(slider.progress) : 1.f;
    const images::texture& current = textures[static_cast<std::size_t>(slider.current)];
    const images::texture& next = textures[static_cast<std::size_t>(slider.next)];
    draw_list->PushClipRect(rect.Min, rect.Max, true);
    draw_list->AddImageRounded(ImTextureRef(current.id), rect.Min, rect.Max, ImVec2(0.f, 0.f),
                               ImVec2(1.f, 1.f), IM_COL32_WHITE, card_rounding);
    if (slider.animating)
    {
        const float offset = px(14.f) * (1.f - transition) * slider.direction;
        draw_list->AddImageRounded(
            ImTextureRef(next.id), ImVec2(rect.Min.x + offset, rect.Min.y),
            ImVec2(rect.Max.x + offset, rect.Max.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f),
            IM_COL32(255, 255, 255, static_cast<int>(transition * 255.f + 0.5f)), card_rounding);
    }
    draw_list->AddRectFilled(rect.Min, rect.Max, faded(options.overlay, 0.11f), card_rounding);
    draw_list->PopClipRect();

    const int shown = slider.animating ? slider.next : slider.current;
    morph_slider_status& public_status = status();
    public_status.shown = shown;
    public_status.previous = slider.animating ? slider.current : shown;
    public_status.count = count;
    public_status.animating = slider.animating;
    public_status.progress = transition;
    public_status.swap = slider.caption;
    public_status.until_next =
        options.autoplay && count > 1
            ? ImClamp(slider.since_change / ImMax(options.autoplay_delay, 1.f), 0.f, 1.f)
            : 0.f;

    if (options.show_captions && !textures[static_cast<std::size_t>(shown)].name.empty())
    {
        const float reveal = mo::EASE_OUT(slider.caption);
        ImFont* font = font_semibold(15.f);
        const char* label = textures[static_cast<std::size_t>(shown)].name.c_str();
        const float width = text_width(font, label);
        const ImVec2 at(rect.Min.x + px(22.f), rect.Max.y - px(59.f) + px(8.f) * (1.f - reveal));
        const ImRect pill(at, ImVec2(at.x + width + px(28.f), at.y + px(37.f)));
        draw_list->AddRectFilled(pill.Min, pill.Max,
                                 faded(IM_COL32(10, 10, 12, 255), 0.48f * reveal), px(10.f));
        draw_list->AddText(font, font->LegacySize,
                           ImVec2(at.x + px(14.f), pill.GetCenter().y - font->LegacySize * 0.5f),
                           faded(IM_COL32_WHITE, reveal), label);
    }

    if (options.show_controls && count > 1)
    {
        const float diameter = px(40.f);
        for (int side = 0; side < 2; ++side)
        {
            const int direction = side == 0 ? -1 : 1;
            const float x = side == 0 ? rect.Min.x + px(16.f) : rect.Max.x - px(16.f) - diameter;
            const ImRect button(ImVec2(x, rect.GetCenter().y - diameter * 0.5f),
                                ImVec2(x + diameter, rect.GetCenter().y + diameter * 0.5f));
            ImGui::PushID(side);
            const ImGuiID id = window->GetID("arrow");
            ImGui::SetCursorScreenPos(button.Min);
            ImGui::ItemSize(ImVec2(0.f, 0.f));
            ImGui::ItemAdd(button, id);
            bool hovered = false;
            bool button_held = false;
            if (ImGui::ButtonBehavior(button, id, &hovered, &button_held))
                go(direction);
            ImGui::PopID();

            draw_list->AddCircleFilled(button.GetCenter(), diameter * 0.5f,
                                       faded(IM_COL32(12, 12, 14, 255), hovered ? 0.64f : 0.44f),
                                       28);
            const float glyph = px(6.f);
            const ImVec2 center = button.GetCenter();
            const float sign = static_cast<float>(direction);
            draw_list->AddLine(ImVec2(center.x - sign * glyph * 0.45f, center.y - glyph),
                               ImVec2(center.x + sign * glyph * 0.55f, center.y), IM_COL32_WHITE,
                               px(2.f));
            draw_list->AddLine(ImVec2(center.x + sign * glyph * 0.55f, center.y),
                               ImVec2(center.x - sign * glyph * 0.45f, center.y + glyph),
                               IM_COL32_WHITE, px(2.f));
        }
    }

    if (options.show_indicators && count > 1)
    {
        const float dot = px(8.f);
        const float gap = px(8.f);
        float total = 0.f;
        for (int index = 0; index < count; ++index)
            total += (index == shown ? px(22.f) : dot) + (index + 1 < count ? gap : 0.f);
        float x = options.indicators_right ? rect.Max.x - px(options.indicator_pad_x) - total
                                           : rect.GetCenter().x - total * 0.5f;
        const float y = rect.Max.y - px(options.indicator_pad_y) - dot;
        for (int index = 0; index < count; ++index)
        {
            const bool active = index == shown;
            const float width = active ? px(22.f) : dot;
            const ImRect indicator(ImVec2(x, y), ImVec2(x + width, y + dot));
            ImGui::PushID(1000 + index);
            const ImGuiID id = window->GetID("indicator");
            ImGui::SetCursorScreenPos(indicator.Min);
            ImGui::ItemSize(ImVec2(0.f, 0.f));
            ImGui::ItemAdd(indicator, id);
            bool hovered = false;
            bool indicator_held = false;
            if (ImGui::ButtonBehavior(indicator, id, &hovered, &indicator_held) && !active)
                go(index > shown ? 1 : -1, index);
            ImGui::PopID();
            draw_list->AddRectFilled(
                indicator.Min, indicator.Max,
                faded(IM_COL32_WHITE, active ? 0.95f : (hovered ? 0.62f : 0.34f)), dot * 0.5f);
            x += width + gap;
        }
    }

    if (options.show_progress && count > 1)
    {
        const float y = rect.Max.y - px(2.f);
        draw_list->AddRectFilled(ImVec2(rect.Min.x, y), rect.Max, faded(IM_COL32_WHITE, 0.12f));
        draw_list->AddRectFilled(
            ImVec2(rect.Min.x, y),
            ImVec2(rect.Min.x + rect.GetWidth() * public_status.until_next, rect.Max.y),
            faded(IM_COL32_WHITE, 0.55f));
    }
    ImGui::PopID();
}
} // namespace solace::slides
