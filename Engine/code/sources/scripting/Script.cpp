// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "scripting/Script.hpp"
#include "systems/Script_System.hpp"

namespace engine
{

    Script::~Script ()
    {
        // The event bus outlives the registry that owns the scripts (see Application).
        if (event_bus)
        {
            for (Subscription & subscription : subscriptions) event_bus->unsubscribe (subscription);
        }
    }

    void Script::destroy (Entity target) const
    {
        if (owner) owner->request_destroy (target == null_entity ? self : target);
    }

}
