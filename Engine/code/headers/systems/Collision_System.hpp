// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "System.hpp"
#include "events/Event_Bus.hpp"

#include <cstddef>
#include <map>
#include <utility>
#include <vector>

#include <glm/vec3.hpp>

namespace engine
{

    class Transform_System;

    struct Box_Collider;

    struct Aabb
    {
        glm::vec3 min;
        glm::vec3 max;

        bool overlaps (const Aabb & other) const
        {
            return min.x < other.max.x && max.x > other.min.x
                && min.y < other.max.y && max.y > other.min.y
                && min.z < other.max.z && max.z > other.min.z;
        }
    };

    /// Finds overlapping Box_Colliders, pushes dynamic solids out of what they hit
    /// and publishes Trigger_Enter / Trigger_Exit / Collision_Enter.
    ///
    /// Runs after Transform_System (it needs world matrices). Candidate pairs come
    /// from sweep and prune: boxes sorted by their minimum x, so each box is only
    /// tested against the ones whose x range overlaps its own. Solid pairs are
    /// separated along the axis of least penetration. If anything was pushed, the
    /// world matrices are recomputed so children follow their parents this frame.
    class Collision_System final : public System
    {
    public:

        using Pair = std::pair< Entity, Entity >;        // always (lower, higher)

    private:

        struct Body
        {
            Entity         entity;
            Aabb           box;
            Box_Collider * collider;
        };

        Event_Bus        & events;
        Transform_System & transforms;

        std::vector< Body > bodies;
        std::vector< Pair > trigger_pairs,  previous_trigger_pairs;
        std::vector< Pair > contact_pairs,  previous_contact_pairs;
        std::map< Pair, glm::vec3 > last_normal_of_pair;     // normal (from second to first) of this frame's contacts

        std::size_t pair_tests = 0;

    public:

        Collision_System (Registry & registry, Event_Bus & events, Transform_System & transforms);

        void execute (float delta_time) override;

    public:

        /// Box-against-box tests done in the last frame. Without the broad phase it would be n(n-1)/2.
        std::size_t get_pair_tests     () const { return pair_tests; }
        std::size_t get_collider_count () const { return bodies.size (); }

        /// Every overlapping pair among `boxes`, by sweep and prune. Pairs hold indices into `boxes`,
        /// lower index first. Exposed for testing.
        static std::vector< std::pair< std::size_t, std::size_t > > find_overlaps (const std::vector< Aabb > & boxes, std::size_t * tests = nullptr);

    private:

        void gather_bodies   ();
        void resolve         (Body & a, Body & b, bool & moved);
        void publish_changes ();
    };

}
