// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <glm/vec3.hpp>

namespace engine
{

    /// Box used by Collision_System, in the entity's local space (it follows the
    /// Transform's position, scale and rotation; a rotated box is wrapped in the
    /// smallest axis-aligned box that contains it).
    ///
    /// - is_trigger: reports Trigger_Enter / Trigger_Exit and never blocks.
    /// - is_static:  never pushed by collisions (walls, platforms). A script may
    ///               still move it.
    /// - otherwise:  solid and dynamic: pushed out of what it overlaps. Dynamic
    ///               colliders should be on root entities (no parent).
    struct Box_Collider
    {
        glm::vec3 half_extents { 0.5f };
        glm::vec3 offset       { 0.f };
        bool      is_trigger   = false;
        bool      is_static    = true;

        /// Total push applied by Collision_System in the last frame (written by it).
        /// last_push.y > 0 means something below held the entity up.
        glm::vec3 last_push    { 0.f };
    };

}
