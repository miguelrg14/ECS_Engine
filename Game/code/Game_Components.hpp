// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

// Gameplay components. Empty structs work as tags: Game_Rules asks
// registry.has< Orb >(entity) to know what the player touched.

struct Orb   { };
struct Drone { };

enum class Round { playing, won, lost };

/// Score and timer of the round, on the "Game" entity. Game_Rules writes it,
/// the window title reads it.
struct Game_State
{
    int   collected    = 0;
    int   total        = 0;
    float time_limit   = 90.f;
    float elapsed      = 0.f;
    Round round        = Round::playing;
    float invulnerable = 0.f;          // seconds left after being caught
};
