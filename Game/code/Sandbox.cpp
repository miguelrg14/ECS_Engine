// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Sandbox.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace
{

    /// Hue, saturation and value in 0..1 to an RGB colour.
    glm::vec3 hsv_to_rgb (float h, float s, float v)
    {
        auto channel = [h, s, v] (float n)
        {
            const float k = std::fmod (n + h * 6.f, 6.f);
            return v - v * s * std::clamp (std::min (k, 4.f - k), 0.f, 1.f);
        };

        return { channel (5.f), channel (3.f), channel (1.f) };
    }

}

Sandbox::Sandbox (engine::Application & given_application)
:
    Task       ("Sandbox", engine::Priority::scripts),
    application(given_application)
{
}

void Sandbox::initialize ()
{
    engine::log_info ("Sandbox: Space = faster, left click = pick colour, wheel = speed, V = vsync, R = reset, Esc = quit");
}

void Sandbox::execute (float delta_time)
{
    const engine::Input & input = application.get_input ();

    if (input.was_key_pressed (SDL_SCANCODE_ESCAPE))
    {
        application.quit ();
    }

    if (input.was_key_pressed (SDL_SCANCODE_V))
    {
        engine::Window & window = application.get_window ();
        window.set_vsync (!window.is_vsync ());
    }

    if (input.was_key_pressed (SDL_SCANCODE_R))
    {
        hue   = 0.6f;
        speed = 0.05f;
    }

    speed = std::clamp (speed + input.get_mouse_wheel () * 0.02f, 0.f, 2.f);

    const float multiplier = input.is_key_down (SDL_SCANCODE_SPACE) ? 6.f : 1.f;

    hue = std::fmod (hue + speed * multiplier * delta_time, 1.f);

    float saturation = 0.45f;
    float value      = 0.22f;

    if (input.is_mouse_down (engine::Mouse_Button::left))
    {
        const engine::Window & window   = application.get_window ();
        const glm::ivec2       position = input.get_mouse_position ();

        // Window size is in drawable pixels; the mouse in window points. Close enough here.
        hue        = std::clamp (float(position.x) / float(std::max (window.get_width (),  1u)), 0.f, 1.f);
        saturation = std::clamp (1.f - float(position.y) / float(std::max (window.get_height (), 1u)), 0.f, 1.f);
        value      = 0.6f;
    }

    engine::Render_Command::set_clear_color (glm::vec4 (hsv_to_rgb (hue, saturation, value), 1.f));
    engine::Render_Command::clear ();

    title_timer += delta_time;

    if (title_timer >= 0.25f)
    {
        title_timer = 0.f;
        update_title ();
    }
}

void Sandbox::finalize ()
{
    engine::log_info ("Sandbox: finalized");
}

void Sandbox::update_title ()
{
    const engine::Kernel & kernel = application.get_kernel ();
    engine::Window       & window = application.get_window ();

    const float fps = kernel.get_fps ();

    char title[128];

    std::snprintf
    (
        title, sizeof(title),
        "ECS Engine  |  %.0f FPS  |  %.2f ms  |  vsync %s",
        fps,
        fps > 0.f ? 1000.f / fps : 0.f,
        window.is_vsync () ? "on" : "off"
    );

    window.set_title (title);
}
