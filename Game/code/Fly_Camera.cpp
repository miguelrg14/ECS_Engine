// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Fly_Camera.hpp"

#include <algorithm>
#include <cmath>

void Fly_Camera::look_at (engine::Transform & transform, const glm::vec3 & from, const glm::vec3 & target)
{
    const glm::vec3 direction = glm::normalize (target - from);

    // Forward for a yaw y around Y is (-sin y, 0, -cos y).
    yaw   = glm::degrees (std::atan2 (-direction.x, -direction.z));
    pitch = glm::degrees (std::asin  (std::clamp (direction.y, -1.f, 1.f)));

    transform.position = from;
    apply_rotation (transform);
}

void Fly_Camera::update (engine::Transform & transform, const engine::Input & input, engine::Window & window, float delta_time)
{
    using engine::Mouse_Button;

    if (input.was_mouse_pressed  (Mouse_Button::right)) window.set_mouse_captured (true );
    if (input.was_mouse_released (Mouse_Button::right)) window.set_mouse_captured (false);

    if (input.is_mouse_down (Mouse_Button::right))
    {
        const glm::ivec2 delta = input.get_mouse_delta ();

        // Moving the mouse right turns right (negative yaw); moving it up looks up.
        yaw   -= float(delta.x) * sensitivity;
        pitch  = std::clamp (pitch - float(delta.y) * sensitivity, -89.f, 89.f);

        apply_rotation (transform);
    }

    glm::vec3 direction { 0.f };

    if (input.is_key_down (SDL_SCANCODE_W)) direction += transform.get_forward ();
    if (input.is_key_down (SDL_SCANCODE_S)) direction -= transform.get_forward ();
    if (input.is_key_down (SDL_SCANCODE_D)) direction += transform.get_right   ();
    if (input.is_key_down (SDL_SCANCODE_A)) direction -= transform.get_right   ();
    if (input.is_key_down (SDL_SCANCODE_E)) direction += glm::vec3(0.f, 1.f, 0.f);
    if (input.is_key_down (SDL_SCANCODE_Q)) direction -= glm::vec3(0.f, 1.f, 0.f);

    if (glm::length (direction) > 0.f)
    {
        const float boost = input.is_key_down (SDL_SCANCODE_LSHIFT) ? 3.f : 1.f;

        transform.position += glm::normalize (direction) * speed * boost * delta_time;
    }
}

void Fly_Camera::apply_rotation (engine::Transform & transform) const
{
    transform.rotation = glm::angleAxis (glm::radians (yaw),   glm::vec3(0.f, 1.f, 0.f))
                       * glm::angleAxis (glm::radians (pitch), glm::vec3(1.f, 0.f, 0.f));
}
