// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Test_Framework.hpp"

#include <ecs/Registry.hpp>

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <vector>

using engine::Entity;
using engine::Registry;

namespace
{

    struct Position { float x = 0, y = 0, z = 0; };
    struct Velocity { float x = 0, y = 0, z = 0; };
    struct Health   { int   value = 100; };
    struct Tag      { };

    /// Not an aggregate: built through its constructor.
    class Name
    {
        std::string text;

    public:

        explicit Name (std::string given) : text(std::move(given)) {}
        const std::string & get () const { return text; }
    };

    /// Move-only component.
    struct Owned_Buffer
    {
        std::unique_ptr< int[] > values;
    };

}

// ---------------------------------------------------------------------------
// Entities

TEST (create_returns_unique_valid_entities)
{
    Registry registry;
    std::set< Entity > seen;

    for (int i = 0; i < 1000; ++i)
    {
        const Entity entity = registry.create ();

        CHECK (registry.valid (entity));
        CHECK (seen.insert (entity).second);
    }

    CHECK (registry.alive_count () == 1000);
}

TEST (null_entity_is_never_valid)
{
    Registry registry;
    registry.create ();

    CHECK (!registry.valid (engine::null_entity));
}

TEST (destroyed_entity_becomes_invalid)
{
    Registry registry;
    const Entity entity = registry.create ();

    registry.destroy (entity);

    CHECK (!registry.valid (entity));
    CHECK (registry.alive_count () == 0);
}

TEST (reused_index_gets_new_version)
{
    Registry registry;

    const Entity old_entity = registry.create ();
    registry.add< Health >(old_entity);
    registry.destroy (old_entity);

    const Entity new_entity = registry.create ();
    registry.add< Health >(new_entity, 5);

    // Same slot, different handle: the old one must not reach the new entity's data.
    CHECK (engine::entity::index_of   (new_entity) == engine::entity::index_of (old_entity));
    CHECK (engine::entity::version_of (new_entity) == engine::entity::version_of (old_entity) + 1);
    CHECK (!registry.valid (old_entity));
    CHECK ( registry.valid (new_entity));
    CHECK (!registry.has< Health >(old_entity));
    CHECK ( registry.has< Health >(new_entity));
}

TEST (clear_invalidates_every_handle)
{
    Registry registry;

    const Entity a = registry.create ();
    const Entity b = registry.create ();
    registry.add< Position >(a);
    registry.destroy (b);

    registry.clear ();

    CHECK (!registry.valid (a));
    CHECK (!registry.valid (b));
    CHECK (registry.alive_count () == 0);
    CHECK (registry.count< Position >() == 0);

    const Entity c = registry.create ();

    CHECK (registry.valid (c));
    CHECK (c != a && c != b);
}

// ---------------------------------------------------------------------------
// Components

TEST (add_get_has_remove)
{
    Registry registry;
    const Entity entity = registry.create ();

    CHECK (!registry.has< Position >(entity));

    registry.add< Position >(entity, 1.f, 2.f, 3.f);

    CHECK (registry.has< Position >(entity));
    CHECK (registry.get< Position >(entity).y == 2.f);

    registry.get< Position >(entity).y = 7.f;
    CHECK (registry.get< Position >(entity).y == 7.f);

    registry.remove< Position >(entity);
    CHECK (!registry.has< Position >(entity));
    CHECK ( registry.valid (entity));
}

TEST (try_get_returns_null_when_missing)
{
    Registry registry;
    const Entity entity = registry.create ();

    CHECK (registry.try_get< Health >(entity) == nullptr);

    registry.add< Health >(entity, 42);

    CHECK (registry.try_get< Health >(entity) != nullptr);
    CHECK (registry.try_get< Health >(entity)->value == 42);
}

TEST (all_of_checks_every_type)
{
    Registry registry;
    const Entity entity = registry.create ();

    registry.add< Position >(entity);
    CHECK (!(registry.all_of< Position, Velocity >(entity)));

    registry.add< Velocity >(entity);
    CHECK ( (registry.all_of< Position, Velocity >(entity)));
}

TEST (non_aggregate_and_move_only_components)
{
    Registry registry;
    const Entity entity = registry.create ();

    registry.add< Name >(entity, "player");
    registry.add< Owned_Buffer >(entity, std::make_unique< int[] >(4));

    registry.get< Owned_Buffer >(entity).values[3] = 9;

    CHECK (registry.get< Name >(entity).get () == "player");
    CHECK (registry.get< Owned_Buffer >(entity).values[3] == 9);
}

TEST (remove_keeps_other_components_intact)
{
    // Swap and pop moves the last component into the removed one's place.
    Registry registry;
    std::vector< Entity > entities;

    for (int i = 0; i < 10; ++i)
    {
        const Entity entity = registry.create ();
        registry.add< Health >(entity, i);
        entities.push_back (entity);
    }

    registry.remove< Health >(entities[0]);
    registry.remove< Health >(entities[4]);
    registry.remove< Health >(entities[9]);         // the last one: no swap

    CHECK (registry.count< Health >() == 7);

    for (int i = 0; i < 10; ++i)
    {
        const bool removed = (i == 0 || i == 4 || i == 9);

        CHECK (registry.has< Health >(entities[std::size_t(i)]) == !removed);

        if (!removed) CHECK (registry.get< Health >(entities[std::size_t(i)]).value == i);
    }
}

