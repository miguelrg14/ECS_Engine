// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <SDL.h>
#include <utility>

namespace engine
{

    // Thin wrappers over SDL_Log, which writes to the console and to the
    // Visual Studio Output window. Format strings follow printf.

    template< typename ... Args >
    inline void log_info (const char * format, Args && ... arguments)
    {
        SDL_LogInfo (SDL_LOG_CATEGORY_APPLICATION, format, std::forward< Args >(arguments)...);
    }

    template< typename ... Args >
    inline void log_warning (const char * format, Args && ... arguments)
    {
        SDL_LogWarn (SDL_LOG_CATEGORY_APPLICATION, format, std::forward< Args >(arguments)...);
    }

    template< typename ... Args >
    inline void log_error (const char * format, Args && ... arguments)
    {
        SDL_LogError (SDL_LOG_CATEGORY_APPLICATION, format, std::forward< Args >(arguments)...);
    }

    // Messages without arguments are printed as they are, never read as a format.
    inline void log_info    (const char * message) { SDL_LogInfo  (SDL_LOG_CATEGORY_APPLICATION, "%s", message); }
    inline void log_warning (const char * message) { SDL_LogWarn  (SDL_LOG_CATEGORY_APPLICATION, "%s", message); }
    inline void log_error   (const char * message) { SDL_LogError (SDL_LOG_CATEGORY_APPLICATION, "%s", message); }

}
