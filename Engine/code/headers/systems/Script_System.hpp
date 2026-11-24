// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "System.hpp"
#include "events/Event_Bus.hpp"

#include <vector>

namespace engine
{

    class Input;
    class Resource_Manager;
    class Window;

    /// Runs every Script of every Script_Component: on_create the first time,
    /// then on_update each frame.
    ///
    /// The entities are listed before running any script, so scripts may create
    /// entities and bind scripts (they start next frame) or destroy entities
    /// through Script::destroy (applied at the end of the pass).
    class Script_System final : public System
    {
        friend class Script;

        Event_Bus        & events;
        const Input      & input;
        Window           * window;
        Resource_Manager * resources;

        std::vector< Entity > snapshot;
        std::vector< Entity > to_destroy;

    public:

        Script_System (Registry & registry, Event_Bus & events, const Input & input,
                       Window * window = nullptr, Resource_Manager * resources = nullptr);

        void execute  (float delta_time) override;

        /// Calls on_destroy on every created script (the game is ending).
        void finalize () override;

    private:

        void request_destroy (Entity entity);
        void destroy_requested ();
    };

}
