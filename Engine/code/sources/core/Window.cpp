// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "core/Window.hpp"
#include "core/Log.hpp"

#include <glad/glad.h>
#include <stdexcept>

namespace engine
{

    Window::Window (const Settings & settings)
    :
        width (settings.width),
        height(settings.height)
    {
        if (SDL_InitSubSystem (SDL_INIT_VIDEO) != 0)
        {
            throw std::runtime_error (std::string("SDL_InitSubSystem failed: ") + SDL_GetError ());
        }

        // The context attributes must be set before the window is created.

        SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, int(settings.gl_major));
        SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, int(settings.gl_minor));
        SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER,          1);
        SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE,            int(settings.depth_bits));
        SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE,          int(settings.stencil_bits));

        #ifdef _DEBUG
        SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        #endif

        Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

        if (settings.resizable) flags |= SDL_WINDOW_RESIZABLE;

        window_handle = SDL_CreateWindow
        (
            settings.title.c_str (),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            int(settings.width),
            int(settings.height),
            flags
        );

        if (!window_handle)
        {
            throw std::runtime_error (std::string("SDL_CreateWindow failed: ") + SDL_GetError ());
        }

        opengl_context = SDL_GL_CreateContext (window_handle);

        if (!opengl_context)
        {
            SDL_DestroyWindow (window_handle);
            throw std::runtime_error (std::string("SDL_GL_CreateContext failed: ") + SDL_GetError ());
        }

        // GLAD resolves the OpenGL entry points of the context that is now current.

        if (!gladLoadGLLoader (reinterpret_cast< GLADloadproc >(SDL_GL_GetProcAddress)))
        {
            SDL_GL_DeleteContext (opengl_context);
            SDL_DestroyWindow    (window_handle);
            throw std::runtime_error ("gladLoadGLLoader failed: OpenGL 3.3 is not available");
        }

        set_vsync (settings.vsync);

        // With high DPI the drawable size can differ from the window size.

        int drawable_width, drawable_height;
        SDL_GL_GetDrawableSize (window_handle, &drawable_width, &drawable_height);
        on_resize (unsigned(drawable_width), unsigned(drawable_height));

        log_info ("OpenGL %s | %s | %s",
            reinterpret_cast< const char * >(glGetString (GL_VERSION )),
            reinterpret_cast< const char * >(glGetString (GL_VENDOR  )),
            reinterpret_cast< const char * >(glGetString (GL_RENDERER)));
    }

    Window::~Window ()
    {
        if (opengl_context) SDL_GL_DeleteContext (opengl_context);
        if (window_handle ) SDL_DestroyWindow    (window_handle );

        SDL_QuitSubSystem (SDL_INIT_VIDEO);
    }

    void Window::swap_buffers ()
    {
        SDL_GL_SwapWindow (window_handle);
    }

    void Window::on_resize (unsigned new_width, unsigned new_height)
    {
        width  = new_width;
        height = new_height;

        glViewport (0, 0, GLsizei(width), GLsizei(height));
    }

    void Window::set_title (const std::string & title)
    {
        SDL_SetWindowTitle (window_handle, title.c_str ());
    }

    void Window::set_vsync (bool enabled)
    {
        // Adaptive vsync (-1) first; plain vsync if the driver does not support it.

        if (enabled)
        {
            vsync = SDL_GL_SetSwapInterval (-1) == 0 || SDL_GL_SetSwapInterval (1) == 0;
        }
        else
        {
            SDL_GL_SetSwapInterval (0);
            vsync = false;
        }
    }

    void Window::set_mouse_captured (bool captured)
    {
        SDL_SetRelativeMouseMode (captured ? SDL_TRUE : SDL_FALSE);
    }

    bool Window::is_mouse_captured () const
    {
        return SDL_GetRelativeMouseMode () == SDL_TRUE;
    }

}