TEST (destroy_removes_every_component)
{
    Registry registry;
    const Entity entity = registry.create ();
    const Entity other  = registry.create ();

    registry.add< Position >(entity);
    registry.add< Velocity >(entity);
    registry.add< Health   >(entity);
    registry.add< Health   >(other, 3);

    registry.destroy (entity);

    CHECK (registry.count< Position >() == 0);
    CHECK (registry.count< Velocity >() == 0);
    CHECK (registry.count< Health   >() == 1);
    CHECK (registry.get< Health >(other).value == 3);
}

// ---------------------------------------------------------------------------
// Views

TEST (view_visits_exactly_the_matching_entities)
{
    Registry registry;
    std::set< Entity > expected;

    for (int i = 0; i < 100; ++i)
    {
        const Entity entity = registry.create ();

        registry.add< Position >(entity);
        if (i % 2 == 0) registry.add< Velocity >(entity);
        if (i % 3 == 0) registry.add< Health   >(entity);

        if (i % 2 == 0 && i % 3 == 0) expected.insert (entity);
    }

    std::set< Entity > visited;

    registry.view< Position, Velocity, Health >().each ([&visited] (Entity entity, Position &, Velocity &, Health &)
    {
        visited.insert (entity);
    });

    CHECK (visited == expected);
    CHECK (registry.view< Position, Velocity, Health >().count () == expected.size ());
}

TEST (view_walks_the_smallest_pool)
{
    Registry registry;

    for (int i = 0; i < 1000; ++i)
    {
        const Entity entity = registry.create ();
        registry.add< Position >(entity);
        if (i < 5) registry.add< Tag >(entity);
    }

    CHECK ((registry.view< Position, Tag >().size_hint ()) == 5);
    CHECK ((registry.view< Tag, Position >().size_hint ()) == 5);
}

TEST (view_of_unused_type_is_empty)
{
    Registry registry;
    registry.add< Position >(registry.create ());

    int calls = 0;
    registry.view< Position, Velocity >().each ([&calls] (Position &, Velocity &) { ++calls; });

    CHECK (calls == 0);
}

TEST (view_callback_without_entity_changes_components)
{
    Registry registry;

    for (int i = 0; i < 10; ++i)
    {
        const Entity entity = registry.create ();
        registry.add< Position >(entity);
        registry.add< Velocity >(entity, 1.f, 2.f, 0.f);
    }

    const float dt = 0.5f;

    registry.view< Position, Velocity >().each ([dt] (Position & position, Velocity & velocity)
    {
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;
    });

    registry.view< Position >().each ([] (Position & position)
    {
        CHECK (position.x == 0.5f);
        CHECK (position.y == 1.0f);
    });
}

TEST (destroy_during_iteration_is_safe)
{
    Registry registry;

    for (int i = 0; i < 50; ++i)
    {
        const Entity entity = registry.create ();
        registry.add< Health >(entity, i % 2 == 0 ? 0 : 10);
    }

    // Destroy the dead ones while walking the pool that holds them.
    registry.view< Health >().each ([&registry] (Entity entity, Health & health)
    {
        if (health.value == 0) registry.destroy (entity);
    });

    CHECK (registry.alive_count () == 25);
    CHECK (registry.count< Health >() == 25);

    registry.view< Health >().each ([] (Health & health) { CHECK (health.value == 10); });
}

TEST (ten_thousand_entities_half_destroyed)
{
    Registry registry;
    std::vector< Entity > entities;

    for (int i = 0; i < 10000; ++i)
    {
        const Entity entity = registry.create ();
        registry.add< Position >(entity, float(i), 0.f, 0.f);
        registry.add< Velocity >(entity);
        entities.push_back (entity);
    }

    for (std::size_t i = 0; i < entities.size (); i += 2) registry.destroy (entities[i]);

    CHECK (registry.alive_count () == 5000);

    std::size_t visited = 0;

    registry.view< Position, Velocity >().each ([&] (Entity entity, Position & position, Velocity &)
    {
        ++visited;

        // Only odd ones survive, and each still has its own data.
        CHECK (registry.valid (entity));
        CHECK (int(position.x) % 2 == 1);
    });

    CHECK (visited == 5000);

    // Freed slots are reused before the registry grows.
    for (int i = 0; i < 5000; ++i) registry.create ();

    CHECK (registry.alive_count () == 10000);
    CHECK (!registry.valid (entities[0]));
}

TEST (each_visits_every_live_entity_once)
{
    Registry registry;
    std::vector< Entity > created;

    for (int i = 0; i < 20; ++i) created.push_back (registry.create ());

    registry.destroy (created[3]);
    registry.destroy (created[7]);
    const Entity reused = registry.create ();       // takes slot 7 back

    std::set< Entity > visited;
    registry.each ([&visited] (Entity entity) { visited.insert (entity); });

    CHECK (visited.size () == 19);
    CHECK (visited.count (created[3]) == 0);
    CHECK (visited.count (created[7]) == 0);
    CHECK (visited.count (reused) == 1);

    registry.clear ();

    int after_clear = 0;
    registry.each ([&after_clear] (Entity ) { ++after_clear; });
    CHECK (after_clear == 0);
}
