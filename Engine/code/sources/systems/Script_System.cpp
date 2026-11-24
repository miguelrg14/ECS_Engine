// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "systems/Script_System.hpp"

#include "scripting/Script_Component.hpp"

#include <algorithm>

namespace engine
{

    Script_System::Script_System
    (
        Registry & given_registry, Event_Bus & given_events, const Input & given_input,
        Window * given_window, Resource_Manager * given_resources
    )
    :
        System   ("Script_System", Priority::scripts, given_registry),
        events   (given_events   ),
        input    (given_input    ),
        window   (given_window   ),
        resources(given_resources)
    {
    }

    void Script_System::execute (float delta_time)
    {
        // List first: running a script can add or remove Script_Components,
        // which may move the pool in memory.

        snapshot.clear ();

        registry.view< Script_Component >().each ([this] (Entity entity, Script_Component & )
        {
            snapshot.push_back (entity);
        });

        // The view walks the pool backwards; reverse to run in creation order.
        std::reverse (snapshot.begin (), snapshot.end ());

        for (const Entity entity : snapshot)
        {
            // Index loop and fresh lookups: a script may bind more scripts or remove the component.
            for (std::size_t i = 0; ; ++i)
            {
                Script_Component * component = registry.try_get< Script_Component >(entity);

                if (!component || i >= component->scripts.size ()) break;

                Script * script = component->scripts[i].get ();

                if (!script) continue;

                if (!script->created)
                {
                    script->self              = entity;
                    script->registry_pointer  = &registry;
                    script->event_bus         = &events;
                    script->input_pointer     = &input;
                    script->window_pointer    = window;
                    script->resources_pointer = resources;
                    script->owner             = this;
                    script->created           = true;

                    script->on_create ();
                }

                script->on_update (delta_time);
            }
        }

        destroy_requested ();
    }

    void Script_System::finalize ()
    {
        destroy_requested ();

        registry.view< Script_Component >().each ([] (Script_Component & component)
        {
            for (auto & script : component.scripts)
            {
                if (script && script->created)
                {
                    script->on_destroy ();
                    script->created = false;
                }
            }
        });
    }

    void Script_System::request_destroy (Entity entity)
    {
        if (std::find (to_destroy.begin (), to_destroy.end (), entity) == to_destroy.end ())
        {
            to_destroy.push_back (entity);
        }
    }

    void Script_System::destroy_requested ()
    {
        // on_destroy may request more destructions: keep going until none are left.

        while (!to_destroy.empty ())
        {
            const Entity entity = to_destroy.back ();
            to_destroy.pop_back ();

            if (!registry.valid (entity)) continue;

            if (Script_Component * component = registry.try_get< Script_Component >(entity))
            {
                for (std::size_t i = 0; i < component->scripts.size (); ++i)
                {
                    Script * script = component->scripts[i].get ();

                    if (script && script->created)
                    {
                        script->created = false;
                        script->on_destroy ();
                    }

                    component = registry.try_get< Script_Component >(entity);     // may have moved
                    if (!component) break;
                }
            }

            if (registry.valid (entity)) registry.destroy (entity);     // on_destroy may have done it
        }
    }

}
