// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <glm/vec3.hpp>

namespace engine
{

    /// Moves and rotates a Transform every frame (Movement_System).
    struct Velocity
    {
        glm::vec3 linear  { 0.f };      // units per second, in the parent's space
        glm::vec3 angular { 0.f };      // radians per second around the entity's own X, Y, Z axes
    };

}
