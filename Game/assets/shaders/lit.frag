// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.
//
// Blinn-Phong lit surface, ported from VS_3D-Renderer (02 · Scene render):
// one point light, albedo texture, linear distance fog.

#version 330 core

struct Light
{
    vec3 position;          // eye space
    vec3 color;
};

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

uniform Light     light;
uniform Material  material;
uniform sampler2D albedo_texture;

uniform bool  fog_enabled;
uniform float fog_start;
uniform float fog_end;
uniform vec3  fog_color;

in vec3 view_position;
in vec3 view_normal;
in vec2 texture_uv;

out vec4 fragment_color;

void main ()
{
    vec4 albedo = texture(albedo_texture, texture_uv) * vec4(material.color, 1.0);

    vec3 color = albedo.rgb;

    if (material.lit)
    {
        vec3 N = normalize(view_normal);
        vec3 L = normalize(light.position - view_position);
        vec3 V = normalize(-view_position);
        vec3 H = normalize(L + V);

        // Lights both faces of thin geometry (leaves, cloth).
        if (!gl_FrontFacing) N = -N;

        float diffuse  = material.diffuse  * max(dot(N, L), 0.0);
        float specular = material.specular * pow(max(dot(N, H), 0.0), material.shininess);

        // No highlight on faces turned away from the light.
        if (dot(N, L) <= 0.0) specular = 0.0;

        color = albedo.rgb * (material.ambient + diffuse * light.color) + specular * light.color;
    }

    if (fog_enabled)
    {
        float distance = length(view_position);
        float fog      = clamp((distance - fog_start) / (fog_end - fog_start), 0.0, 1.0);

        color = mix(color, fog_color, fog);
    }

    fragment_color = vec4(color, albedo.a);
}
