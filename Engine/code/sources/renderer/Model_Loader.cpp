// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "renderer/Model_Loader.hpp"
#include "core/Log.hpp"

#include <tiny_obj_loader.h>
#include <glm/geometric.hpp>

#include <functional>
#include <stdexcept>
#include <unordered_map>

namespace engine
{

    namespace
    {

        /// Identifies an .obj vertex by its (position, normal, uv) indices.
        struct Index_Key
        {
            int position, normal, uv;

            bool operator == (const Index_Key & ) const = default;
        };

        struct Index_Key_Hash
        {
            std::size_t operator () (const Index_Key & key) const
            {
                std::size_t hash = std::hash< int >{}(key.position);
                hash = hash * 31 + std::hash< int >{}(key.normal);
                hash = hash * 31 + std::hash< int >{}(key.uv);
                return hash;
            }
        };

        void generate_smooth_normals (std::vector< Vertex > & vertices, const std::vector< std::uint32_t > & indices)
        {
            for (Vertex & vertex : vertices) vertex.normal = glm::vec3(0.f);

            // Each triangle adds its area-weighted normal to its three vertices.

            for (std::size_t i = 0; i + 2 < indices.size (); i += 3)
            {
                Vertex & a = vertices[indices[i    ]];
                Vertex & b = vertices[indices[i + 1]];
                Vertex & c = vertices[indices[i + 2]];

                const glm::vec3 face_normal = glm::cross (b.position - a.position, c.position - a.position);

                a.normal += face_normal;
                b.normal += face_normal;
                c.normal += face_normal;
            }

            for (Vertex & vertex : vertices)
            {
                const float length = glm::length (vertex.normal);
                vertex.normal = length > 0.f ? vertex.normal / length : glm::vec3(0.f, 1.f, 0.f);
            }
        }

    }

    std::shared_ptr< Mesh > load_obj (const std::string & path)
    {
        tinyobj::ObjReaderConfig config;
        config.triangulate  = true;
        config.vertex_color = false;
        config.mtl_search_path = "";        // materials come from the engine, not from the .mtl

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile (path, config))
        {
            throw std::runtime_error ("Model: cannot load '" + path + "': " + reader.Error ());
        }

        const tinyobj::attrib_t & attributes = reader.GetAttrib ();

        std::vector< Vertex        > vertices;
        std::vector< std::uint32_t > indices;
        std::unordered_map< Index_Key, std::uint32_t, Index_Key_Hash > unique_vertices;

        bool has_all_normals = true;

        for (const tinyobj::shape_t & shape : reader.GetShapes ())
        {
            for (const tinyobj::index_t & index : shape.mesh.indices)
            {
                const Index_Key key { index.vertex_index, index.normal_index, index.texcoord_index };

                auto found = unique_vertices.find (key);

                if (found != unique_vertices.end ())
                {
                    indices.push_back (found->second);
                    continue;
                }

                Vertex vertex {};

                const auto p = std::size_t(index.vertex_index) * 3;
                vertex.position = { attributes.vertices[p], attributes.vertices[p + 1], attributes.vertices[p + 2] };

                if (index.normal_index >= 0)
                {
                    const auto n = std::size_t(index.normal_index) * 3;
                    vertex.normal = { attributes.normals[n], attributes.normals[n + 1], attributes.normals[n + 2] };
                }
                else
                {
                    has_all_normals = false;
                }

                if (index.texcoord_index >= 0)
                {
                    const auto t = std::size_t(index.texcoord_index) * 2;
                    vertex.uv = { attributes.texcoords[t], attributes.texcoords[t + 1] };
                }

                const auto new_index = std::uint32_t(vertices.size ());

                vertices.push_back (vertex);
                indices .push_back (new_index);
                unique_vertices.emplace (key, new_index);
            }
        }

        if (indices.empty ())
        {
            throw std::runtime_error ("Model: '" + path + "' has no triangles");
        }

        if (!has_all_normals) generate_smooth_normals (vertices, indices);

        log_info ("Model: %s (%zu vertices, %zu triangles)", path.c_str (), vertices.size (), indices.size () / 3);

        return std::make_shared< Mesh >(vertices, indices);
    }

}
