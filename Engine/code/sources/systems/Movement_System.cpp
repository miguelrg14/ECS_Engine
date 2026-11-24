// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "systems/Movement_System.hpp"

#include "components/Transform.hpp"
#include "components/Velocity.hpp"

#include <glm/geometric.hpp>

namespace engine
{

    Movement_System::Movement_System (Registry & given_registry)
    :
        System ("Movement_System", Priority::movement, given_registry)
    {
    }

    void Movement_System::execute (float delta_time)
    {
        registry.view< Transform, Velocity >().each ([delta_time] (Transform & transform, Velocity & velocity)
        {
            transform.position += velocity.linear * delta_time;

            const float speed = glm::length (velocity.angular);

            if (speed > 0.f)
            {
                // Multiplying on the right rotates around the entity's local axes.
                transform.rotation = glm::normalize
                (
                    transform.rotation * glm::angleAxis (speed * delta_time, velocity.angular / speed)
                );
            }
        });
    }

}
