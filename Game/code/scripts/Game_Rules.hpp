// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>
#include <memory>
#include <vector>

/// The round: collect every orb before the time runs out; touching a drone
/// sends the player back to the start. R starts a new round.
///
/// Lives on the "Game" entity next to its Game_State. It learns what happens
/// only through events (Trigger_Enter from the collision system) and announces
/// results with Orb_Collected, Player_Caught and Round_Over.
class Game_Rules final : public engine::Script
{
    engine::Entity player;
    glm::vec3      start;

    std::vector< glm::vec3 >          orb_positions;
    std::shared_ptr< engine::Mesh     > orb_mesh;
    std::shared_ptr< engine::Material > orb_material;

    std::vector< engine::Entity > orbs;

public:

    Game_Rules (engine::Entity given_player, const glm::vec3 & given_start, std::vector< glm::vec3 > given_orb_positions);

    void on_create ()                 override;
    void on_update (float delta_time) override;

private:

    void start_round ();
    void on_trigger  (const engine::Trigger_Enter & event);
    void finish      (bool won);
};
