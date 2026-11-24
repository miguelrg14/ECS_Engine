// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "systems/Transform_System.hpp"

#include "components/Transform.hpp"

namespace engine
{

    Transform_System::Transform_System (Registry & given_registry)
    :
        System ("Transform_System", Priority::transform, given_registry)
    {
    }

    void Transform_System::execute (float )
    {
        update ();
    }

    void Transform_System::update ()
    {
        ++frame;

        if (frame == 0) { stamps.assign (stamps.size (), 0); frame = 1; }     // wrapped around

        broken_chains = 0;

        registry.view< Transform >().each ([this] (Entity entity, Transform & )
        {
            compute (entity, 0);
        });
    }

    const glm::mat4 & Transform_System::compute (Entity entity, int depth)
    {
        Transform & transform = registry.get< Transform >(entity);

        const std::uint32_t index = entity::index_of (entity);

        if (index >= stamps.size ()) stamps.resize (std::size_t(index) + 1, 0);

        if (stamps[index] == frame) return transform.world;

        // Mark before recursing: a cycle that comes back here stops at the check above.
        stamps[index] = frame;

        const glm::mat4 local = transform.get_local_matrix ();

        const bool has_parent = transform.parent != null_entity
                             && transform.parent != entity
                             && registry.valid (transform.parent)
                             && registry.has< Transform >(transform.parent);

        if (has_parent && depth < max_depth)
        {
            // Copy: compute() on the parent returns a reference into the same pool.
            const glm::mat4 parent_world = compute (transform.parent, depth + 1);

            transform.world = parent_world * local;
        }
        else
        {
            if (has_parent) ++broken_chains;

            transform.world = local;
        }

        return transform.world;
    }

}
