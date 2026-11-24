// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <SDL.h>
#include <array>
#include <glm/vec2.hpp>

namespace engine
{

    /// Keys are identified by their physical position (scancode), so WASD stays
    /// in the same place on any keyboard layout. Example: Key::SDL_SCANCODE_W.
    using Key = SDL_Scancode;

    enum class Mouse_Button : Uint8
    {
        left   = SDL_BUTTON_LEFT,
        middle = SDL_BUTTON_MIDDLE,
        right  = SDL_BUTTON_RIGHT,
    };

    /// Keyboard and mouse state for the current frame. Read-only for the game:
    /// Input_System fills it at the start of every frame.
    ///
    /// - is_*_down:      held this frame.
    /// - was_*_pressed:  went down this frame (true for one frame only).
    /// - was_*_released: went up this frame (true for one frame only).
    class Input
    {
        friend class Input_System;

        std::array< bool, SDL_NUM_SCANCODES > keys_current  {};
        std::array< bool, SDL_NUM_SCANCODES > keys_previous {};

        Uint32     mouse_current  = 0;
        Uint32     mouse_previous = 0;
        glm::ivec2 mouse_position { 0, 0 };
        glm::ivec2 mouse_delta    { 0, 0 };
        float      mouse_wheel    = 0.f;

        bool quit_requested = false;

    public:

        bool is_key_down      (Key key) const { return  keys_current[key]; }
        bool was_key_pressed  (Key key) const { return  keys_current[key] && !keys_previous[key]; }
        bool was_key_released (Key key) const { return !keys_current[key] &&  keys_previous[key]; }

        bool is_mouse_down      (Mouse_Button button) const { return  down (mouse_current, button); }
        bool was_mouse_pressed  (Mouse_Button button) const { return  down (mouse_current, button) && !down (mouse_previous, button); }
        bool was_mouse_released (Mouse_Button button) const { return !down (mouse_current, button) &&  down (mouse_previous, button); }

        /// Position in window pixels, origin at the top-left corner.
        glm::ivec2 get_mouse_position () const { return mouse_position; }

        /// Movement since the previous frame, in pixels.
        glm::ivec2 get_mouse_delta    () const { return mouse_delta;    }

        /// Wheel steps this frame: positive away from the user.
        float      get_mouse_wheel    () const { return mouse_wheel;    }

        bool       is_quit_requested  () const { return quit_requested; }

    private:

        static bool down (Uint32 mask, Mouse_Button button)
        {
            return (mask & Uint32(SDL_BUTTON(static_cast< Uint32 >(button)))) != 0;
        }

        /// Turns this frame's state into the previous one before new events arrive.
        void begin_frame ()
        {
            keys_previous  = keys_current;
            mouse_previous = mouse_current;
            mouse_delta    = { 0, 0 };
            mouse_wheel    = 0.f;
        }
    };

}
