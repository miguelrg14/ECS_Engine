// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "core/Task.hpp"

#include <SDL.h>
#include <functional>

namespace engine
{

    class Input;
    class Kernel;
    class Window;

    /// First task of every frame: reads the SDL event queue, updates Input,
    /// forwards resizes to the Window and stops the Kernel when the window is closed.
    class Input_System final : public Task
    {
    public:

        /// Sees every event first. Returns which kinds of input it took for itself
        /// (for example a debug window under the mouse), so the game ignores them.
        struct Capture { bool keyboard = false; bool mouse = false; };
        using Event_Filter = std::function< Capture (const SDL_Event & ) >;

    private:

        Input        & input;
        Window       & window;
        Kernel       & kernel;
        Event_Filter   filter;

    public:

        Input_System (Input & input, Window & window, Kernel & kernel);

        void execute (float delta_time) override;

        void set_event_filter (Event_Filter new_filter) { filter = std::move(new_filter); }
    };

}
