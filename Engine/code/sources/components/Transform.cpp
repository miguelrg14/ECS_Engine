// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "components/Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

namespace engine
{

    glm::mat4 Transform::get_local_matrix () const
    {
        // Scale first, then rotate, then move: T * R * S.
        glm::mat4 matrix = glm::translate (glm::mat4(1.f), position);
        matrix          *= glm::mat4_cast (rotation);
        matrix           = glm::scale     (matrix, scale);
        return matrix;
    }

    void Transform::set_euler_degrees (const glm::vec3 & pitch_yaw_roll)
    {
        const glm::vec3 radians = glm::radians (pitch_yaw_roll);

        rotation = glm::angleAxis (radians.y, glm::vec3(0.f, 1.f, 0.f))
                 * glm::angleAxis (radians.x, glm::vec3(1.f, 0.f, 0.f))
                 * glm::angleAxis (radians.z, glm::vec3(0.f, 0.f, 1.f));
    }

}
