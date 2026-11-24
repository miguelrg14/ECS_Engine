// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "renderer/Mesh.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace engine
{

    /// Loads assets from disk the first time they are asked for and returns the
    /// same object afterwards, so ten entities with one model share one mesh.
    /// Paths are relative to the assets folder next to the executable.
    class Resource_Manager
    {
        std::string root;

        std::unordered_map< std::string, std::shared_ptr< Shader  > > shaders;
        std::unordered_map< std::string, std::shared_ptr< Texture > > textures;
        std::unordered_map< std::string, std::shared_ptr< Mesh    > > meshes;

    public:

        explicit Resource_Manager (std::string assets_folder = "assets/");

        /// "lit" loads shaders/lit.vert and shaders/lit.frag.
        std::shared_ptr< Shader  > shader  (const std::string & name);

        /// "textures/wood.png"
        std::shared_ptr< Texture > texture (const std::string & path);

        /// "models/Umbrella.obj"
        std::shared_ptr< Mesh    > mesh    (const std::string & path);

        /// Keeps a mesh built in code (a cube, a plane) under a name of its own.
        std::shared_ptr< Mesh    > add_mesh (const std::string & name, std::shared_ptr< Mesh > mesh);

        /// Releases every cached asset. Objects still held elsewhere stay alive.
        void clear ();

        const std::string & get_root () const { return root; }
    };

}
