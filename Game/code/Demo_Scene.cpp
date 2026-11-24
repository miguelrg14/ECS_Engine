// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Demo_Scene.hpp"

#include "Game_Events.hpp"
#include "scripts/Bob.hpp"
#include "scripts/Fly_Camera.hpp"
#include "scripts/Follow_Camera.hpp"
#include "scripts/Player_Controller.hpp"
#include "scripts/Squash_And_Stretch.hpp"

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

    auto lit        = resources.shader   ("lit");
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

    // Floating, spinning cube with an orbiting child.

    const Entity cube = create_object ("Cube", cube_mesh, checker_material, { 0.f, 1.2f, -1.f });
    registry.add< Velocity         >(cube, glm::vec3(0.f), glm::vec3(0.f, 0.8f, 0.f));
    registry.add< Script_Component >(cube).bind< Bob >(0.15f, 2.f);

    const Entity moon = create_object ("Moon", cube_mesh, checker_material, { 1.3f, 0.f, 0.f }, glm::vec3(0.3f), cube);
    registry.add< Velocity >(moon, glm::vec3(0.f), glm::vec3(1.f, 2.f, 0.f));

    // Light hanging from a spinning pivot.

    light_pivot = registry.create ();
    registry.add< Name      >(light_pivot, "Light pivot");
    registry.add< Transform >(light_pivot).position = { 0.f, 4.f, 0.f };
    registry.add< Velocity  >(light_pivot, glm::vec3(0.f), glm::vec3(0.f, 0.6f, 0.f));

    const Entity light = create_object ("Light", cube_mesh, light_material, { 4.f, 0.f, 0.f }, glm::vec3(0.2f), light_pivot);
    registry.add< Light_Component >(light, glm::vec3(1.f, 0.95f, 0.85f), 1.f);

    // Cameras: the follow camera starts active; Tab switches to the fly camera.

    follow_camera = registry.create ();
    registry.add< Name             >(follow_camera, "Follow camera");
    registry.add< Transform        >(follow_camera);
    registry.add< Camera_Component >(follow_camera);

    fly_camera = registry.create ();
    registry.add< Name             >(fly_camera, "Fly camera");
    registry.add< Transform        >(fly_camera);
    registry.add< Camera_Component >(fly_camera).active = false;
    registry.add< Script_Component >(fly_camera).bind< Fly_Camera >();

    build_player (cube_mesh, lit);

    registry.add< Script_Component >(follow_camera).bind< Follow_Camera >(player);

    // A game-level listener: the scene counts landings without touching the player's code.

    landed_subscription = application.get_events ().subscribe< Landed >([this] (const Landed & event)
    {
        ++landings;
        log_info ("Demo_Scene: landing #%u at %.1f units/s", landings, double(event.impact_speed));
    });

    log_info ("Demo_Scene: %zu entities. WASD = move, Space = jump, Shift = run, right mouse = orbit, wheel = zoom, "
              "Tab = fly camera, C = 1000 cubes, L = light, F = fog, X = wireframe, V = vsync, Esc = quit",
              registry.alive_count ());
}

void Demo_Scene::build_player (std::shared_ptr< Mesh > cube_mesh, std::shared_ptr< Shader > shader)
{
    Registry & registry = application.get_registry ();

    auto body_material = make_material (shader, nullptr, { 0.95f, 0.55f, 0.20f });
    auto head_material = make_material (shader, application.get_resources ().texture ("textures/uv-checker.png"));
    auto eye_material  = make_material (shader, nullptr, { 0.1f, 0.1f, 0.12f });

    // The root sits at the feet and carries the scripts; the visible parts are children,
    // so turning, jumping and squashing the root moves the whole character.

    player = registry.create ();
    registry.add< Name      >(player, "Player");
    registry.add< Transform >(player).position = { 0.f, 0.f, 3.f };

    Script_Component & scripts = registry.add< Script_Component >(player);
    scripts.bind< Player_Controller  >(follow_camera);
    scripts.bind< Squash_And_Stretch >();

    create_object ("Player body", cube_mesh, body_material, { 0.f, 0.55f, 0.f }, { 0.7f, 1.1f, 0.45f }, player);

    const Entity head = create_object ("Player head", cube_mesh, head_material, { 0.f, 1.35f, 0.f }, glm::vec3(0.45f), player);

    // Eyes on the head's -Z face, so they show which way the player faces.
    create_object ("Player eye L", cube_mesh, eye_material, { -0.18f, 0.08f, -0.5f }, { 0.16f, 0.16f, 0.06f }, head);
    create_object ("Player eye R", cube_mesh, eye_material, {  0.18f, 0.08f, -0.5f }, { 0.16f, 0.16f, 0.06f }, head);
}

void Demo_Scene::execute (float delta_time)
{
    handle_input ();

    title_timer += delta_time;

    if (title_timer >= 0.25f)
    {
        title_timer = 0.f;
        update_title ();
    }
}

void Demo_Scene::finalize ()
{
    application.get_events ().unsubscribe (landed_subscription);
    application.get_window ().set_mouse_captured (false);
    Render_Command::set_wireframe (false);
}

Entity Demo_Scene::create_object
(
    const char * object_name,
    std::shared_ptr< Mesh > mesh,
    std::shared_ptr< Material > material,
    const glm::vec3 & position,
    const glm::vec3 & scale,
    Entity parent
)
{
    Registry & registry = application.get_registry ();

    const Entity entity = registry.create ();

    registry.add< Name >(entity, object_name);

    Transform & transform = registry.add< Transform >(entity);
    transform.position = position;
    transform.scale    = scale;
    transform.parent   = parent;

    registry.add< Mesh_Renderer >(entity, std::move(mesh), std::move(material));

    return entity;
}

void Demo_Scene::switch_camera ()
{
    Registry & registry = application.get_registry ();

    Camera_Component & follow = registry.get< Camera_Component >(follow_camera);
    Camera_Component & fly    = registry.get< Camera_Component >(fly_camera);

    if (follow.active)
    {
        // The fly camera starts where the follow camera is, looking the same way.
        registry.get< Transform >(fly_camera) = registry.get< Transform >(follow_camera);
    }

    follow.active = !follow.active;
    fly   .active = !fly.active;

    application.get_window ().set_mouse_captured (false);

    log_info ("Demo_Scene: %s camera", follow.active ? "follow" : "fly");
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

void Demo_Scene::handle_input ()
{
    const Input & input    = application.get_input    ();
    Window      & window   = application.get_window   ();
    Registry    & registry = application.get_registry ();

    if (input.was_key_pressed (SDL_SCANCODE_ESCAPE)) application.quit ();
    if (input.was_key_pressed (SDL_SCANCODE_TAB))    switch_camera ();
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
}

void Demo_Scene::update_title ()
{
    const float fps = application.get_kernel ().get_fps ();

    char title[224];

    std::snprintf
    (
        title, sizeof(title),
        "ECS Engine  |  %.0f FPS  |  %.2f ms  |  %zu entities  |  %u draw calls  |  %s camera  |  %u landings",
        fps,
        fps > 0.f ? 1000.f / fps : 0.f,
        application.get_registry ().alive_count (),
        application.get_renderer ().get_draw_calls (),
        application.get_registry ().get< Camera_Component >(follow_camera).active ? "follow" : "fly",
        landings
    );

    application.get_window ().set_title (title);
}
