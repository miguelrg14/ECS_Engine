// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "renderer/Camera.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace engine
{

    void Camera::set_rotation (float new_yaw, float new_pitch)
    {
        yaw   = std::fmod (new_yaw, 360.f);
        pitch = std::clamp (new_pitch, -89.f, 89.f);
    }

    void Camera::look_at (const glm::vec3 & target)
    {
        const glm::vec3 direction = glm::normalize (target - position);

        set_rotation
        (
            glm::degrees (std::atan2 (direction.z, direction.x)),
            glm::degrees (std::asin  (std::clamp (direction.y, -1.f, 1.f)))
        );
    }

    void Camera::set_perspective (float fov, float near_z, float far_z)
    {
        fov_degrees = fov;
        near_plane  = near_z;
        far_plane   = far_z;
    }

    glm::vec3 Camera::get_forward () const
    {
        const float yaw_radians   = glm::radians (yaw  );
        const float pitch_radians = glm::radians (pitch);

        return glm::normalize (glm::vec3
        (
            std::cos (yaw_radians) * std::cos (pitch_radians),
            std::sin (pitch_radians),
            std::sin (yaw_radians) * std::cos (pitch_radians)
        ));
    }

    glm::vec3 Camera::get_right () const
    {
        return glm::normalize (glm::cross (get_forward (), glm::vec3(0.f, 1.f, 0.f)));
    }

    glm::mat4 Camera::get_view () const
    {
        return glm::lookAt (position, position + get_forward (), glm::vec3(0.f, 1.f, 0.f));
    }

    glm::mat4 Camera::get_projection () const
    {
        return glm::perspective (glm::radians (fov_degrees), aspect, near_plane, far_plane);
    }

}
