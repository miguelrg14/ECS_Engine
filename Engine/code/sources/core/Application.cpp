// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "core/Application.hpp"

#include <stdexcept>
#include <string>

namespace engine
{

    Application::SDL_Library::SDL_Library ()
    {
        if (SDL_Init (SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
        {
            throw std::runtime_error (std::string("SDL_Init failed: ") + SDL_GetError ());
        }
    }

    Application::SDL_Library::~SDL_Library ()
    {
        SDL_Quit ();
    }

    Application::Application (const Window::Settings & settings)
    :
        window        (settings),
        input_system  (input, window, kernel),
        present_system(window)
    {
        kernel.add (input_system  );
        kernel.add (present_system);
    }

}
