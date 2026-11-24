// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>

/// Phase 2 test task: exercises the kernel, the timer and the input.
///
/// - The background colour cycles over time; Space speeds it up while held.
/// - Holding the left mouse button picks the colour from the cursor position.
/// - The mouse wheel changes the cycling speed.
/// - V toggles vsync, R resets, Esc quits.
/// - The window title shows FPS, frame time and the vsync state.
///
/// Clearing the screen moves to Render_System in phase 5.
class Sandbox final : public engine::Task
{
    engine::Application & application;

    float hue          = 0.6f;     // 0..1 around the colour wheel
    float speed        = 0.05f;    // turns per second
    float title_timer  = 0.f;

public:

    explicit Sandbox (engine::Application & application);

    void initialize ()                 override;
    void execute    (float delta_time) override;
    void finalize   ()                 override;

private:

    void update_title ();
};
