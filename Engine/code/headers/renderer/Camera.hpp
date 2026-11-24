// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace engine
{

    /// Perspective camera described by a position and yaw/pitch angles in degrees.
    /// Yaw -90 looks down -Z; positive pitch looks up. It has no input handling:
    /// whoever moves it (a script, a debug controller) calls its setters.
    class Camera
    {
        glm::vec3 position { 0.f, 0.f, 0.f };
        float     yaw          = -90.f;
        float     pitch        =   0.f;

        float     fov_degrees  = 60.f;
        float     aspect       = 16.f / 9.f;
        float     near_plane   = 0.1f;
        float     far_plane    = 200.f;

    public:

        void set_position     (const glm::vec3 & new_position) { position = new_position; }
        void move             (const glm::vec3 & offset)       { position += offset; }

        /// Pitch is clamped to +/-89 degrees so the view never flips.
        void set_rotation     (float new_yaw, float new_pitch);
        void rotate           (float delta_yaw, float delta_pitch) { set_rotation (yaw + delta_yaw, pitch + delta_pitch); }

        /// Points the camera at a world position.
        void look_at          (const glm::vec3 & target);

        void set_perspective  (float fov, float near_z, float far_z);
        void set_aspect       (float new_aspect) { aspect = new_aspect; }

    public:

        const glm::vec3 & get_position () const { return position; }
        float             get_yaw      () const { return yaw;      }
        float             get_pitch    () const { return pitch;    }
        float             get_far      () const { return far_plane; }

        glm::vec3 get_forward () const;
        glm::vec3 get_right   () const;

        glm::mat4 get_view       () const;
        glm::mat4 get_projection () const;
    };

}
