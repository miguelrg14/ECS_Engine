// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Shader.hpp"
#include "Texture.hpp"

#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace engine
{

    /// How a surface looks: the shader that draws it, its texture and the
    /// Blinn-Phong parameters. Many meshes can share one material.
    struct Material
    {
        std::shared_ptr< Shader  > shader;
        std::shared_ptr< Texture > albedo;                 // white when null

        glm::vec3 color             { 1.f };              // multiplies the texture
        glm::vec2 uv_tiling         { 1.f };              // repeats the texture

        float     ambient           = 0.25f;
        float     diffuse           = 0.85f;
        float     specular          = 0.35f;
        float     shininess         = 32.f;

        bool      receives_lighting = true;               // false: flat colour (light markers, UI)
    };

}
