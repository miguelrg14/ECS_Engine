// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Game_Rules.hpp"

#include "Bob.hpp"
#include "../Game_Components.hpp"
#include "../Game_Events.hpp"

using namespace engine;

Game_Rules::Game_Rules (Entity given_player, const glm::vec3 & given_start, std::vector< glm::vec3 > given_orb_positions)
:
    player       (given_player),
    start        (given_start ),
    orb_positions(std::move(given_orb_positions))
{
}

void Game_Rules::on_create ()
{
    orb_mesh = resources ()->mesh ("cube");

    orb_material                    = std::make_shared< Material >();
    orb_material->shader            = resources ()->shader ("lit");
    orb_material->color             = { 1.f, 0.82f, 0.25f };
    orb_material->receives_lighting = false;              // glows: always full colour

    subscribe< Trigger_Enter >([this] (const Trigger_Enter & event) { on_trigger (event); });

    start_round ();
}

void Game_Rules::on_update (float delta_time)
{
    Game_State & state = get< Game_State >();

    if (input ().was_key_pressed (SDL_SCANCODE_R)) start_round ();

    state.invulnerable = std::max (state.invulnerable - delta_time, 0.f);

    if (state.round != Round::playing) return;

    state.elapsed += delta_time;

    if (state.elapsed >= state.time_limit) finish (false);
}

void Game_Rules::start_round ()
{
    Registry   & world = registry ();
    Game_State & state = get< Game_State >();

    for (Entity orb : orbs) if (world.valid (orb)) world.destroy (orb);
    orbs.clear ();

    for (const glm::vec3 & position : orb_positions)
    {
        const Entity orb = world.create ();

        world.add< Name          >(orb, "Orb");
        world.add< Orb           >(orb);
        world.add< Mesh_Renderer >(orb, orb_mesh, orb_material);
        world.add< Velocity      >(orb, glm::vec3(0.f), glm::vec3(0.6f, 2.f, 0.3f));

        Transform & transform = world.add< Transform >(orb);
        transform.position = position;
        transform.scale    = glm::vec3(0.35f);

        // Bigger than the visible cube: easier to grab.
        world.add< Box_Collider >(orb, glm::vec3(1.2f), glm::vec3(0.f), true, true);

        world.add< Script_Component >(orb).bind< Bob >(0.2f, 2.5f);

        orbs.push_back (orb);
    }

    state = Game_State{};
    state.total = int(orbs.size ());

    world.get< Transform >(player).position = start;

    log_info ("Game_Rules: new round, %d orbs, %.0f seconds", state.total, double(state.time_limit));
}

void Game_Rules::on_trigger (const Trigger_Enter & event)
{
    Registry   & world = registry ();
    Game_State & state = get< Game_State >();

    if (event.other != player || !world.valid (event.trigger) || state.round != Round::playing) return;

    if (world.has< Orb >(event.trigger))
    {
        destroy (event.trigger);
        ++state.collected;

        publish (Orb_Collected{ state.collected, state.total });
        log_info ("Game_Rules: orb %d / %d", state.collected, state.total);

        if (state.collected == state.total) finish (true);
    }
    else if (world.has< Drone >(event.trigger) && state.invulnerable <= 0.f)
    {
        world.get< Transform >(player).position = start;
        state.invulnerable = 1.5f;

        publish (Player_Caught{ event.trigger });
        log_info ("Game_Rules: caught by a drone, back to the start");
    }
}

void Game_Rules::finish (bool won)
{
    Game_State & state = get< Game_State >();

    state.round = won ? Round::won : Round::lost;

    publish (Round_Over{ won, state.elapsed });
    log_info ("Game_Rules: %s after %.1f s, %d / %d orbs", won ? "won" : "time is up", double(state.elapsed), state.collected, state.total);
}
