// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>
#include <memory>
#include <vector>

/// Phase 6 test scene. This task builds the scene and handles the keys that
/// affect the whole game; the per-entity logic now lives in scripts:
///
/// - Player_Controller   moves and jumps the player, publishes Jumped / Landed.
/// - Squash_And_Stretch  reacts to those events on the player.
/// - Follow_Camera       third-person camera; Fly_Camera free debug camera (Tab switches).
/// - Bob                 makes the checker cube float.
///
/// Spinning, parenting and drawing are still done by the engine's systems from components.
class Demo_Scene final : public engine::Task
{
    engine::Application & application;

    engine::Entity player        = engine::null_entity;
    engine::Entity follow_camera = engine::null_entity;
    engine::Entity fly_camera    = engine::null_entity;
    engine::Entity light_pivot   = engine::null_entity;

    std::vector< engine::Entity > crowd;
    std::vector< std::shared_ptr< engine::Material > > crowd_materials;

    engine::Subscription landed_subscription;
    unsigned             landings    = 0;

    bool  wireframe   = false;
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

    void build_player  (std::shared_ptr< engine::Mesh > cube_mesh, std::shared_ptr< engine::Shader > shader);
    void switch_camera ();
    void toggle_crowd  ();
    void handle_input  ();
    void update_title  ();
};
