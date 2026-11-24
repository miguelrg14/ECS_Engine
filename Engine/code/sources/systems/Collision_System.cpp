// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "systems/Collision_System.hpp"

#include "components/Box_Collider.hpp"
#include "components/Transform.hpp"
#include "events/Collision_Events.hpp"
#include "systems/Transform_System.hpp"

#include <glm/common.hpp>
#include <glm/mat3x3.hpp>

#include <algorithm>
#include <numeric>

namespace engine
{

    namespace
    {

        Collision_System::Pair make_ordered_pair (Entity a, Entity b)
        {
            return a < b ? Collision_System::Pair{ a, b } : Collision_System::Pair{ b, a };
        }

        /// World-space box that contains the collider, whatever the entity's rotation.
        Aabb world_box (const Transform & transform, const Box_Collider & collider)
        {
            const glm::vec3 center = glm::vec3(transform.world * glm::vec4(collider.offset, 1.f));

            // Each local axis, scaled and rotated, contributes |axis| * half extent to the world half size.
            const glm::mat3 axes (transform.world);

            const glm::vec3 half = glm::abs (axes[0]) * collider.half_extents.x
                                 + glm::abs (axes[1]) * collider.half_extents.y
                                 + glm::abs (axes[2]) * collider.half_extents.z;

            return { center - half, center + half };
        }

    }

    Collision_System::Collision_System (Registry & given_registry, Event_Bus & given_events, Transform_System & given_transforms)
    :
        System    ("Collision_System", Priority::collision, given_registry),
        events    (given_events    ),
        transforms(given_transforms)
    {
    }

    void Collision_System::execute (float )
    {
        gather_bodies ();

        std::vector< Aabb > boxes;
        boxes.reserve (bodies.size ());
        for (const Body & body : bodies) boxes.push_back (body.box);

        pair_tests = 0;

        const auto overlaps = find_overlaps (boxes, &pair_tests);

        trigger_pairs.clear ();
        contact_pairs.clear ();

        bool moved = false;

        for (const auto & [first, second] : overlaps)
        {
            Body & a = bodies[first ];
            Body & b = bodies[second];

            // Boxes move while pairs are resolved: check again with the current ones.
            if (!a.box.overlaps (b.box)) continue;

            const bool a_trigger = a.collider->is_trigger;
            const bool b_trigger = b.collider->is_trigger;

            if (a_trigger && b_trigger) continue;                       // triggers ignore each other

            if (a_trigger || b_trigger)
            {
                const Entity trigger = a_trigger ? a.entity : b.entity;
                const Entity other   = a_trigger ? b.entity : a.entity;

                // Ordered (trigger, other) so enter/exit always name the trigger first.
                trigger_pairs.emplace_back (trigger, other);
                continue;
            }

            if (a.collider->is_static && b.collider->is_static) continue;   // walls touching walls

            contact_pairs.push_back (make_ordered_pair (a.entity, b.entity));
            resolve (a, b, moved);
        }

        // Children of pushed entities must follow in this same frame.
        if (moved) transforms.update ();

        publish_changes ();
    }

    void Collision_System::gather_bodies ()
    {
        bodies.clear ();

        registry.view< Transform, Box_Collider >().each ([this] (Entity entity, Transform & transform, Box_Collider & collider)
        {
            collider.last_push = glm::vec3(0.f);
            bodies.push_back ({ entity, world_box (transform, collider), &collider });
        });

        // The view walks pools backwards; sort for a stable, creation-ordered result.
        std::sort (bodies.begin (), bodies.end (), [] (const Body & a, const Body & b) { return a.entity < b.entity; });
    }

