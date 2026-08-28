#include "ui/foundation/rounded_panel.h"

#include <algorithm>

namespace solace::rounded_panel
{
bool init(ID3D11Device*, ID3D11DeviceContext*)
{
    return true;
}

void shutdown() {}

void draw(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max, ImU32 fill, float radius)
{
    if (!draw_list || max.x <= min.x || max.y <= min.y)
        return;
    draw_list->AddRectFilled(min, max, fill, std::max(radius, 0.f));
}
} // namespace solace::rounded_panel
