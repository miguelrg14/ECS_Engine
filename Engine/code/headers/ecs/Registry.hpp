// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Component_Pool.hpp"
#include "Entity.hpp"
#include "View.hpp"

#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

namespace engine
{

    namespace detail
    {
        /// Gives every component type a small consecutive number, assigned the
        /// first time the type is used. It indexes the registry's pool array.
        inline std::size_t next_component_id ()
        {
            static std::size_t counter = 0;
            return counter++;
        }

        template< typename T >
        std::size_t component_id ()
        {
            static const std::size_t id = next_component_id ();
            return id;
        }
    }

    /// Owns every entity and component of a world.
    ///
    ///     Entity player = registry.create ();
    ///     registry.add< Transform >(player, glm::vec3{ 0, 1, 0 });
    ///
    ///     registry.view< Transform, Velocity >().each ([dt] (Transform & t, Velocity & v)
    ///     {
    ///         t.position += v.linear * dt;
    ///     });
    ///
    /// Components are plain structs: no base class, no virtual functions.
    /// Each type lives in its own Component_Pool, created the first time it is added.
    class Registry
    {
        std::vector< Entity >        slots;              // slots[i] = entity using index i (current version)
        std::vector< std::uint32_t > free_indices;       // destroyed slots, reused first
        std::vector< bool          > in_use;             // in_use[i]: slot i holds a live entity
        std::size_t                  alive = 0;

        std::vector< std::unique_ptr< IComponent_Pool > > pools;

    public:

        Registry () = default;

        Registry (const Registry & ) = delete;
        Registry & operator = (const Registry & ) = delete;

    public:

        Entity create  ();

        /// Removes all its components and frees its index for reuse.
        void   destroy (Entity entity);

        /// False for null_entity and for handles of destroyed entities.
        bool   valid   (Entity entity) const;

        std::size_t alive_count () const { return alive; }

        /// Destroys every entity and component.
        void   clear   ();

        /// Calls function(entity) for every live entity, in slot order.
        template< typename Function >
        void each (Function && function) const
        {
            for (std::size_t index = 0; index < slots.size (); ++index)
            {
                if (in_use[index]) function (slots[index]);
            }
        }

    public:

        template< typename T, typename ... Args >
        T & add (Entity entity, Args && ... arguments)
        {
            assert (valid (entity) && "add on an invalid entity");
            return pool< T >().emplace (entity, std::forward< Args >(arguments)...);
        }

        template< typename T >
        void remove (Entity entity)
        {
            assert (valid (entity) && "remove on an invalid entity");
            pool< T >().remove (entity);
        }

        template< typename T >
        bool has (Entity entity) const
        {
            const Component_Pool< T > * found = find_pool< T >();
            return found && found->has (entity);
        }

        template< typename ... Ts >
        bool all_of (Entity entity) const
        {
            return (has< Ts >(entity) && ...);
        }

        template< typename T >
        T & get (Entity entity)
        {
            assert (has< T >(entity) && "get of a component the entity does not have");
            return pool< T >().get (entity);
        }

        template< typename T >
        const T & get (Entity entity) const
        {
            assert (has< T >(entity) && "get of a component the entity does not have");
            return find_pool< T >()->get (entity);
        }

        /// Pointer to the component, or nullptr if the entity does not have it.
        template< typename T >
        T * try_get (Entity entity)
        {
            Component_Pool< T > * found = find_pool< T >();
            return found ? found->try_get (entity) : nullptr;
        }

        template< typename T >
        std::size_t count () const
        {
            const Component_Pool< T > * found = find_pool< T >();
            return found ? found->size () : 0;
        }

        template< typename ... Ts >
        View< Ts... > view ()
        {
            return View< Ts... >(find_pool< Ts >()...);
        }

    private:

        /// The pool of T, created on first use.
        template< typename T >
        Component_Pool< T > & pool ()
        {
            static_assert (std::is_same_v< T, std::decay_t< T > >, "use the plain component type");

            const std::size_t id = detail::component_id< T >();

            if (id >= pools.size ()) pools.resize (id + 1);

            if (!pools[id]) pools[id] = std::make_unique< Component_Pool< T > >();

            return static_cast< Component_Pool< T > & >(*pools[id]);
        }

        /// The pool of T, or nullptr if no component of that type was ever added.
        template< typename T >
        Component_Pool< T > * find_pool () const
        {
            const std::size_t id = detail::component_id< T >();

            return id < pools.size () ? static_cast< Component_Pool< T > * >(pools[id].get ()) : nullptr;
        }
    };

}
