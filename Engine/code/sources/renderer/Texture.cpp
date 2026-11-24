// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "renderer/Texture.hpp"

#include <glad/glad.h>
#include <stb_image.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace engine
{

    std::shared_ptr< Texture > Texture::from_file (const std::string & path)
    {
        // Image files store the top row first; OpenGL expects the bottom row first.
        stbi_set_flip_vertically_on_load (1);

        int image_width = 0, image_height = 0, channels = 0;

        // Always 4 channels (RGBA), whatever the file holds.
        stbi_uc * pixels = stbi_load (path.c_str (), &image_width, &image_height, &channels, 4);

        if (!pixels)
        {
            throw std::runtime_error ("Texture: cannot load '" + path + "': " + stbi_failure_reason ());
        }

        auto texture = std::make_shared< Texture >(image_width, image_height, pixels);

        stbi_image_free (pixels);

        return texture;
    }

    std::shared_ptr< Texture > Texture::from_color (const glm::vec4 & color)
    {
        auto to_byte = [] (float value)
        {
            return std::uint8_t(std::lround (std::clamp (value, 0.f, 1.f) * 255.f));
        };

        const std::array< std::uint8_t, 4 > pixel { to_byte (color.r), to_byte (color.g), to_byte (color.b), to_byte (color.a) };

        return std::make_shared< Texture >(1, 1, pixel.data ());
    }

    Texture::Texture (int given_width, int given_height, const std::uint8_t * pixels)
    :
        width (given_width ),
        height(given_height)
    {
        glGenTextures (1, &texture_id);
        glBindTexture (GL_TEXTURE_2D, texture_id);

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Rows of RGBA8 are always 4-byte aligned, but be explicit.
        glPixelStorei (GL_UNPACK_ALIGNMENT, 4);

        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap (GL_TEXTURE_2D);

        glBindTexture (GL_TEXTURE_2D, 0);
    }

    Texture::~Texture ()
    {
        glDeleteTextures (1, &texture_id);
    }

    void Texture::bind (unsigned unit) const
    {
        glActiveTexture (GL_TEXTURE0 + unit);
        glBindTexture   (GL_TEXTURE_2D, texture_id);
    }

}
