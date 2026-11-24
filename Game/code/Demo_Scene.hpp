// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>
#include <memory>
#include <vector>

/// Builds the demo level and handles the keys that affect the whole program.
/// The game itself is made of scripts and components:
///
/// - Game_Rules          round, score and timer (on the "Game" entity, with Game_State).
/// - Player_Controller   moves and jumps the player, publishes Jumped / Landed.
/// - Squash_And_Stretch  reacts to those events on the player.
/// - Patrol + Bob        drones flying their routes; orbs floating.
/// - Follow_Camera       third-person camera; Fly_Camera free debug camera (Tab switches).
///
/// Walls, crates and platforms are just Box_Colliders: Collision_System keeps
/// the player out of them and reports orbs and drones as Trigger_Enter events.
class Demo_Scene final : public engine::Task
{
    engine::Application & application;

    engine::Entity player        = engine::null_entity;
    engine::Entity follow_camera = engine::null_entity;
    engine::Entity fly_camera    = engine::null_entity;
    engine::Entity light_pivot   = engine::null_entity;
    engine::Entity game          = engine::null_entity;

    std::vector< engine::Entity > crowd;
    std::vector< std::shared_ptr< engine::Material > > crowd_materials;

    engine::Subscription landed_subscription;

    float title_timer = 0.f;

public:

    explicit Demo_Scene (engine::Application & application);

    void initialize ()                 override;
    void execute    (float delta_time) override;
    void finalize   ()                 override;

private:

    engine::Entity create_object
    (
        const char * object_name,
        std::shared_ptr< engine::Mesh > mesh,
        std::shared_ptr< engine::Material > material,
        const glm::vec3 & position,
        const glm::vec3 & scale  = glm::vec3(1.f),
        engine::Entity    parent = engine::null_entity
    );

    /// Visible, solid box: a unit cube scaled to `size` with a static collider.
    engine::Entity create_block
    (
        const char * object_name,
        const glm::vec3 & center,
        const glm::vec3 & size,
        std::shared_ptr< engine::Material > material
    );

    void build_arena   (std::shared_ptr< engine::Shader > shader);
    void build_drone   (const std::vector< glm::vec3 > & route, float speed, std::shared_ptr< engine::Shader > shader);
    void build_player  (std::shared_ptr< engine::Mesh > cube_mesh, std::shared_ptr< engine::Shader > shader);
    void switch_camera ();
    void toggle_crowd  ();
    void handle_input  ();
    void update_title  ();
    void draw_hud      ();
    void add_debug_ui  ();
};
