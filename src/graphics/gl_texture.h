#pragma once

#include "imgui.h"

#include <filesystem>
#include <vector>

namespace solace::gl_texture
{
struct image
{
    int width = 0;
    int height = 0;
    std::vector<unsigned char> rgba;
};

[[nodiscard]] bool decode(const std::filesystem::path& path, image& output);
[[nodiscard]] image resize_cover(const image& source, int width, int height, float saturation = 1.f,
                                 float radius_ratio = 0.f);
[[nodiscard]] ImTextureID upload(const image& source);
void destroy(ImTextureID texture);
} // namespace solace::gl_texture
