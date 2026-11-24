// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Script.hpp"

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine
{

    /// The scripts attached to an entity, run in the order they were bound.
    ///
    ///     registry.add< Script_Component >(player).bind< Player_Controller >(camera);
    ///     registry.get< Script_Component >(player).bind< Squash_On_Land >();
    struct Script_Component
    {
        std::vector< std::unique_ptr< Script > > scripts;

        template< typename T, typename ... Args >
        T & bind (Args && ... arguments)
        {
            static_assert (std::is_base_of_v< Script, T >, "scripts derive from engine::Script");

            auto script = std::make_unique< T >(std::forward< Args >(arguments)...);
            T  & result = *script;

            scripts.push_back (std::move(script));
            return result;
        }
    };

}
