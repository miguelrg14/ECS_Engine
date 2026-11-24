// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "ecs/Registry.hpp"

#include <stdexcept>

namespace engine
{

    Entity Registry::create ()
    {
        ++alive;

        if (!free_indices.empty ())
        {
            // Reuse a slot. Its version was already incremented when it was destroyed.
            const std::uint32_t index = free_indices.back ();
            free_indices.pop_back ();
            return slots[index];
        }

        if (slots.size () >= entity::max_entities)
        {
            --alive;
            throw std::length_error ("Registry: too many entities");
        }

        const Entity entity = entity::make (std::uint32_t(slots.size ()), 0);
        slots.push_back (entity);
        return entity;
    }

    void Registry::destroy (Entity entity)
    {
        assert (valid (entity) && "destroy on an invalid entity");

        if (!valid (entity)) return;

        for (auto & component_pool : pools)
        {
            if (component_pool && component_pool->has (entity)) component_pool->remove (entity);
        }

        const std::uint32_t index   = entity::index_of   (entity);
        const std::uint32_t version = entity::version_of (entity) + 1;     // wraps after 255 reuses

        slots[index] = entity::make (index, version);
        free_indices.push_back (index);

        --alive;
    }

    bool Registry::valid (Entity entity) const
    {
        if (entity == null_entity) return false;

        const std::uint32_t index = entity::index_of (entity);

        return index < slots.size () && slots[index] == entity;
    }

    void Registry::clear ()
    {
        for (auto & component_pool : pools)
        {
            if (component_pool) component_pool->clear ();
        }

        // Live slots get a new version, as if destroyed one by one, so old handles
        // stay invalid. Free slots already got theirs when they were destroyed.

        std::vector< bool > is_free (slots.size (), false);

        for (std::uint32_t index : free_indices) is_free[index] = true;

        free_indices.clear ();

        for (std::uint32_t index = std::uint32_t(slots.size ()); index-- > 0; )
        {
            if (!is_free[index])
            {
                slots[index] = entity::make (index, entity::version_of (slots[index]) + 1);
            }

            free_indices.push_back (index);     // pushed last to first, so index 0 is reused first
        }

        alive = 0;
    }

}
