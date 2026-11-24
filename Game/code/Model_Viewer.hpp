// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Fly_Camera.hpp"

#include <Engine.hpp>
#include <memory>

/// Phase 3 test scene for the renderer module, still without entities:
/// textured .obj models, a spinning cube with an orbiting child, a ground plane,
/// an orbiting point light, Blinn-Phong shading and distance fog.
///
/// Controls: right mouse + WASD/QE to fly, L pause light, F fog, X wireframe,
/// V vsync, Esc quit. From phase 5 on this becomes entities drawn by Render_System.
class Model_Viewer final : public engine::Task
{
    engine::Application & application;

    engine::Camera      camera;
    Fly_Camera          camera_controller;
    engine::Point_Light light;
    engine::Fog         fog;

    std::shared_ptr< engine::Mesh > ground_mesh;
    std::shared_ptr< engine::Mesh > cube_mesh;
    std::shared_ptr< engine::Mesh > umbrella_mesh;
    std::shared_ptr< engine::Mesh > plant_mesh;

    engine::Material ground_material;
    engine::Material checker_material;
    engine::Material umbrella_material;
    engine::Material plant_material;
    engine::Material light_material;

    float time         = 0.f;
    float light_angle  = 0.f;
    bool  light_orbits = true;
    bool  wireframe    = false;
    float title_timer  = 0.f;

public:

    explicit Model_Viewer (engine::Application & application);

    void initialize ()                 override;
    void execute    (float delta_time) override;
    void finalize   ()                 override;

private:

    void handle_input (float delta_time);
    void draw_scene   ();
    void update_title ();
};
