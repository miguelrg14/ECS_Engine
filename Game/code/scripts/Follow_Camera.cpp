// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Follow_Camera.hpp"
#include "Camera_Math.hpp"

#include <algorithm>
#include <cmath>

using namespace engine;

void Follow_Camera::on_update (float delta_time)
{
    if (!get< Camera_Component >().active) { snap = true; return; }

    if (!registry ().valid (target) || !registry ().has< Transform >(target)) return;

    const Input & keys = input ();

    if (keys.was_mouse_pressed  (Mouse_Button::right) && window ()) window ()->set_mouse_captured (true );
    if (keys.was_mouse_released (Mouse_Button::right) && window ()) window ()->set_mouse_captured (false);

    if (keys.is_mouse_down (Mouse_Button::right))
    {
        const glm::ivec2 delta = keys.get_mouse_delta ();

        yaw   -= float(delta.x) * sensitivity;
        pitch  = std::clamp (pitch - float(delta.y) * sensitivity, -70.f, 10.f);
    }

    distance = std::clamp (distance - keys.get_mouse_wheel () * 0.8f, 2.5f, 15.f);

    // Aim at the target's chest, not its feet.
    const glm::vec3 focus = registry ().get< Transform >(target).position + glm::vec3(0.f, 1.2f, 0.f);

    // From the focus, the camera sits `distance` units back along the view direction.
    // If that would leave the arena, come closer along the same line instead of
    // moving sideways: the angle stays the same, only the distance shrinks.

    const glm::vec3 back = -camera_math::direction_from_yaw_pitch (yaw, pitch);

    float reach = distance;

    for (int axis : { 0, 2 })
    {
        if (back[axis] >  0.0001f) reach = std::min (reach, ( bounds_half - focus[axis]) / back[axis]);
        if (back[axis] < -0.0001f) reach = std::min (reach, (-bounds_half - focus[axis]) / back[axis]);
    }

    glm::vec3 desired = focus + back * std::max (reach, 1.f);
    desired.y = std::max (desired.y, 0.3f);                  // never below the floor

    Transform & transform = get< Transform >();

    if (snap)
    {
        transform.position = desired;
        snap = false;
    }
    else
    {
        // Frame-rate independent easing: the same feel at 30 or 240 FPS.
        const float blend = 1.f - std::exp (-smoothing * delta_time);
        transform.position += (desired - transform.position) * blend;
    }

    float look_yaw, look_pitch;
    camera_math::yaw_pitch_from_direction (focus - transform.position, look_yaw, look_pitch);
    transform.rotation = camera_math::rotation_from_yaw_pitch (look_yaw, look_pitch);
}
