// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "systems/Input_System.hpp"

#include "core/Input.hpp"
#include "core/Kernel.hpp"
#include "core/Window.hpp"

namespace engine
{

    Input_System::Input_System (Input & given_input, Window & given_window, Kernel & given_kernel)
    :
        Task  ("Input_System", Priority::input),
        input (given_input ),
        window(given_window),
        kernel(given_kernel)
    {
    }

    void Input_System::execute (float )
    {
        input.begin_frame ();

        SDL_Event event;

        while (SDL_PollEvent (&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                {
                    input.quit_requested = true;
                    kernel.stop ();
                    break;
                }

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    // Held keys repeat KEYDOWN events; the state is already "down".
                    if (event.key.repeat == 0)
                    {
                        input.keys_current[event.key.keysym.scancode] = event.type == SDL_KEYDOWN;
                    }
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                {
                    input.mouse_current |=  Uint32(SDL_BUTTON(event.button.button));
                    break;
                }

                case SDL_MOUSEBUTTONUP:
                {
                    input.mouse_current &= ~Uint32(SDL_BUTTON(event.button.button));
                    break;
                }

                case SDL_MOUSEMOTION:
                {
                    input.mouse_position  = { event.motion.x,    event.motion.y    };
                    input.mouse_delta    += glm::ivec2{ event.motion.xrel, event.motion.yrel };
                    break;
                }

                case SDL_MOUSEWHEEL:
                {
                    input.mouse_wheel += event.wheel.preciseY;
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
                    else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    {
                        // Keys released while the window has no focus never send KEYUP.
                        input.keys_current.fill (false);
                        input.mouse_current = 0;
                    }
                    break;
                }
            }
        }
    }

}