    std::vector< std::pair< std::size_t, std::size_t > > Collision_System::find_overlaps (const std::vector< Aabb > & boxes, std::size_t * tests)
    {
        // Sweep and prune on X: after sorting by min.x, box i can only overlap the boxes
        // that start before it ends; the inner loop stops at the first one that does not.

        std::vector< std::size_t > order (boxes.size ());
        std::iota (order.begin (), order.end (), std::size_t(0));

        std::stable_sort (order.begin (), order.end (), [&boxes] (std::size_t a, std::size_t b)
        {
            return boxes[a].min.x < boxes[b].min.x;
        });

        std::vector< std::pair< std::size_t, std::size_t > > result;

        for (std::size_t i = 0; i < order.size (); ++i)
        {
            const Aabb & box = boxes[order[i]];

            for (std::size_t j = i + 1; j < order.size (); ++j)
            {
                const Aabb & other = boxes[order[j]];

                if (other.min.x >= box.max.x) break;          // sorted: no later box can overlap in x

                if (tests) ++*tests;

                if (box.overlaps (other))
                {
                    result.emplace_back (std::min (order[i], order[j]), std::max (order[i], order[j]));
                }
            }
        }

        // Deterministic order regardless of positions.
        std::sort (result.begin (), result.end ());

        return result;
    }

    void Collision_System::resolve (Body & a, Body & b, bool & moved)
    {
        // Penetration along each axis; the smallest one is the cheapest way out.
        const glm::vec3 depth = glm::min (a.box.max, b.box.max) - glm::max (a.box.min, b.box.min);

        int axis = 0;
        if (depth.y < depth[axis]) axis = 1;
        if (depth.z < depth[axis]) axis = 2;

        const float a_center = (a.box.min[axis] + a.box.max[axis]) * 0.5f;
        const float b_center = (b.box.min[axis] + b.box.max[axis]) * 0.5f;

        // Normal from b to a.
        glm::vec3 normal (0.f);
        normal[axis] = a_center >= b_center ? 1.f : -1.f;

        const bool a_moves = !a.collider->is_static;
        const bool b_moves = !b.collider->is_static;

        // Both dynamic: half each. One static: the other takes it all.
        const float a_share = a_moves ? (b_moves ? 0.5f : 1.f) : 0.f;
        const float b_share = b_moves ? (a_moves ? 0.5f : 1.f) : 0.f;

        auto push = [&] (Body & body, const glm::vec3 & delta)
        {
            if (delta == glm::vec3(0.f)) return;

            Transform & transform = registry.get< Transform >(body.entity);

            transform.position   += delta;          // dynamic colliders are roots: local == world
            transform.world[3]   += glm::vec4(delta, 0.f);
            body.box.min         += delta;
            body.box.max         += delta;
            body.collider->last_push += delta;

            moved = true;
        };

        push (a,  normal * depth[axis] * a_share);
        push (b, -normal * depth[axis] * b_share);

        // Remember the normal for the enter event (published later, only for new pairs).
        // Stored for the (lower, higher) order of the pair: from the higher entity towards the lower.
        last_normal_of_pair[make_ordered_pair (a.entity, b.entity)] = a.entity < b.entity ? normal : -normal;
    }

    void Collision_System::publish_changes ()
    {
        std::sort (trigger_pairs.begin (), trigger_pairs.end ());
        std::sort (contact_pairs.begin (), contact_pairs.end ());

        trigger_pairs.erase (std::unique (trigger_pairs.begin (), trigger_pairs.end ()), trigger_pairs.end ());
        contact_pairs.erase (std::unique (contact_pairs.begin (), contact_pairs.end ()), contact_pairs.end ());

        // New pairs: in this frame, not in the previous one. Gone pairs: the opposite.

        std::vector< Pair > changed;

        std::set_difference (trigger_pairs.begin (), trigger_pairs.end (),
                             previous_trigger_pairs.begin (), previous_trigger_pairs.end (), std::back_inserter (changed));

        for (const Pair & pair : changed) events.publish (Trigger_Enter{ pair.first, pair.second });

        changed.clear ();

        std::set_difference (previous_trigger_pairs.begin (), previous_trigger_pairs.end (),
                             trigger_pairs.begin (), trigger_pairs.end (), std::back_inserter (changed));

        for (const Pair & pair : changed) events.publish (Trigger_Exit{ pair.first, pair.second });

        changed.clear ();

        std::set_difference (contact_pairs.begin (), contact_pairs.end (),
                             previous_contact_pairs.begin (), previous_contact_pairs.end (), std::back_inserter (changed));

        for (const Pair & pair : changed) events.publish (Collision_Enter{ pair.first, pair.second, last_normal_of_pair[pair] });

        previous_trigger_pairs.swap (trigger_pairs);
        previous_contact_pairs.swap (contact_pairs);
        last_normal_of_pair.clear ();
    }

}
