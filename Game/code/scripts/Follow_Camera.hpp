// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>

/// Third-person camera orbiting a target entity. Only acts while its
/// Camera_Component is active.
///
/// - Right mouse button (hold) + move: orbit around the target.
/// - Mouse wheel: zoom.
///
/// The camera eases towards its ideal position instead of jumping there, so
/// it trails the target slightly and smooths out jumps. It stays inside
/// `bounds_half` (x and z): near a wall it moves closer to the target along its
/// line of sight, so it never ends up behind the wall.
class Follow_Camera final : public engine::Script
{
    engine::Entity target;

    float yaw         =   0.f;     // degrees around the target
    float pitch       = -24.f;     // degrees, negative looks down at the target
    float distance    =   6.5f;
    float sensitivity =   0.15f;
    float smoothing   =  10.f;     // higher follows more tightly
    bool  snap        = true;      // first frame: place it directly
    float bounds_half;

public:

    explicit Follow_Camera (engine::Entity given_target, float given_bounds_half = 1e6f)
    :
        target     (given_target),
        bounds_half(given_bounds_half)
    {
    }

    void on_update (float delta_time) override;
};
