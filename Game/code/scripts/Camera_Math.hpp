// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <cmath>

/// Yaw / pitch helpers shared by the camera scripts. Yaw 0 looks down -Z.
namespace camera_math
{

    /// Forward for yaw y and pitch p is (-sin y cos p, sin p, -cos y cos p).
    inline glm::vec3 direction_from_yaw_pitch (float yaw_degrees, float pitch_degrees)
    {
        const float yaw   = glm::radians (yaw_degrees);
        const float pitch = glm::radians (pitch_degrees);

        return { -std::sin (yaw) * std::cos (pitch), std::sin (pitch), -std::cos (yaw) * std::cos (pitch) };
    }

    inline void yaw_pitch_from_direction (const glm::vec3 & direction, float & yaw_degrees, float & pitch_degrees)
    {
        const glm::vec3 d = glm::normalize (direction);

        yaw_degrees   = glm::degrees (std::atan2 (-d.x, -d.z));
        pitch_degrees = glm::degrees (std::asin  (std::clamp (d.y, -1.f, 1.f)));
    }

    inline glm::quat rotation_from_yaw_pitch (float yaw_degrees, float pitch_degrees)
    {
        return glm::angleAxis (glm::radians (yaw_degrees),   glm::vec3(0.f, 1.f, 0.f))
             * glm::angleAxis (glm::radians (pitch_degrees), glm::vec3(1.f, 0.f, 0.f));
    }

}
