// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "renderer/Render_Command.hpp"

#include <glad/glad.h>

namespace engine::Render_Command
{

    void set_clear_color (const glm::vec4 & color)
    {
        glClearColor (color.r, color.g, color.b, color.a);
    }

    void clear ()
    {
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void set_viewport (int x, int y, int width, int height)
    {
        glViewport (x, y, width, height);
    }

}
