// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "System.hpp"

namespace engine
{

    /// Applies Velocity to Transform: position += linear * dt and a rotation of
    /// angular * dt around the entity's own axes.
    class Movement_System final : public System
    {
    public:

        explicit Movement_System (Registry & registry);

        void execute (float delta_time) override;
    };

}
