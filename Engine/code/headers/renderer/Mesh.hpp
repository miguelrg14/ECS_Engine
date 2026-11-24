// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace engine
{

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    struct Bounds
    {
        glm::vec3 min { 0.f };
        glm::vec3 max { 0.f };

        glm::vec3 get_size   () const { return max - min; }
        glm::vec3 get_center () const { return (min + max) * 0.5f; }
    };

    /// Indexed triangle mesh on the GPU: one VAO with an interleaved vertex buffer
    /// (position, normal, uv at attribute locations 0, 1, 2) and an index buffer.
    class Mesh
    {
        std::uint32_t vao_id      = 0;
        std::uint32_t vbo_id      = 0;
        std::uint32_t ebo_id      = 0;
        std::int32_t  index_count = 0;
        Bounds        bounds;

    public:

        Mesh (const std::vector< Vertex > & vertices, const std::vector< std::uint32_t > & indices);
       ~Mesh ();

        Mesh (const Mesh & ) = delete;
        Mesh & operator = (const Mesh & ) = delete;

    public:

        void draw () const;

        const Bounds & get_bounds         () const { return bounds;      }
        std::int32_t   get_index_count    () const { return index_count; }

    public:

        /// Unit cube centred at the origin, one texture per face.
        static std::shared_ptr< Mesh > create_cube  ();

        /// Flat square on the XZ plane, facing +Y. uv_tiling repeats the texture.
        static std::shared_ptr< Mesh > create_plane (float size, float uv_tiling = 1.f);
    };

}
