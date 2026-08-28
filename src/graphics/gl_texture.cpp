#include "graphics/gl_texture.h"

#include "assets/asset_io.h"

#include <GLES3/gl3.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_NO_STDIO
#include "../../thirdparty/stb/stb_image.h"

namespace solace::gl_texture
{
bool decode(const std::filesystem::path& path, image& output)
{
    const std::vector<unsigned char> bytes = asset_io::read_binary(path);
    if (bytes.empty())
        return false;

    int components = 0;
    stbi_uc* pixels = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()),
                                            &output.width, &output.height, &components, 4);
    if (!pixels || output.width <= 0 || output.height <= 0)
    {
        if (pixels)
            stbi_image_free(pixels);
        output = {};
        return false;
    }

    const std::size_t length = static_cast<std::size_t>(output.width) * output.height * 4;
    output.rgba.assign(pixels, pixels + length);
    stbi_image_free(pixels);
    return true;
}

image resize_cover(const image& source, int width, int height, float saturation, float radius_ratio)
{
    image output;
    if (source.width <= 0 || source.height <= 0 || source.rgba.empty() || width <= 0 || height <= 0)
        return output;

    output.width = width;
    output.height = height;
    output.rgba.resize(static_cast<std::size_t>(width) * height * 4);

    const float source_aspect = static_cast<float>(source.width) / source.height;
    const float target_aspect = static_cast<float>(width) / height;
    int crop_width = source.width;
    int crop_height = source.height;
    if (source_aspect > target_aspect)
        crop_width = std::max(1, static_cast<int>(source.height * target_aspect + 0.5f));
    else
        crop_height = std::max(1, static_cast<int>(source.width / target_aspect + 0.5f));

    const int origin_x = (source.width - crop_width) / 2;
    const int origin_y = (source.height - crop_height) / 2;
    const float radius = std::max(0.f, radius_ratio) * width;

    for (int y = 0; y < height; ++y)
    {
        const int y0 =
            origin_y + static_cast<int>(static_cast<std::int64_t>(y) * crop_height / height);
        const int y1 =
            std::max(y0 + 1, origin_y + static_cast<int>(static_cast<std::int64_t>(y + 1) *
                                                         crop_height / height));
        for (int x = 0; x < width; ++x)
        {
            const int x0 =
                origin_x + static_cast<int>(static_cast<std::int64_t>(x) * crop_width / width);
            const int x1 =
                std::max(x0 + 1, origin_x + static_cast<int>(static_cast<std::int64_t>(x + 1) *
                                                             crop_width / width));

            unsigned int channels[4] = {0, 0, 0, 0};
            unsigned int samples = 0;
            for (int sample_y = y0; sample_y < std::min(y1, source.height); ++sample_y)
            {
                for (int sample_x = x0; sample_x < std::min(x1, source.width); ++sample_x)
                {
                    const unsigned char* pixel =
                        source.rgba.data() +
                        (static_cast<std::size_t>(sample_y) * source.width + sample_x) * 4;
                    for (int channel = 0; channel < 4; ++channel)
                        channels[channel] += pixel[channel];
                    ++samples;
                }
            }

            unsigned char* destination =
                output.rgba.data() + (static_cast<std::size_t>(y) * width + x) * 4;
            samples = std::max(samples, 1u);
            for (int channel = 0; channel < 4; ++channel)
                destination[channel] = static_cast<unsigned char>(channels[channel] / samples);

            const float luma =
                0.2126f * destination[0] + 0.7152f * destination[1] + 0.0722f * destination[2];
            for (int channel = 0; channel < 3; ++channel)
                destination[channel] = static_cast<unsigned char>(
                    std::clamp(luma + (destination[channel] - luma) * saturation, 0.f, 255.f));

            if (radius > 0.f)
            {
                const float edge_x = std::fabs(x + 0.5f - width * 0.5f) - (width * 0.5f - radius);
                const float edge_y = std::fabs(y + 0.5f - height * 0.5f) - (height * 0.5f - radius);
                const float qx = std::max(edge_x, 0.f);
                const float qy = std::max(edge_y, 0.f);
                const float distance = std::sqrt(qx * qx + qy * qy) - radius;
                const float coverage = std::clamp(0.5f - distance, 0.f, 1.f);
                destination[3] = static_cast<unsigned char>(destination[3] * coverage);
            }
        }
    }
    return output;
}

ImTextureID upload(const image& source)
{
    if (source.width <= 0 || source.height <= 0 || source.rgba.empty())
        return ImTextureID_Invalid;

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, source.width, source.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, source.rgba.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return static_cast<ImTextureID>(texture);
}

void destroy(ImTextureID texture)
{
    if (texture == ImTextureID_Invalid)
        return;
    const GLuint handle = static_cast<GLuint>(texture);
    glDeleteTextures(1, &handle);
}
} // namespace solace::gl_texture
