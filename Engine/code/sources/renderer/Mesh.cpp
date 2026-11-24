// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "renderer/Mesh.hpp"

#include <glad/glad.h>
#include <glm/common.hpp>
#include <cstddef>

namespace engine
{

    Mesh::Mesh (const std::vector< Vertex > & vertices, const std::vector< std::uint32_t > & indices)
    :
        index_count(std::int32_t(indices.size ()))
    {
        if (!vertices.empty ())
        {
            bounds.min = bounds.max = vertices.front ().position;

            for (const Vertex & vertex : vertices)
            {
                bounds.min = glm::min (bounds.min, vertex.position);
                bounds.max = glm::max (bounds.max, vertex.position);
            }
        }

        glGenVertexArrays (1, &vao_id);
        glGenBuffers      (1, &vbo_id);
        glGenBuffers      (1, &ebo_id);

        glBindVertexArray (vao_id);

        glBindBuffer (GL_ARRAY_BUFFER, vbo_id);
        glBufferData (GL_ARRAY_BUFFER, GLsizeiptr(vertices.size () * sizeof(Vertex)), vertices.data (), GL_STATIC_DRAW);

        // The index buffer binding is stored in the VAO.
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ebo_id);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indices.size () * sizeof(std::uint32_t)), indices.data (), GL_STATIC_DRAW);

        constexpr GLsizei stride = sizeof(Vertex);

        glEnableVertexAttribArray (0);
        glVertexAttribPointer     (0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast< const void * >(offsetof(Vertex, position)));

        glEnableVertexAttribArray (1);
        glVertexAttribPointer     (1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast< const void * >(offsetof(Vertex, normal  )));

        glEnableVertexAttribArray (2);
        glVertexAttribPointer     (2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast< const void * >(offsetof(Vertex, uv      )));

        glBindVertexArray (0);
    }

    Mesh::~Mesh ()
    {
        glDeleteVertexArrays (1, &vao_id);
        glDeleteBuffers      (1, &vbo_id);
        glDeleteBuffers      (1, &ebo_id);
    }

    void Mesh::draw () const
    {
        glBindVertexArray (vao_id);
        glDrawElements    (GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
    }

    std::shared_ptr< Mesh > Mesh::create_cube ()
    {
        // Each face has its own 4 vertices so it gets its own normal and UVs.

        struct Face { glm::vec3 normal, u, v; };

        constexpr Face faces[] =
        {
            { {  1,  0,  0 }, {  0,  0, -1 }, { 0, 1, 0 } },
            { { -1,  0,  0 }, {  0,  0,  1 }, { 0, 1, 0 } },
            { {  0,  1,  0 }, {  1,  0,  0 }, { 0, 0, -1 } },
            { {  0, -1,  0 }, {  1,  0,  0 }, { 0, 0,  1 } },
            { {  0,  0,  1 }, {  1,  0,  0 }, { 0, 1, 0 } },
            { {  0,  0, -1 }, { -1,  0,  0 }, { 0, 1, 0 } },
        };

        std::vector< Vertex        > vertices;
        std::vector< std::uint32_t > indices;

        for (const Face & face : faces)
        {
            const auto first  = std::uint32_t(vertices.size ());
            const auto centre = face.normal * 0.5f;

            vertices.push_back ({ centre - face.u * 0.5f - face.v * 0.5f, face.normal, { 0, 0 } });
            vertices.push_back ({ centre + face.u * 0.5f - face.v * 0.5f, face.normal, { 1, 0 } });
            vertices.push_back ({ centre + face.u * 0.5f + face.v * 0.5f, face.normal, { 1, 1 } });
            vertices.push_back ({ centre - face.u * 0.5f + face.v * 0.5f, face.normal, { 0, 1 } });

            // Counter-clockwise seen from outside.
            indices.insert (indices.end (), { first, first + 1, first + 2, first, first + 2, first + 3 });
        }

        return std::make_shared< Mesh >(vertices, indices);
    }

    std::shared_ptr< Mesh > Mesh::create_plane (float size, float uv_tiling)
    {
        const float h = size * 0.5f;
        const glm::vec3 up { 0, 1, 0 };

        const std::vector< Vertex > vertices
        {
            { { -h, 0,  h }, up, { 0,         0         } },
            { {  h, 0,  h }, up, { uv_tiling, 0         } },
            { {  h, 0, -h }, up, { uv_tiling, uv_tiling } },
            { { -h, 0, -h }, up, { 0,         uv_tiling } },
        };

        const std::vector< std::uint32_t > indices { 0, 1, 2, 0, 2, 3 };

        return std::make_shared< Mesh >(vertices, indices);
    }

}
