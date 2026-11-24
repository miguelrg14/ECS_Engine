// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Test_Framework.hpp"

#include <components/Transform.hpp>
#include <components/Velocity.hpp>
#include <ecs/Registry.hpp>
#include <systems/Movement_System.hpp>
#include <systems/Transform_System.hpp>

#include <cmath>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

using namespace engine;

namespace
{

    bool close_to (const glm::vec3 & a, const glm::vec3 & b, float tolerance = 1e-4f)
    {
        return glm::length (a - b) < tolerance;
    }

    Entity make (Registry & registry, const glm::vec3 & position, Entity parent = null_entity)
    {
        const Entity entity = registry.create ();
        Transform & transform = registry.add< Transform >(entity);
        transform.position = position;
        transform.parent   = parent;
        return entity;
    }

}

// ---------------------------------------------------------------------------
// Transform_System

TEST (root_world_matrix_is_its_local_matrix)
{
    Registry registry;
    Transform_System system (registry);

    const Entity entity = make (registry, { 1.f, 2.f, 3.f });
    registry.get< Transform >(entity).scale = glm::vec3(2.f);

    system.update ();

    const Transform & transform = registry.get< Transform >(entity);

    CHECK (close_to (transform.get_world_position (), { 1.f, 2.f, 3.f }));
    CHECK (close_to (glm::vec3(transform.world[0]), { 2.f, 0.f, 0.f }));
}

TEST (child_inherits_parent_rotation_and_position)
{
    Registry registry;
    Transform_System system (registry);

    const Entity parent = make (registry, { 1.f, 0.f, 0.f });
    const Entity child  = make (registry, { 1.f, 0.f, 0.f }, parent);

    // 90 degrees around Y turns +X into -Z.
    registry.get< Transform >(parent).rotation = glm::angleAxis (glm::radians (90.f), glm::vec3(0.f, 1.f, 0.f));

    system.update ();

    CHECK (close_to (registry.get< Transform >(child).get_world_position (), { 1.f, 0.f, -1.f }));
}

TEST (hierarchy_does_not_depend_on_creation_order)
{
    // The grandchild is created first, so it sits first in the pool.
    Registry registry;
    Transform_System system (registry);

    const Entity grandchild = make (registry, { 0.f, 0.f, 1.f });
    const Entity child      = make (registry, { 0.f, 1.f, 0.f });
    const Entity root       = make (registry, { 1.f, 0.f, 0.f });

    registry.get< Transform >(grandchild).parent = child;
    registry.get< Transform >(child     ).parent = root;

    system.update ();

    CHECK (close_to (registry.get< Transform >(grandchild).get_world_position (), { 1.f, 1.f, 1.f }));
}

TEST (parent_scale_applies_to_child_offset)
{
    Registry registry;
    Transform_System system (registry);

    const Entity parent = make (registry, { 0.f, 0.f, 0.f });
    const Entity child  = make (registry, { 1.f, 0.f, 0.f }, parent);

    registry.get< Transform >(parent).scale = glm::vec3(3.f);

    system.update ();

    CHECK (close_to (registry.get< Transform >(child).get_world_position (), { 3.f, 0.f, 0.f }));
}

TEST (child_of_destroyed_parent_becomes_root)
{
    Registry registry;
    Transform_System system (registry);

    const Entity parent = make (registry, { 5.f, 0.f, 0.f });
    const Entity child  = make (registry, { 1.f, 0.f, 0.f }, parent);

    registry.destroy (parent);
    make (registry, { 100.f, 0.f, 0.f });          // reuses the parent's slot with a new version

    system.update ();

    CHECK (close_to (registry.get< Transform >(child).get_world_position (), { 1.f, 0.f, 0.f }));
}

TEST (parent_cycle_terminates)
{
    Registry registry;
    Transform_System system (registry);

    const Entity a = make (registry, { 1.f, 0.f, 0.f });
    const Entity b = make (registry, { 0.f, 1.f, 0.f }, a);
    registry.get< Transform >(a).parent = b;

    const Entity self = make (registry, { 0.f, 0.f, 1.f });
    registry.get< Transform >(self).parent = self;

    system.update ();                               // must return

    CHECK (close_to (registry.get< Transform >(self).get_world_position (), { 0.f, 0.f, 1.f }));
}

TEST (world_follows_parent_every_update)
{
    Registry registry;
    Transform_System system (registry);

    const Entity parent = make (registry, { 0.f, 0.f, 0.f });
    const Entity child  = make (registry, { 0.f, 1.f, 0.f }, parent);

    system.update ();
    registry.get< Transform >(parent).position.x = 4.f;
    system.update ();

    CHECK (close_to (registry.get< Transform >(child).get_world_position (), { 4.f, 1.f, 0.f }));
}

// ---------------------------------------------------------------------------
// Movement_System

TEST (linear_velocity_moves_the_transform)
{
    Registry registry;
    Movement_System system (registry);

    const Entity entity = make (registry, { 0.f, 0.f, 0.f });
    registry.add< Velocity >(entity, glm::vec3(2.f, 0.f, -1.f));

    system.execute (0.5f);

    CHECK (close_to (registry.get< Transform >(entity).position, { 1.f, 0.f, -0.5f }));
}

TEST (angular_velocity_rotates_around_local_axes)
{
    Registry registry;
    Movement_System system (registry);

    const Entity entity = make (registry, { 0.f, 0.f, 0.f });
    registry.add< Velocity >(entity, glm::vec3(0.f), glm::vec3(0.f, glm::radians (90.f), 0.f));

    system.execute (1.f);

    // After a quarter turn around Y, forward (-Z) points to -X.
    CHECK (close_to (registry.get< Transform >(entity).get_forward (), { -1.f, 0.f, 0.f }));
}

TEST (entities_without_velocity_do_not_move)
{
    Registry registry;
    Movement_System system (registry);

    const Entity entity = make (registry, { 3.f, 0.f, 0.f });

    system.execute (1.f);

    CHECK (close_to (registry.get< Transform >(entity).position, { 3.f, 0.f, 0.f }));
}
