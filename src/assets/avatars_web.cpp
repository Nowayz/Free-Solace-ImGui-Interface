#include "assets/avatars.h"

#include "assets/asset_io.h"
#include "graphics/gl_texture.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace solace::avatars
{
namespace
{
struct named_texture
{
    std::string name;
    ImTextureID id = ImTextureID_Invalid;
};

ImTextureID g_me = ImTextureID_Invalid;
std::vector<ImTextureID> g_others;
std::vector<ImTextureID> g_logos;
std::vector<named_texture> g_brands;

ImTextureID load_texture(const std::filesystem::path& path)
{
    gl_texture::image decoded;
    if (!gl_texture::decode(path, decoded))
        return ImTextureID_Invalid;
    return gl_texture::upload(gl_texture::resize_cover(decoded, 128, 128));
}

ImTextureID pick(const std::vector<ImTextureID>& textures, int index)
{
    if (textures.empty())
        return ImTextureID_Invalid;
    const int count = static_cast<int>(textures.size());
    return textures[static_cast<std::size_t>(((index % count) + count) % count)];
}
} // namespace

void load(const std::filesystem::path& people_directory,
          const std::filesystem::path& logo_directory, const std::filesystem::path& brand_directory,
          ID3D11Device*, ID3D11DeviceContext*)
{
    if (g_me != ImTextureID_Invalid || !g_others.empty() || !g_logos.empty() || !g_brands.empty())
        return;

    for (const std::filesystem::path& path : asset_io::image_files(people_directory))
    {
        const ImTextureID texture = load_texture(path);
        if (texture == ImTextureID_Invalid)
            continue;

        std::string name = asset_io::stem_utf8(path);
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
        if (name == "me" || name.rfind("me_", 0) == 0)
            g_me = texture;
        else
            g_others.push_back(texture);
    }

    for (const std::filesystem::path& path : asset_io::image_files(logo_directory))
    {
        const ImTextureID texture = load_texture(path);
        if (texture != ImTextureID_Invalid)
            g_logos.push_back(texture);
    }

    for (const std::filesystem::path& path : asset_io::image_files(brand_directory))
    {
        if (path.extension() != ".png")
            continue;
        const ImTextureID texture = load_texture(path);
        if (texture != ImTextureID_Invalid)
            g_brands.push_back(named_texture{asset_io::stem_utf8(path), texture});
    }
}

void shutdown()
{
    gl_texture::destroy(g_me);
    for (const ImTextureID texture : g_others)
        gl_texture::destroy(texture);
    for (const ImTextureID texture : g_logos)
        gl_texture::destroy(texture);
    for (const named_texture& item : g_brands)
        gl_texture::destroy(item.id);
    g_me = ImTextureID_Invalid;
    g_others.clear();
    g_logos.clear();
    g_brands.clear();
}

ImTextureID me()
{
    return g_me;
}

ImTextureID other(int index)
{
    return pick(g_others, index);
}

ImTextureID logo(int index)
{
    return pick(g_logos, index);
}

ImTextureID brand(const char* name)
{
    if (name)
        for (const named_texture& item : g_brands)
            if (item.name == name)
                return item.id;
    return ImTextureID_Invalid;
}

bool draw(ImDrawList* draw_list, ImTextureID texture, const ImVec2& top_left, float size,
          float alpha, float rounding)
{
    if (!draw_list || texture == ImTextureID_Invalid || alpha <= 0.004f)
        return false;

    const float radius = rounding < 0.f ? size * 0.5f : rounding;
    draw_list->AddImageRounded(
        ImTextureRef(texture), top_left, ImVec2(top_left.x + size, top_left.y + size),
        ImVec2(0.f, 0.f), ImVec2(1.f, 1.f),
        IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f + 0.5f)), radius);
    return true;
}
} // namespace solace::avatars
