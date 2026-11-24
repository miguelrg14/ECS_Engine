// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "core/Task.hpp"
#include "events/Event_Bus.hpp"

namespace engine
{

    /// Delivers the events queued during the frame, after scripts, movement and
    /// collisions have run and before the frame is drawn.
    class Event_System final : public Task
    {
        Event_Bus & events;

    public:

        explicit Event_System (Event_Bus & given_events)
        :
            Task  ("Event_System", Priority::events),
            events(given_events)
        {
        }

        void execute (float ) override
        {
            events.dispatch ();
        }
    };

}
