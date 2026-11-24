// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Player_Controller.hpp"
#include "Camera_Math.hpp"
#include "../Game_Events.hpp"

#include <algorithm>
#include <cmath>

using namespace engine;

void Player_Controller::on_update (float delta_time)
{
    Transform & transform = get< Transform >();

    const bool has_control = registry ().valid (camera)
                          && registry ().has< Camera_Component >(camera)
                          && registry ().get< Camera_Component >(camera).active;

    // Walking and turning.

    const glm::vec3 direction = has_control ? read_direction () : glm::vec3(0.f);

    if (glm::length (direction) > 0.f)
    {
        const float speed = input ().is_key_down (SDL_SCANCODE_LSHIFT) ? run_speed : walk_speed;

        transform.position   += direction * speed * delta_time;
        transform.position.x  = std::clamp (transform.position.x, -arena_half_size, arena_half_size);
        transform.position.z  = std::clamp (transform.position.z, -arena_half_size, arena_half_size);

        float target_yaw, unused_pitch;
        camera_math::yaw_pitch_from_direction (direction, target_yaw, unused_pitch);

        const glm::quat target_rotation = camera_math::rotation_from_yaw_pitch (target_yaw, 0.f);

        transform.rotation = glm::slerp (transform.rotation, target_rotation, 1.f - std::exp (-turn_speed * delta_time));
    }

    // Jumping and falling.

    if (has_control && grounded && input ().was_key_pressed (SDL_SCANCODE_SPACE))
    {
        vertical_velocity = jump_speed;
        grounded          = false;

        publish (Jumped{ entity () });
    }

    if (!grounded)
    {
        vertical_velocity    -= gravity * delta_time;
        transform.position.y += vertical_velocity * delta_time;

        if (transform.position.y <= 0.f)
        {
            transform.position.y = 0.f;
            grounded = true;

            publish (Landed{ entity (), -vertical_velocity });

            vertical_velocity = 0.f;
        }
    }
}

glm::vec3 Player_Controller::read_direction () const
{
    // The camera's forward, flattened onto the ground.
    glm::vec3 forward = registry ().get< Transform >(camera).get_forward ();
    forward.y = 0.f;

    forward = glm::length (forward) > 0.001f ? glm::normalize (forward) : glm::vec3(0.f, 0.f, -1.f);

    const glm::vec3 right = glm::cross (forward, glm::vec3(0.f, 1.f, 0.f));

    glm::vec3 direction { 0.f };

    if (input ().is_key_down (SDL_SCANCODE_W)) direction += forward;
    if (input ().is_key_down (SDL_SCANCODE_S)) direction -= forward;
    if (input ().is_key_down (SDL_SCANCODE_D)) direction += right;
    if (input ().is_key_down (SDL_SCANCODE_A)) direction -= right;

    return glm::length (direction) > 0.f ? glm::normalize (direction) : direction;
}
