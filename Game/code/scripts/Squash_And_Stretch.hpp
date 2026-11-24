// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "../Game_Events.hpp"

#include <Engine.hpp>
#include <algorithm>
#include <cmath>

/// Stretches the entity when it jumps and squashes it when it lands, harder the
/// faster it falls. It only listens to events: it knows nothing about how the
/// entity moves, so it works with any script that publishes Jumped / Landed.
class Squash_And_Stretch final : public engine::Script
{
    float amount = 0.f;            // > 0 squashed, < 0 stretched

public:

    void on_create () override
    {
        subscribe< Jumped >([this] (const Jumped & event)
        {
            if (event.entity == entity ()) amount = -0.18f;
        });

        subscribe< Landed >([this] (const Landed & event)
        {
            if (event.entity == entity ()) amount = std::min (0.05f + event.impact_speed * 0.03f, 0.45f);
        });
    }

    void on_update (float delta_time) override
    {
        amount *= std::exp (-9.f * delta_time);              // eases back to the normal shape

        get< engine::Transform >().scale = { 1.f + amount * 0.5f, 1.f - amount, 1.f + amount * 0.5f };
    }
};
