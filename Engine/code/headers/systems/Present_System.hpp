// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "core/Task.hpp"
#include "core/Window.hpp"

namespace engine
{

    /// Last task of every frame: shows what was drawn by swapping the window buffers.
    class Present_System final : public Task
    {
        Window & window;

    public:

        explicit Present_System (Window & given_window)
        :
            Task  ("Present_System", Priority::present),
            window(given_window)
        {
        }

        void execute (float ) override
        {
            window.swap_buffers ();
        }
    };

}
