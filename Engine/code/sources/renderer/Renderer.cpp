// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "renderer/Renderer.hpp"
#include "renderer/Render_Command.hpp"

#include <glm/matrix.hpp>
#include <algorithm>

namespace engine
{

    Renderer::Renderer ()
    :
        white_texture(Texture::from_color ({ 1.f, 1.f, 1.f, 1.f }))
    {
        Render_Command::set_depth_test (true);
    }

    void Renderer::begin_frame (const Camera & camera, const Point_Light & frame_light, const Fog & frame_fog)
    {
        begin_frame (camera.get_view (), camera.get_projection (), frame_light, frame_fog);
    }

    void Renderer::begin_frame (const glm::mat4 & frame_view, const glm::mat4 & frame_projection, const Point_Light & frame_light, const Fog & frame_fog)
    {
        view       = frame_view;
        projection = frame_projection;
        light      = frame_light;
        fog        = frame_fog;

        prepared_shaders.clear ();
        draw_calls = 0;
        triangles  = 0;
    }

    void Renderer::prepare_shader (const Shader & shader)
    {
        shader.bind ();

        if (std::ranges::find (prepared_shaders, &shader) != prepared_shaders.end ()) return;

        // Lighting is computed in eye space, so the light is moved there once per frame.

        const glm::vec3 light_view_position = glm::vec3(view * glm::vec4(light.position, 1.f));

        shader.set ("projection_matrix", projection);
        shader.set ("light.position",    light_view_position);
        shader.set ("light.color",       light.color);
        shader.set ("fog_enabled",       fog.enabled ? 1 : 0);
        shader.set ("fog_start",         fog.start);
        shader.set ("fog_end",           fog.end);
        shader.set ("fog_color",         fog.color);
        shader.set ("albedo_texture",    0);

        prepared_shaders.push_back (&shader);
    }

    void Renderer::submit (const Mesh & mesh, const Material & material, const glm::mat4 & model_matrix)
    {
        if (!material.shader) return;

        const Shader & shader = *material.shader;

        prepare_shader (shader);

        const glm::mat4 model_view = view * model_matrix;

        // Normals need the inverse transpose so non-uniform scales keep them perpendicular.
        const glm::mat3 normal_matrix = glm::transpose (glm::inverse (glm::mat3(model_view)));

        shader.set ("model_view_matrix", model_view);
        shader.set ("normal_matrix",     normal_matrix);

        shader.set ("material.color",     material.color);
        shader.set ("material.uv_tiling", material.uv_tiling);
        shader.set ("material.ambient",   material.ambient);
        shader.set ("material.diffuse",   material.diffuse);
        shader.set ("material.specular",  material.specular);
        shader.set ("material.shininess", material.shininess);
        shader.set ("material.lit",       material.receives_lighting ? 1 : 0);

        (material.albedo ? *material.albedo : *white_texture).bind (0);

        mesh.draw ();

        ++draw_calls;
        triangles += unsigned(mesh.get_index_count () / 3);
    }

}
