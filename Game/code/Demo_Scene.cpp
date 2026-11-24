// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Demo_Scene.hpp"

#include <cmath>
#include <cstdio>
#include <random>

using namespace engine;

namespace
{

    std::shared_ptr< Material > make_material
    (
        std::shared_ptr< Shader  > shader,
        std::shared_ptr< Texture > albedo,
        const glm::vec3          & color = glm::vec3(1.f)
    )
    {
        auto material    = std::make_shared< Material >();
        material->shader = std::move(shader);
        material->albedo = std::move(albedo);
        material->color  = color;
        return material;
    }

}

Demo_Scene::Demo_Scene (Application & given_application)
:
    Task       ("Demo_Scene", Priority::scripts),
    application(given_application)
{
}

void Demo_Scene::initialize ()
{
    Resource_Manager & resources = application.get_resources ();
    Registry         & registry  = application.get_registry  ();

    auto lit   = resources.shader   ("lit");
    auto cube_mesh  = resources.add_mesh ("cube",   Mesh::create_cube  ());
    auto plane_mesh = resources.add_mesh ("ground", Mesh::create_plane (60.f));

    auto ground_material    = make_material (lit, resources.texture ("textures/wood.png"));
    ground_material->uv_tiling = glm::vec2(15.f);
    ground_material->specular  = 0.1f;

    auto checker_material   = make_material (lit, resources.texture ("textures/uv-checker.png"));
    checker_material->specular  = 0.6f;
    checker_material->shininess = 64.f;

    auto umbrella_material  = make_material (lit, resources.texture ("textures/umbrella.png"));
    auto plant_material     = make_material (lit, resources.texture ("textures/plant.png"));
    plant_material->specular = 0.15f;

    auto light_material     = make_material (lit, nullptr, { 1.f, 0.9f, 0.6f });
    light_material->receives_lighting = false;

    // Six shared materials for the crowd: a thousand cubes, six materials.
    const glm::vec3 palette[] =
    {
        { 0.90f, 0.35f, 0.30f }, { 0.95f, 0.70f, 0.25f }, { 0.40f, 0.75f, 0.40f },
        { 0.30f, 0.60f, 0.90f }, { 0.65f, 0.45f, 0.85f }, { 0.85f, 0.85f, 0.85f },
    };

    for (const glm::vec3 & color : palette) crowd_materials.push_back (make_material (lit, nullptr, color));

    // Static props.

    create_object ("Ground",   plane_mesh,                             ground_material,   { 0.f, 0.f, 0.f });
    create_object ("Umbrella", resources.mesh ("models/Umbrella.obj"), umbrella_material, { -2.5f, 0.f, 0.5f }, glm::vec3(1.4f));
    create_object ("Plant",    resources.mesh ("models/Plant.obj"),    plant_material,    { 3.f, 0.f, -1.5f },  glm::vec3(0.4f));

    // Spinning cube with an orbiting child: the child follows because its
    // world matrix is the parent's world matrix times its own local one.

    cube = create_object ("Cube", cube_mesh, checker_material, { 0.f, 1.2f, 0.f });
    registry.add< Velocity >(cube, glm::vec3(0.f), glm::vec3(0.f, 0.8f, 0.f));

    const Entity moon = create_object ("Moon", cube_mesh, checker_material, { 1.3f, 0.f, 0.f }, glm::vec3(0.3f));
    registry.get< Transform >(moon).parent = cube;
    registry.add< Velocity >(moon, glm::vec3(0.f), glm::vec3(1.f, 2.f, 0.f));

    // The light hangs from a spinning pivot, so it orbits the scene.

    light_pivot = registry.create ();
    registry.add< Name      >(light_pivot, "Light pivot");
    registry.add< Transform >(light_pivot).position = { 0.f, 4.f, 0.f };
    registry.add< Velocity  >(light_pivot, glm::vec3(0.f), glm::vec3(0.f, 0.6f, 0.f));

    const Entity light = create_object ("Light", cube_mesh, light_material, { 4.f, 0.f, 0.f }, glm::vec3(0.2f));
    registry.get< Transform >(light).parent = light_pivot;
    registry.add< Light_Component >(light, glm::vec3(1.f, 0.95f, 0.85f), 1.f);

    // Camera.

    camera = registry.create ();
    registry.add< Name             >(camera, "Camera");
    registry.add< Transform        >(camera);
    registry.add< Camera_Component >(camera);

    camera_controller.look_at (registry.get< Transform >(camera), { 0.f, 2.2f, 7.5f }, { 0.f, 1.2f, 0.f });

    log_info ("Demo_Scene: %zu entities. Right mouse + WASD/QE = fly, C = 1000 cubes, L = light, F = fog, X = wireframe, V = vsync, Esc = quit",
              registry.alive_count ());
}

