// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Camera.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace engine
{

    struct Point_Light
    {
        glm::vec3 position  { 0.f, 5.f, 0.f };       // world space
        glm::vec3 color     { 1.f };
    };

    struct Fog
    {
        bool      enabled   = true;
        float     start     = 25.f;                  // distance from the camera
        float     end       = 60.f;
        glm::vec3 color     { 0.75f, 0.80f, 0.88f };
    };

    /// Draws meshes with materials. It knows nothing about entities: a frame is
    /// begin_frame (camera and lights) followed by one submit per object.
    ///
    /// Per-frame uniforms (projection, light, fog) are sent once per shader per
    /// frame; per-object uniforms (matrices, material) on every submit.
    class Renderer
    {
        glm::mat4   view       { 1.f };
        glm::mat4   projection { 1.f };
        Point_Light light;
        Fog         fog;

        std::shared_ptr< Texture > white_texture;
        std::vector< const Shader * > prepared_shaders;     // shaders that got this frame's uniforms

        unsigned draw_calls = 0;
        unsigned triangles  = 0;

    public:

        Renderer ();

        void begin_frame (const Camera & camera, const Point_Light & light, const Fog & fog);

        void submit (const Mesh & mesh, const Material & material, const glm::mat4 & model_matrix);

    public:

        unsigned get_draw_calls () const { return draw_calls; }
        unsigned get_triangles  () const { return triangles;  }

    private:

        void prepare_shader (const Shader & shader);
    };

}
