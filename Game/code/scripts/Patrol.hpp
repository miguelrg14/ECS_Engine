// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Camera_Math.hpp"

#include <Engine.hpp>
#include <cmath>
#include <vector>

/// Moves the entity along a closed loop of waypoints on the ground plane (x, z)
/// at constant speed, turning to face where it goes. Height is left alone,
/// so it combines with Bob.
class Patrol final : public engine::Script
{
    std::vector< glm::vec3 > waypoints;
    float                    speed;
    std::size_t              next = 0;

public:

    Patrol (std::vector< glm::vec3 > given_waypoints, float given_speed)
    :
        waypoints(std::move(given_waypoints)),
        speed    (given_speed)
    {
    }

    void on_update (float delta_time) override
    {
        if (waypoints.size () < 2) return;

        engine::Transform & transform = get< engine::Transform >();

        float step = speed * delta_time;

        // Several waypoints can be passed in one long frame.
        while (step > 0.f)
        {
            glm::vec3 to_target = waypoints[next] - transform.position;
            to_target.y = 0.f;

            const float distance = glm::length (to_target);

            if (distance <= step)
            {
                transform.position.x = waypoints[next].x;
                transform.position.z = waypoints[next].z;
                step -= distance;
                next  = (next + 1) % waypoints.size ();
                continue;
            }

            const glm::vec3 direction = to_target / distance;

            transform.position += direction * step;
            step = 0.f;

            float yaw, pitch;
            camera_math::yaw_pitch_from_direction (direction, yaw, pitch);

            transform.rotation = glm::slerp
            (
                transform.rotation, camera_math::rotation_from_yaw_pitch (yaw, 0.f), 1.f - std::exp (-8.f * delta_time)
            );
        }
    }
};
