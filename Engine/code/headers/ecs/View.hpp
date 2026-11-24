// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Component_Pool.hpp"

#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <vector>

namespace engine
{

    /// Every entity that has all of the components Ts... at the time of iteration.
    ///
    /// each() walks the entities of the smallest pool and checks the others, so
    /// its cost depends on the rarest component, not on the number of entities.
    ///
    /// The callback takes (Entity, Ts &...) or just (Ts &...). It runs from the
    /// last packed element to the first, so the callback may remove components
    /// from, or destroy, the entity it is visiting. Adding components of the
    /// viewed types while iterating is not supported: it can move them in memory.
    template< typename ... Ts >
    class View
    {
        static_assert (sizeof...(Ts) > 0, "a view needs at least one component type");

        std::tuple< Component_Pool< Ts > * ... > pools;

    public:

        /// A null pool (component type never added) makes the view empty.
        explicit View (Component_Pool< Ts > * ... given_pools) : pools(given_pools...) {}

        template< typename Function >
        void each (Function && function)
        {
            const std::vector< Entity > * candidates = smallest_entity_list ();

            if (!candidates) return;

            for (std::size_t i = candidates->size (); i-- > 0; )
            {
                if (i >= candidates->size ()) continue;       // the callback removed several elements

                const Entity entity = (*candidates)[i];

                if (!(std::get< Component_Pool< Ts > * >(pools)->has (entity) && ...)) continue;

                if constexpr (std::is_invocable_v< Function &, Entity, Ts & ... >)
                {
                    function (entity, std::get< Component_Pool< Ts > * >(pools)->get (entity)...);
                }
                else
                {
                    static_assert (std::is_invocable_v< Function &, Ts & ... >,
                                   "the callback must take (Entity, Ts &...) or (Ts &...)");

                    function (std::get< Component_Pool< Ts > * >(pools)->get (entity)...);
                }
            }
        }

        /// Upper bound of the number of entities in the view: the smallest pool's size.
        std::size_t size_hint () const
        {
            const std::vector< Entity > * candidates = smallest_entity_list ();
            return candidates ? candidates->size () : 0;
        }

        /// Exact number of entities in the view; walks it.
        std::size_t count ()
        {
            std::size_t total = 0;
            each ([&total] (Entity, Ts & ...) { ++total; });
            return total;
        }

    private:

        /// Entities of the pool with fewest components, or nullptr if a pool is missing.
        const std::vector< Entity > * smallest_entity_list () const
        {
            if (!((std::get< Component_Pool< Ts > * >(pools) != nullptr) && ...)) return nullptr;

            const std::vector< Entity > * smallest = nullptr;

            auto consider = [&smallest] (const std::vector< Entity > & list)
            {
                if (!smallest || list.size () < smallest->size ()) smallest = &list;
            };

            (consider (std::get< Component_Pool< Ts > * >(pools)->entities ()), ...);

            return smallest;
        }
    };

}
