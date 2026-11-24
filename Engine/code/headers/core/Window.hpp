// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <SDL.h>
#include <string>

namespace engine
{

    /// SDL2 window with an OpenGL context whose functions are loaded with GLAD.
    /// Owns both handles (RAII): they are released when the object is destroyed.
    class Window
    {
    public:

        struct Settings
        {
            std::string title          = "ECS Engine";
            unsigned    width          = 1280;
            unsigned    height         = 720;
            bool        resizable      = true;
            bool        vsync          = true;
            unsigned    gl_major       = 3;
            unsigned    gl_minor       = 3;
            unsigned    depth_bits     = 24;
            unsigned    stencil_bits   = 8;
        };

    private:

        SDL_Window  * window_handle  = nullptr;
        SDL_GLContext opengl_context = nullptr;
        unsigned      width          = 0;
        unsigned      height         = 0;
        bool          vsync          = false;

    public:

        explicit Window (const Settings & settings);
                ~Window ();

        Window (const Window & ) = delete;
        Window & operator = (const Window & ) = delete;

        Window (Window && ) = delete;
        Window & operator = (Window && ) = delete;

    public:

        /// Shows the frame drawn in the back buffer.
        void swap_buffers ();

        /// Called when SDL reports a new drawable size.
        void on_resize (unsigned new_width, unsigned new_height);

        void set_title (const std::string & title);
        void set_vsync (bool enabled);

        unsigned      get_width   () const { return width;  }
        unsigned      get_height  () const { return height; }
        bool          is_vsync    () const { return vsync;  }
        float         get_aspect  () const { return height > 0 ? float(width) / float(height) : 1.f; }
        SDL_Window  * get_handle  () const { return window_handle;  }
        SDL_GLContext get_context () const { return opengl_context; }
    };

}
