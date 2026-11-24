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

    void set_depth_test (bool enabled)
    {
        if (enabled) { glEnable (GL_DEPTH_TEST); glDepthFunc (GL_LESS); }
        else           glDisable (GL_DEPTH_TEST);
    }

    void set_face_culling (bool enabled)
    {
        if (enabled) { glEnable (GL_CULL_FACE); glCullFace (GL_BACK); }
        else           glDisable (GL_CULL_FACE);
    }

    void set_wireframe (bool enabled)
    {
        glPolygonMode (GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
    }

}
