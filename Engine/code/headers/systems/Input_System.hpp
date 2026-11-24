// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "core/Task.hpp"

namespace engine
{

    class Input;
    class Kernel;
    class Window;

    /// First task of every frame: reads the SDL event queue, updates Input,
    /// forwards resizes to the Window and stops the Kernel when the window is closed.
    class Input_System final : public Task
    {
        Input  & input;
        Window & window;
        Kernel & kernel;

    public:

        Input_System (Input & input, Window & window, Kernel & kernel);

        void execute (float delta_time) override;
    };

}
