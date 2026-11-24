// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <glm/vec4.hpp>

namespace engine
{

    /// 2D RGBA texture with mipmaps and repeat wrapping.
    class Texture
    {
        std::uint32_t texture_id = 0;
        int           width      = 0;
        int           height     = 0;

    public:

        /// Loads PNG, JPG, TGA or BMP. Throws std::runtime_error if the file cannot be read.
        static std::shared_ptr< Texture > from_file (const std::string & path);

        /// 1x1 texture of a single colour (components 0..1).
        static std::shared_ptr< Texture > from_color (const glm::vec4 & color);

        /// pixels: width * height RGBA8 values, first row at the bottom (OpenGL convention).
        Texture (int width, int height, const std::uint8_t * pixels);
       ~Texture ();

        Texture (const Texture & ) = delete;
        Texture & operator = (const Texture & ) = delete;

    public:

        void bind (unsigned unit = 0) const;

        int           get_width  () const { return width;      }
        int           get_height () const { return height;     }
        std::uint32_t get_id     () const { return texture_id; }
    };

}
