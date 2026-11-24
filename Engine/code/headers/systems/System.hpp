// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "core/Task.hpp"
#include "ecs/Registry.hpp"

#include <string>

namespace engine
{

    /// A task that works on the entities of a registry. Systems hold no entity
    /// data of their own: each frame they read and write components through views.
    class System : public Task
    {
    protected:

        Registry & registry;

    public:

        System (std::string given_name, int given_priority, Registry & given_registry)
        :
            Task    (std::move(given_name), given_priority),
            registry(given_registry)
        {
        }
    };

}
