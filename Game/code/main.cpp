// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Demo_Scene.hpp"

#include <Engine.hpp>
#include <exception>

int main (int , char * [])
{
    try
    {
        engine::Application application ({ .title = "ECS Engine", .width = 1280, .height = 720 });

        Demo_Scene scene (application);

        application.get_kernel ().add (scene);
        application.run ();
    }
    catch (const std::exception & exception)
    {
        engine::log_error ("%s", exception.what ());
        SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR, "ECS Engine", exception.what (), nullptr);
        return 1;
    }

    return 0;
}
