// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <glm/vec4.hpp>

namespace engine
{

    /// Stateless OpenGL calls for render state. The rest of the engine goes
    /// through here instead of calling gl* directly, so the API stays in one place.
    namespace Render_Command
    {
        void set_clear_color  (const glm::vec4 & color);
        void clear            ();
        void set_viewport     (int x, int y, int width, int height);

        void set_depth_test   (bool enabled);
        void set_face_culling (bool enabled);
        void set_wireframe    (bool enabled);
    }

}
