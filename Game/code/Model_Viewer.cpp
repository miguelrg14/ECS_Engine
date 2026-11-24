// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Model_Viewer.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstdio>

Model_Viewer::Model_Viewer (engine::Application & given_application)
:
    Task       ("Model_Viewer", engine::Priority::scripts),
    application(given_application)
{
}

void Model_Viewer::initialize ()
{
    engine::Resource_Manager & resources = application.get_resources ();

    auto lit = resources.shader ("lit");

    ground_mesh   = resources.add_mesh ("ground", engine::Mesh::create_plane (60.f));
    cube_mesh     = resources.add_mesh ("cube",   engine::Mesh::create_cube  ());
    umbrella_mesh = resources.mesh ("models/Umbrella.obj");
    plant_mesh    = resources.mesh ("models/Plant.obj");

    ground_material.shader     = lit;
    ground_material.albedo     = resources.texture ("textures/wood.png");
    ground_material.uv_tiling  = glm::vec2(15.f);
    ground_material.specular   = 0.1f;

    checker_material.shader    = lit;
    checker_material.albedo    = resources.texture ("textures/uv-checker.png");
    checker_material.specular  = 0.6f;
    checker_material.shininess = 64.f;

    umbrella_material.shader   = lit;
    umbrella_material.albedo   = resources.texture ("textures/umbrella.png");

    plant_material.shader      = lit;
    plant_material.albedo      = resources.texture ("textures/plant.png");
    plant_material.specular    = 0.15f;

    light_material.shader             = lit;
    light_material.color              = { 1.f, 0.9f, 0.6f };
    light_material.receives_lighting  = false;

    light.color = { 1.f, 0.95f, 0.85f };

    camera.set_perspective (60.f, 0.1f, 200.f);
    camera.set_position    ({ 0.f, 2.2f, 7.5f });
    camera.look_at         ({ 0.f, 1.2f, 0.f });

    engine::Render_Command::set_clear_color ({ fog.color, 1.f });

    engine::log_info ("Model_Viewer: right mouse + WASD/QE = fly, Shift = faster, L = light, F = fog, X = wireframe, V = vsync, Esc = quit");
}

void Model_Viewer::execute (float delta_time)
{
    time += delta_time;

    handle_input (delta_time);

    if (light_orbits) light_angle += delta_time * 0.6f;

    light.position = { std::cos (light_angle) * 4.f, 4.f, std::sin (light_angle) * 4.f };

    draw_scene ();

    title_timer += delta_time;

    if (title_timer >= 0.25f)
    {
        title_timer = 0.f;
        update_title ();
    }
}

void Model_Viewer::finalize ()
{
    application.get_window ().set_mouse_captured (false);
    engine::Render_Command::set_wireframe (false);
}

void Model_Viewer::handle_input (float delta_time)
{
    const engine::Input & input  = application.get_input  ();
    engine::Window      & window = application.get_window ();

    if (input.was_key_pressed (SDL_SCANCODE_ESCAPE)) application.quit ();
    if (input.was_key_pressed (SDL_SCANCODE_L))      light_orbits = !light_orbits;
    if (input.was_key_pressed (SDL_SCANCODE_F))      fog.enabled  = !fog.enabled;
    if (input.was_key_pressed (SDL_SCANCODE_V))      window.set_vsync (!window.is_vsync ());

    if (input.was_key_pressed (SDL_SCANCODE_X))
    {
        wireframe = !wireframe;
        engine::Render_Command::set_wireframe (wireframe);
    }

    camera_controller.update (camera, input, window, delta_time);
}

void Model_Viewer::draw_scene ()
{
    engine::Renderer & renderer = application.get_renderer ();

    camera.set_aspect (application.get_window ().get_aspect ());

    engine::Render_Command::clear ();

    renderer.begin_frame (camera, light, fog);

    const glm::mat4 identity { 1.f };

    renderer.submit (*ground_mesh, ground_material, identity);

    // Models from VS_3D-Renderer. The plant is a tree about 8.6 units tall.

    renderer.submit
    (
        *umbrella_mesh, umbrella_material,
        glm::scale (glm::translate (identity, { -2.5f, 0.f, 0.5f }), glm::vec3(1.4f))
    );

    renderer.submit
    (
        *plant_mesh, plant_material,
        glm::scale (glm::translate (identity, { 3.f, 0.f, -1.5f }), glm::vec3(0.4f))
    );

    // Spinning cube with an orbiting child: the child's matrix is the parent's
    // times its own local transform, the same rule Transform_System will use.

    const glm::mat4 parent = glm::rotate
    (
        glm::translate (identity, { 0.f, 1.2f + 0.15f * std::sin (time * 2.f), 0.f }),
        time * 0.8f, { 0.f, 1.f, 0.f }
    );

    renderer.submit (*cube_mesh, checker_material, parent);

    const glm::mat4 child = parent
                          * glm::rotate    (identity, time * 2.f, { 0.f, 1.f, 0.f })
                          * glm::translate (identity, { 1.3f, 0.f, 0.f })
                          * glm::scale     (identity, glm::vec3(0.3f));

    renderer.submit (*cube_mesh, checker_material, child);

    // The light, drawn as a small unlit cube.

    renderer.submit (*cube_mesh, light_material, glm::scale (glm::translate (identity, light.position), glm::vec3(0.2f)));
}

void Model_Viewer::update_title ()
{
    const engine::Kernel   & kernel   = application.get_kernel   ();
    const engine::Renderer & renderer = application.get_renderer ();

    const float fps = kernel.get_fps ();

    char title[160];

    std::snprintf
    (
        title, sizeof(title),
        "ECS Engine  |  %.0f FPS  |  %.2f ms  |  %u draw calls  |  %u triangles",
        fps,
        fps > 0.f ? 1000.f / fps : 0.f,
        renderer.get_draw_calls (),
        renderer.get_triangles  ()
    );

    application.get_window ().set_title (title);
}
