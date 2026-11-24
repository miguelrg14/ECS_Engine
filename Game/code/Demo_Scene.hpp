// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Fly_Camera.hpp"

#include <Engine.hpp>
#include <memory>
#include <vector>

/// Phase 5 test scene, now made of entities. This task only builds the scene
/// and reacts to input; moving, parenting and drawing are done by the engine's
/// systems from the components:
///
/// - the cube spins (Velocity) and carries a smaller cube as a child (Transform::parent),
/// - the light is the child of a spinning pivot, so it orbits without any code,
/// - C adds or removes a field of 1,000 spinning cubes.
///
/// Controls: right mouse + WASD/QE to fly, C crowd, L pause light, F fog,
/// X wireframe, V vsync, Esc quit. In phase 6 the logic here moves into scripts.
class Demo_Scene final : public engine::Task
{
    engine::Application & application;

    Fly_Camera     camera_controller;
    engine::Entity camera      = engine::null_entity;
    engine::Entity cube        = engine::null_entity;
    engine::Entity light_pivot = engine::null_entity;

    std::vector< engine::Entity > crowd;
    std::vector< std::shared_ptr< engine::Material > > crowd_materials;

    float time        = 0.f;
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
        const glm::vec3 & scale = glm::vec3(1.f)
    );

    void toggle_crowd ();
    void handle_input (float delta_time);
    void update_title ();
};
