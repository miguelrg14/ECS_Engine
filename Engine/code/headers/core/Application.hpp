// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Input.hpp"
#include "Kernel.hpp"
#include "Window.hpp"
#include "renderer/Renderer.hpp"
#include "resources/Resource_Manager.hpp"
#include "systems/Input_System.hpp"
#include "systems/Present_System.hpp"

namespace engine
{

    /// Owns the pieces every game needs (SDL, window, resources, renderer, input, kernel) and adds the
    /// engine's own tasks. The game adds its tasks to get_kernel() and calls run().
    class Application
    {
        /// Initializes SDL before any other member and shuts it down after all of them:
        /// members are built in declaration order and destroyed in reverse.
        struct SDL_Library
        {
             SDL_Library ();
            ~SDL_Library ();
        };

        SDL_Library      sdl;
        Window           window;
        Resource_Manager resources;      // after the window: GPU objects die before the GL context
        Renderer         renderer;
        Input            input;
        Kernel           kernel;
        Input_System     input_system;
        Present_System   present_system;

    public:

        explicit Application (const Window::Settings & settings);

        Application (const Application & ) = delete;
        Application & operator = (const Application & ) = delete;

        void run  () { kernel.run  (); }
        void quit () { kernel.stop (); }

        Window           & get_window    ()       { return window;    }
        Resource_Manager & get_resources ()       { return resources; }
        Renderer         & get_renderer  ()       { return renderer;  }
        const Input  & get_input  () const { return input;  }
        Kernel       & get_kernel ()       { return kernel; }
    };

}
