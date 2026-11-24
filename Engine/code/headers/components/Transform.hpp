// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "ecs/Entity.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine
{

    /// Position, rotation and scale relative to the parent (or to the world when
    /// there is no parent). Transform_System writes `world` every frame:
    /// world = parent's world * local. Read `world`, write the other fields.
    struct Transform
    {
        glm::vec3 position { 0.f };
        glm::quat rotation { 1.f, 0.f, 0.f, 0.f };          // identity (w, x, y, z)
        glm::vec3 scale    { 1.f };

        Entity    parent   = null_entity;

        glm::mat4 world    { 1.f };                        // written by Transform_System

    public:

        glm::mat4 get_local_matrix () const;

        /// Rotation from Euler angles in degrees, applied as yaw (Y), then pitch (X), then roll (Z).
        void      set_euler_degrees (const glm::vec3 & pitch_yaw_roll);

        /// Directions in the parent's space. The model faces -Z, like the camera.
        glm::vec3 get_forward () const { return rotation * glm::vec3(0.f, 0.f, -1.f); }
        glm::vec3 get_right   () const { return rotation * glm::vec3(1.f, 0.f,  0.f); }
        glm::vec3 get_up      () const { return rotation * glm::vec3(0.f, 1.f,  0.f); }

        /// Position in the world, from the last Transform_System update.
        glm::vec3 get_world_position () const { return glm::vec3(world[3]); }
    };

}
