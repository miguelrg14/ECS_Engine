// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Test_Framework.hpp"

#include <components/Box_Collider.hpp>
#include <components/Transform.hpp>
#include <ecs/Registry.hpp>
#include <events/Collision_Events.hpp>
#include <events/Event_Bus.hpp>
#include <systems/Collision_System.hpp>
#include <systems/Transform_System.hpp>

#include <cmath>
#include <random>
#include <set>
#include <utility>
#include <vector>

using namespace engine;

namespace
{

    /// Registry + the two systems, run in engine order.
    struct World
    {
        Registry         registry;
        Event_Bus        events;
        Transform_System transforms { registry };
        Collision_System collisions { registry, events, transforms };

        int enters = 0, exits = 0, contacts = 0;
        glm::vec3 last_normal { 0.f };

        World ()
        {
            events.subscribe< Trigger_Enter   >([this] (const Trigger_Enter   & ) { ++enters; });
            events.subscribe< Trigger_Exit    >([this] (const Trigger_Exit    & ) { ++exits;  });
            events.subscribe< Collision_Enter >([this] (const Collision_Enter & event) { ++contacts; last_normal = event.normal; });
        }

        Entity box (const glm::vec3 & position, const glm::vec3 & half, bool is_static, bool is_trigger = false)
        {
            const Entity entity = registry.create ();
            registry.add< Transform    >(entity).position = position;
            registry.add< Box_Collider >(entity, half, glm::vec3(0.f), is_trigger, is_static);
            return entity;
        }

        void step ()
        {
            transforms.update ();
            collisions.execute (0.016f);
            events.dispatch ();
        }
    };

    bool close_to (float a, float b) { return std::abs (a - b) < 1e-4f; }

}

TEST (sweep_and_prune_matches_brute_force)
{
    std::mt19937 random (42);
    std::uniform_real_distribution< float > position (-20.f, 20.f);
    std::uniform_real_distribution< float > size     (0.1f, 2.f);

    std::vector< Aabb > boxes;

    for (int i = 0; i < 400; ++i)
    {
        const glm::vec3 center { position (random), position (random) * 0.2f, position (random) };
        const glm::vec3 half   { size (random), size (random), size (random) };
        boxes.push_back ({ center - half, center + half });
    }

    std::set< std::pair< std::size_t, std::size_t > > expected;

    for (std::size_t i = 0; i < boxes.size (); ++i)
        for (std::size_t j = i + 1; j < boxes.size (); ++j)
            if (boxes[i].overlaps (boxes[j])) expected.emplace (i, j);

    std::size_t tests = 0;
    const auto found = Collision_System::find_overlaps (boxes, &tests);

    CHECK (std::set< std::pair< std::size_t, std::size_t > >(found.begin (), found.end ()) == expected);
    CHECK (found.size () == expected.size ());

    // The broad phase must test far fewer pairs than all of them (79,800).
    CHECK (tests < boxes.size () * (boxes.size () - 1) / 2 / 4);
}

TEST (touching_faces_do_not_overlap)
{
    const Aabb a { { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f } };
    const Aabb b { { 1.f, 0.f, 0.f }, { 2.f, 1.f, 1.f } };

    CHECK (!a.overlaps (b));
}

TEST (dynamic_box_is_pushed_out_of_static_along_least_penetration)
{
    World world;

    world.box ({ 0.f, 0.f, 0.f }, glm::vec3(1.f), true);                        // wall
    const Entity mover = world.box ({ 1.3f, 0.f, 0.f }, glm::vec3(0.5f), false); // 0.2 into it on X, 1 on Y and Z

    world.step ();

    const Transform    & transform = world.registry.get< Transform    >(mover);
    const Box_Collider & collider  = world.registry.get< Box_Collider >(mover);

    CHECK (close_to (transform.position.x, 1.5f));
    CHECK (close_to (transform.position.y, 0.f));
    CHECK (close_to (collider.last_push.x, 0.2f));
    CHECK (world.contacts == 1);
    CHECK (world.last_normal == glm::vec3(1.f, 0.f, 0.f) || world.last_normal == glm::vec3(-1.f, 0.f, 0.f));
}

