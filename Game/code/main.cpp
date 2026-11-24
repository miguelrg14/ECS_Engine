// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

// Phase 1: opens a window with an OpenGL 3.3 context, clears it every frame
// and closes with Esc or the window's close button. The loop here moves into
// the engine's Kernel in phase 2.

#include <Engine.hpp>

#include <exception>

int main (int , char * [])
{
    if (SDL_Init (SDL_INIT_EVENTS) != 0)
    {
        engine::log_error ("SDL_Init failed: %s", SDL_GetError ());
        return 1;
    }

    try
    {
        engine::Window window ({ .title = "ECS Engine - Phase 1", .width = 1280, .height = 720 });

        engine::Render_Command::set_clear_color ({ 0.08f, 0.10f, 0.14f, 1.f });

        bool running = true;

        while (running)
        {
            SDL_Event event;

            while (SDL_PollEvent (&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                    {
                        running = false;
                        break;
                    }
                    case SDL_KEYDOWN:
                    {
                        if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
                        break;
                    }
                    case SDL_WINDOWEVENT:
                    {
                        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                        {
                            int width, height;
                            SDL_GL_GetDrawableSize (window.get_handle (), &width, &height);
                            window.on_resize (unsigned(width), unsigned(height));
                        }
                        break;
                    }
                }
            }

            engine::Render_Command::clear ();

            window.swap_buffers ();
        }
    }
    catch (const std::exception & exception)
    {
        engine::log_error ("%s", exception.what ());
        SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR, "ECS Engine", exception.what (), nullptr);
        SDL_Quit ();
        return 1;
    }

    SDL_Quit ();
    return 0;
}
