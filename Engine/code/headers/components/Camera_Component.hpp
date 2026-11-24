// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

namespace engine
{

    /// Perspective camera placed by the entity's Transform; it looks down its -Z axis.
    /// Render_System draws from the first active one it finds.
    struct Camera_Component
    {
        float fov_degrees = 60.f;
        float near_plane  = 0.1f;
        float far_plane   = 200.f;
        bool  active      = true;
    };

}
