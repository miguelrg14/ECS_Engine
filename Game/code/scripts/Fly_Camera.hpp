// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>

/// Free-flying debug camera. Only acts while its Camera_Component is active.
///
/// - Right mouse button (hold): look around; the cursor is captured meanwhile.
/// - W A S D: move, Q E: down and up, Shift: faster.
///
/// Yaw and pitch are kept here and turned into the Transform's rotation, so the
/// camera never rolls. When it becomes active it starts from its current rotation.
class Fly_Camera final : public engine::Script
{
    float yaw         = 0.f;      // degrees, 0 looks down -Z
    float pitch       = 0.f;      // degrees, positive looks up
    float speed       = 5.f;      // units per second
    float sensitivity = 0.12f;    // degrees per pixel
    bool  was_active  = false;

public:

    void on_update (float delta_time) override;

private:

    void read_rotation  ();
    void apply_rotation ();
};
