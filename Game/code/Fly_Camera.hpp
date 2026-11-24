// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>

/// Free-flying debug camera that drives an entity's Transform.
///
/// - Right mouse button (hold): look around; the cursor is captured meanwhile.
/// - W A S D: move, Q E: down and up, Shift: faster.
///
/// Yaw and pitch are kept here and turned into the Transform's rotation, so the
/// camera never rolls. In phase 6 this becomes a script component.
class Fly_Camera
{
    float yaw         = 0.f;      // degrees, 0 looks down -Z
    float pitch       = 0.f;      // degrees, positive looks up
    float speed       = 5.f;      // units per second
    float sensitivity = 0.12f;    // degrees per pixel

public:

    /// Points the camera from `from` towards `target` and places it at `from`.
    void look_at (engine::Transform & transform, const glm::vec3 & from, const glm::vec3 & target);

    void update  (engine::Transform & transform, const engine::Input & input, engine::Window & window, float delta_time);

private:

    void apply_rotation (engine::Transform & transform) const;
};
