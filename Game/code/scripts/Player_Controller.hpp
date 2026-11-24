// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>

/// Moves the player relative to where the camera looks, turns it towards its
/// movement and makes it jump. Reads input only while its camera is active.
///
/// - W A S D: move, Shift: run, Space: jump.
///
/// Walls and platforms are handled by Collision_System: this script moves
/// freely and reads Box_Collider::last_push to learn whether something held
/// it up (it is standing) or pushed it down (it hit its head).
///
/// Publishes Jumped and Landed; it does not know who listens.
class Player_Controller final : public engine::Script
{
    engine::Entity camera;

    float walk_speed        =  4.f;
    float run_speed         =  7.5f;
    float jump_speed        =  8.f;
    float gravity           = 22.f;
    float turn_speed        = 14.f;

    float vertical_velocity = 0.f;
    bool  grounded          = true;

public:

    explicit Player_Controller (engine::Entity given_camera) : camera(given_camera) {}

    void on_update (float delta_time) override;

private:

    glm::vec3 read_direction () const;
};
