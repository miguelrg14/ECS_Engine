// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>

// Gameplay events. Any struct can be an event: there is no base class to inherit.

struct Jumped
{
    engine::Entity entity;
};

struct Landed
{
    engine::Entity entity;
    float          impact_speed;       // vertical speed when touching the ground, units/s
};

struct Orb_Collected
{
    int collected;
    int total;
};

struct Player_Caught
{
    engine::Entity drone;
};

struct Round_Over
{
    bool  won;
    float seconds;                     // time played
};
