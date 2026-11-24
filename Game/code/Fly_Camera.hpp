// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>

/// Free-flying debug camera.
///
/// - Right mouse button (hold): look around; the cursor is captured meanwhile.
/// - W A S D: move, Q E: down and up, Shift: faster.
class Fly_Camera
{
    float speed       = 5.f;      // units per second
    float sensitivity = 0.12f;    // degrees per pixel

public:

    void update (engine::Camera & camera, const engine::Input & input, engine::Window & window, float delta_time);
};
