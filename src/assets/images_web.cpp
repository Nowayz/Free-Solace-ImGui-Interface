#include "assets/images.h"

#include "assets/asset_io.h"
#include "graphics/gl_texture.h"

#include <algorithm>
#include <cmath>

namespace solace::images
{
namespace
{
std::vector<texture> g_textures;
bool g_loaded = false;
} // namespace

void load_folder(const std::filesystem::path& directory, const options& opts)
{
    if (g_loaded || directory.empty())
        return;
    g_loaded = true;

    for (const std::filesystem::path& path : asset_io::image_files(directory))
    {
        gl_texture::image decoded;
        if (!gl_texture::decode(path, decoded))
            continue;

        const float aspect =
            opts.aspect > 0.f ? opts.aspect : static_cast<float>(decoded.width) / decoded.height;
        int width = decoded.width;
        int height = decoded.height;
        if (static_cast<float>(width) / height > aspect)
            width = std::max(1, static_cast<int>(height * aspect + 0.5f));
        else
            height = std::max(1, static_cast<int>(width / aspect + 0.5f));

        if (opts.max_edge > 0 && width > opts.max_edge)
        {
            width = opts.max_edge;
            height = std::max(1, static_cast<int>(width / aspect + 0.5f));
        }

        gl_texture::image prepared =
            gl_texture::resize_cover(decoded, width, height, opts.saturate, opts.radius_ratio);
        const ImTextureID id = gl_texture::upload(prepared);
        if (id == ImTextureID_Invalid)
            continue;

        g_textures.push_back(
            texture{id, prepared.width, prepared.height, asset_io::stem_utf8(path)});
    }
}

void update(ID3D11Device*, ID3D11DeviceContext*) {}

const std::vector<texture>& ready()
{
    return g_textures;
}

void shutdown()
{
    for (const texture& item : g_textures)
        gl_texture::destroy(item.id);
    g_textures.clear();
    g_loaded = false;
}
} // namespace solace::images