TEST (standing_on_a_platform_pushes_up)
{
    World world;

    world.box ({ 0.f, 0.f, 0.f }, { 2.f, 0.5f, 2.f }, true);                     // platform, top at 0.5
    const Entity mover = world.box ({ 0.f, 0.95f, 0.f }, glm::vec3(0.5f), false);

    world.step ();

    CHECK (close_to (world.registry.get< Transform >(mover).position.y, 1.f));
    CHECK (world.registry.get< Box_Collider >(mover).last_push.y > 0.f);
}

TEST (two_dynamic_boxes_share_the_push)
{
    World world;

    const Entity a = world.box ({ 0.f,  0.f, 0.f }, glm::vec3(0.5f), false);
    const Entity b = world.box ({ 0.8f, 0.f, 0.f }, glm::vec3(0.5f), false);

    world.step ();

    CHECK (close_to (world.registry.get< Transform >(a).position.x, -0.1f));
    CHECK (close_to (world.registry.get< Transform >(b).position.x,  0.9f));
}

TEST (static_boxes_never_move)
{
    World world;

    const Entity a = world.box ({ 0.f,  0.f, 0.f }, glm::vec3(0.5f), true);
    const Entity b = world.box ({ 0.5f, 0.f, 0.f }, glm::vec3(0.5f), true);

    world.step ();

    CHECK (close_to (world.registry.get< Transform >(a).position.x, 0.f));
    CHECK (close_to (world.registry.get< Transform >(b).position.x, 0.5f));
    CHECK (world.contacts == 0);
}

TEST (trigger_reports_enter_once_and_exit_once)
{
    World world;

    world.box ({ 0.f, 0.f, 0.f }, glm::vec3(1.f), true, true);                   // trigger zone
    const Entity visitor = world.box ({ 0.5f, 0.f, 0.f }, glm::vec3(0.5f), false);

    world.step ();
    world.step ();
    world.step ();

    CHECK (world.enters == 1);
    CHECK (world.exits  == 0);
    CHECK (close_to (world.registry.get< Transform >(visitor).position.x, 0.5f));   // triggers do not push

    world.registry.get< Transform >(visitor).position.x = 5.f;
    world.step ();
    world.step ();

    CHECK (world.enters == 1);
    CHECK (world.exits  == 1);
}

TEST (destroyed_entity_produces_trigger_exit)
{
    World world;

    world.box ({ 0.f, 0.f, 0.f }, glm::vec3(1.f), true, true);
    const Entity visitor = world.box ({ 0.f, 0.f, 0.f }, glm::vec3(0.5f), false);

    world.step ();
    world.registry.destroy (visitor);
    world.step ();

    CHECK (world.enters == 1);
    CHECK (world.exits  == 1);
}

TEST (rotated_box_is_wrapped_by_its_world_aabb)
{
    World world;

    // A 2 x 0.2 bar turned 90 degrees around Y now spans Z, not X.
    const Entity bar = world.registry.create ();
    Transform & transform = world.registry.add< Transform >(bar);
    transform.rotation = glm::angleAxis (glm::radians (90.f), glm::vec3(0.f, 1.f, 0.f));
    world.registry.add< Box_Collider >(bar, glm::vec3(1.f, 0.1f, 0.1f));

    const Entity on_x = world.box ({ 0.9f, 0.f, 0.f }, glm::vec3(0.05f), false, false);
    const Entity on_z = world.box ({ 0.f, 0.f, 0.9f }, glm::vec3(0.05f), false, false);

    world.step ();

    CHECK (close_to (world.registry.get< Transform >(on_x).position.x, 0.9f));   // no longer touching
    CHECK (world.registry.get< Box_Collider >(on_z).last_push != glm::vec3(0.f)); // now touching
}
