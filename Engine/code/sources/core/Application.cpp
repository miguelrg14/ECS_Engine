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
        window          (settings),
        input_system    (input, window, kernel),
        script_system   (registry, events, input, &window, &resources),
        movement_system (registry),
        transform_system(registry),
        event_system    (events),
        render_system   (registry, renderer, window),
        present_system  (window)
    {
        kernel.add (input_system    );
        kernel.add (script_system   );
        kernel.add (movement_system );
        kernel.add (transform_system);
        kernel.add (event_system    );
        kernel.add (render_system   );
        kernel.add (present_system  );
    }

}