void Demo_Scene::execute (float delta_time)
{
    time += delta_time;

    handle_input (delta_time);

    // Gameplay-style logic still lives here until scripts exist (phase 6).
    application.get_registry ().get< Transform >(cube).position.y = 1.2f + 0.15f * std::sin (time * 2.f);

    title_timer += delta_time;

    if (title_timer >= 0.25f)
    {
        title_timer = 0.f;
        update_title ();
    }
}

void Demo_Scene::finalize ()
{
    application.get_window ().set_mouse_captured (false);
    Render_Command::set_wireframe (false);
}

Entity Demo_Scene::create_object
(
    const char * object_name,
    std::shared_ptr< Mesh > mesh,
    std::shared_ptr< Material > material,
    const glm::vec3 & position,
    const glm::vec3 & scale
)
{
    Registry & registry = application.get_registry ();

    const Entity entity = registry.create ();

    registry.add< Name >(entity, object_name);

    Transform & transform = registry.add< Transform >(entity);
    transform.position = position;
    transform.scale    = scale;

    registry.add< Mesh_Renderer >(entity, std::move(mesh), std::move(material));

    return entity;
}

void Demo_Scene::toggle_crowd ()
{
    Registry & registry = application.get_registry ();

    if (!crowd.empty ())
    {
        for (Entity entity : crowd) registry.destroy (entity);

        log_info ("Demo_Scene: removed %zu cubes, %zu entities left", crowd.size (), registry.alive_count ());
        crowd.clear ();
        return;
    }

    auto cube_mesh = application.get_resources ().mesh ("cube");

    std::mt19937 random (1234);         // fixed seed: the same field every time
    std::uniform_real_distribution< float > spin (-2.f, 2.f);

    // 40 x 25 grid behind the props.

    for (int row = 0; row < 25; ++row)
    {
        for (int column = 0; column < 40; ++column)
        {
            const Entity entity = create_object
            (
                "Crowd cube", cube_mesh,
                crowd_materials[std::size_t(row * 40 + column) % crowd_materials.size ()],
                { -19.5f + float(column), 0.6f, -6.f - float(row) },
                glm::vec3(0.5f)
            );

            registry.add< Velocity >(entity, glm::vec3(0.f), glm::vec3(spin (random), spin (random), spin (random)));

            crowd.push_back (entity);
        }
    }

    log_info ("Demo_Scene: added %zu cubes, %zu entities", crowd.size (), registry.alive_count ());
}

void Demo_Scene::handle_input (float delta_time)
{
    const Input & input    = application.get_input    ();
    Window      & window   = application.get_window   ();
    Registry    & registry = application.get_registry ();

    if (input.was_key_pressed (SDL_SCANCODE_ESCAPE)) application.quit ();
    if (input.was_key_pressed (SDL_SCANCODE_C))      toggle_crowd ();
    if (input.was_key_pressed (SDL_SCANCODE_V))      window.set_vsync (!window.is_vsync ());

    if (input.was_key_pressed (SDL_SCANCODE_F))
    {
        Fog & fog = application.get_render_system ().get_fog ();
        fog.enabled = !fog.enabled;
    }

    if (input.was_key_pressed (SDL_SCANCODE_L))
    {
        // Pausing the light is just removing its pivot's Velocity (and adding it back).
        if (registry.has< Velocity >(light_pivot)) registry.remove< Velocity >(light_pivot);
        else                                       registry.add< Velocity >(light_pivot, glm::vec3(0.f), glm::vec3(0.f, 0.6f, 0.f));
    }

    if (input.was_key_pressed (SDL_SCANCODE_X))
    {
        wireframe = !wireframe;
        Render_Command::set_wireframe (wireframe);
    }

    camera_controller.update (registry.get< Transform >(camera), input, window, delta_time);
}

void Demo_Scene::update_title ()
{
    const float fps = application.get_kernel ().get_fps ();

    char title[192];

    std::snprintf
    (
        title, sizeof(title),
        "ECS Engine  |  %.0f FPS  |  %.2f ms  |  %zu entities  |  %u draw calls  |  %u triangles",
        fps,
        fps > 0.f ? 1000.f / fps : 0.f,
        application.get_registry ().alive_count (),
        application.get_renderer ().get_draw_calls (),
        application.get_renderer ().get_triangles  ()
    );

    application.get_window ().set_title (title);
}
