// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.
//
// Blinn-Phong lit surface, ported from VS_3D-Renderer (02 · Scene render).
// Lighting is computed in eye space.

#version 330 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform mat3 normal_matrix;

struct Material
{
    vec3  color;
    vec2  uv_tiling;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
    bool  lit;
};

uniform Material material;

out vec3 view_position;
out vec3 view_normal;
out vec2 texture_uv;

void main ()
{
    vec4 position_in_view = model_view_matrix * vec4(vertex_position, 1.0);

    view_position = position_in_view.xyz;
    view_normal   = normal_matrix * vertex_normal;
    texture_uv    = vertex_uv * material.uv_tiling;

    gl_Position   = projection_matrix * position_in_view;
}
