// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "ecs/Entity.hpp"

#include <glm/vec3.hpp>

namespace engine
{

    // Published by Collision_System and delivered by Event_System in the same frame.
    // An entity destroyed while overlapping produces an exit event with its old
    // handle: check registry.valid() before using it.

    /// `other` started overlapping the trigger `trigger`.
    struct Trigger_Enter
    {
        Entity trigger;
        Entity other;
    };

    /// `other` stopped overlapping the trigger `trigger` (or one of them was destroyed).
    struct Trigger_Exit
    {
        Entity trigger;
        Entity other;
    };

    /// Two solid colliders started touching. `normal` points from `b` towards `a`.
    struct Collision_Enter
    {
        Entity    a;
        Entity    b;
        glm::vec3 normal;
    };

}
