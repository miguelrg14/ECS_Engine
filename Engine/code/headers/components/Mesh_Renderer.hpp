// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "renderer/Material.hpp"
#include "renderer/Mesh.hpp"

#include <memory>

namespace engine
{

    /// Draws `mesh` with `material` at the entity's Transform (Render_System).
    /// Mesh and material are shared: a thousand entities can point to one of each.
    struct Mesh_Renderer
    {
        std::shared_ptr< Mesh     > mesh;
        std::shared_ptr< Material > material;
        bool                        visible = true;
    };

}
