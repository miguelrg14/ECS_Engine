// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Fly_Camera.hpp"

void Fly_Camera::update (engine::Camera & camera, const engine::Input & input, engine::Window & window, float delta_time)
{
    using engine::Mouse_Button;

    if (input.was_mouse_pressed  (Mouse_Button::right)) window.set_mouse_captured (true );
    if (input.was_mouse_released (Mouse_Button::right)) window.set_mouse_captured (false);

    if (input.is_mouse_down (Mouse_Button::right))
    {
        const glm::ivec2 delta = input.get_mouse_delta ();

        // Moving the mouse up (negative y) looks up.
        camera.rotate (float(delta.x) * sensitivity, float(-delta.y) * sensitivity);
    }

    glm::vec3 direction { 0.f };

    if (input.is_key_down (SDL_SCANCODE_W)) direction += camera.get_forward ();
    if (input.is_key_down (SDL_SCANCODE_S)) direction -= camera.get_forward ();
    if (input.is_key_down (SDL_SCANCODE_D)) direction += camera.get_right   ();
    if (input.is_key_down (SDL_SCANCODE_A)) direction -= camera.get_right   ();
    if (input.is_key_down (SDL_SCANCODE_E)) direction += glm::vec3(0.f, 1.f, 0.f);
    if (input.is_key_down (SDL_SCANCODE_Q)) direction -= glm::vec3(0.f, 1.f, 0.f);

    if (glm::length (direction) > 0.f)
    {
        const float boost = input.is_key_down (SDL_SCANCODE_LSHIFT) ? 3.f : 1.f;

        camera.move (glm::normalize (direction) * speed * boost * delta_time);
    }
}
