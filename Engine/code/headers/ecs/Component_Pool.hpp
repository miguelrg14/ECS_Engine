// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Entity.hpp"

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine
{

    /// What the Registry needs from a pool without knowing its component type.
    class IComponent_Pool
    {
    public:

        virtual ~IComponent_Pool () = default;

        virtual bool        has    (Entity entity) const = 0;
        virtual void        remove (Entity entity)       = 0;
        virtual std::size_t size   ()              const = 0;
        virtual void        clear  ()                    = 0;
    };

    /// Stores every component of type T in one contiguous array: a sparse set.
    ///
    ///   sparse[index]  -> position in dense/data, or `absent`
    ///   dense[i]       -> entity that owns data[i]
    ///   data[i]        -> the component
    ///
    /// add, get, has and remove are O(1). Removing moves the last component into
    /// the gap (swap and pop), so data stays packed and is iterated without holes.
    /// Order is not preserved.
    template< typename T >
    class Component_Pool final : public IComponent_Pool
    {
        static constexpr std::uint32_t absent = std::numeric_limits< std::uint32_t >::max ();

        std::vector< std::uint32_t > sparse;
        std::vector< Entity        > dense;
        std::vector< T             > data;

    public:

        template< typename ... Args >
        T & emplace (Entity entity, Args && ... arguments)
        {
            assert (!has (entity) && "the entity already has this component");

            const std::uint32_t index = entity::index_of (entity);

            if (index >= sparse.size ()) sparse.resize (std::size_t(index) + 1, absent);

            sparse[index] = std::uint32_t(dense.size ());
            dense.push_back (entity);

            // Aggregates (plain structs) are built with braces so Position{1, 2, 3} works.
            if constexpr (std::is_aggregate_v< T >)
            {
                return data.emplace_back (T{ std::forward< Args >(arguments)... });
            }
            else
            {
                return data.emplace_back (std::forward< Args >(arguments)...);
            }
        }

        void remove (Entity entity) override
        {
            assert (has (entity) && "the entity does not have this component");

            const std::uint32_t position = sparse[entity::index_of (entity)];
            const Entity        last     = dense.back ();

            // Move the last element into the gap and fix its sparse entry.
            if (position + 1 != dense.size ())
            {
                dense[position] = last;
                data [position] = std::move (data.back ());
                sparse[entity::index_of (last)] = position;
            }

            sparse[entity::index_of (entity)] = absent;

            dense.pop_back ();
            data .pop_back ();
        }

        bool has (Entity entity) const override
        {
            const std::uint32_t index = entity::index_of (entity);

            // Comparing the full entity rejects a stale handle with an old version.
            return index < sparse.size ()
                && sparse[index] != absent
                && dense[sparse[index]] == entity;
        }

        T & get (Entity entity)
        {
            assert (has (entity) && "the entity does not have this component");
            return data[sparse[entity::index_of (entity)]];
        }

        const T & get (Entity entity) const
        {
            assert (has (entity) && "the entity does not have this component");
            return data[sparse[entity::index_of (entity)]];
        }

        T * try_get (Entity entity)
        {
            return has (entity) ? &data[sparse[entity::index_of (entity)]] : nullptr;
        }

        std::size_t size () const override { return dense.size (); }

        void clear () override
        {
            sparse.clear ();
            dense .clear ();
            data  .clear ();
        }

        /// Packed entities and components, same order: entities()[i] owns components()[i].
        const std::vector< Entity > & entities   () const { return dense; }
        std::vector< T >            & components ()       { return data;  }
    };

}
