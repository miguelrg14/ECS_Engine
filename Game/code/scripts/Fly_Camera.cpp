// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Fly_Camera.hpp"
#include "Camera_Math.hpp"

#include <algorithm>

using namespace engine;

void Fly_Camera::on_update (float delta_time)
{
    const bool active = get< Camera_Component >().active;

    if (!active) { was_active = false; return; }

    if (!was_active) { read_rotation (); was_active = true; }

    const Input & keys      = input ();
    Transform   & transform = get< Transform >();

    if (keys.was_mouse_pressed  (Mouse_Button::right) && window ()) window ()->set_mouse_captured (true );
    if (keys.was_mouse_released (Mouse_Button::right) && window ()) window ()->set_mouse_captured (false);

    if (keys.is_mouse_down (Mouse_Button::right))
    {
        const glm::ivec2 delta = keys.get_mouse_delta ();

        // Moving the mouse right turns right (negative yaw); moving it up looks up.
        yaw   -= float(delta.x) * sensitivity;
        pitch  = std::clamp (pitch - float(delta.y) * sensitivity, -89.f, 89.f);

        apply_rotation ();
    }

    glm::vec3 direction { 0.f };

    if (keys.is_key_down (SDL_SCANCODE_W)) direction += transform.get_forward ();
    if (keys.is_key_down (SDL_SCANCODE_S)) direction -= transform.get_forward ();
    if (keys.is_key_down (SDL_SCANCODE_D)) direction += transform.get_right   ();
    if (keys.is_key_down (SDL_SCANCODE_A)) direction -= transform.get_right   ();
    if (keys.is_key_down (SDL_SCANCODE_E)) direction += glm::vec3(0.f, 1.f, 0.f);
    if (keys.is_key_down (SDL_SCANCODE_Q)) direction -= glm::vec3(0.f, 1.f, 0.f);

    if (glm::length (direction) > 0.f)
    {
        const float boost = keys.is_key_down (SDL_SCANCODE_LSHIFT) ? 3.f : 1.f;

        transform.position += glm::normalize (direction) * speed * boost * delta_time;
    }
}

void Fly_Camera::read_rotation ()
{
    camera_math::yaw_pitch_from_direction (get< Transform >().get_forward (), yaw, pitch);
}

void Fly_Camera::apply_rotation ()
{
    get< Transform >().rotation = camera_math::rotation_from_yaw_pitch (yaw, pitch);
}
