// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <glm/vec3.hpp>

namespace engine
{

    /// Point light at the entity's world position. The renderer supports one
    /// light for now: Render_System uses the first one it finds.
    struct Light_Component
    {
        glm::vec3 color     { 1.f };
        float     intensity = 1.f;
    };

}
