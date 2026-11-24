// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <Engine.hpp>
#include <cmath>

/// Moves the entity up and down around the height it had when created.
class Bob final : public engine::Script
{
    float amplitude;
    float frequency;            // radians per second
    float base_height = 0.f;
    float time        = 0.f;

public:

    explicit Bob (float given_amplitude = 0.15f, float given_frequency = 2.f)
    :
        amplitude(given_amplitude),
        frequency(given_frequency)
    {
    }

    void on_create () override
    {
        base_height = get< engine::Transform >().position.y;
    }

    void on_update (float delta_time) override
    {
        time += delta_time;
        get< engine::Transform >().position.y = base_height + amplitude * std::sin (time * frequency);
    }
};
