// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace engine
{

    /// GLSL program made of a vertex and a fragment shader. Compilation and link
    /// errors throw std::runtime_error with the driver's log.
    /// Uniform locations are looked up once and cached by name.
    class Shader
    {
        std::uint32_t program_id = 0;
        std::string   name;

        mutable std::unordered_map< std::string, std::int32_t > locations;

    public:

        Shader (const std::string & vertex_code, const std::string & fragment_code, std::string debug_name);
       ~Shader ();

        Shader (const Shader & ) = delete;
        Shader & operator = (const Shader & ) = delete;

    public:

        void bind () const;

        void set (const std::string & uniform, int               value) const;
        void set (const std::string & uniform, float             value) const;
        void set (const std::string & uniform, const glm::vec2 & value) const;
        void set (const std::string & uniform, const glm::vec3 & value) const;
        void set (const std::string & uniform, const glm::vec4 & value) const;
        void set (const std::string & uniform, const glm::mat3 & value) const;
        void set (const std::string & uniform, const glm::mat4 & value) const;

        const std::string & get_name () const { return name;       }
        std::uint32_t       get_id   () const { return program_id; }

    private:

        /// -1 when the uniform does not exist or was optimized away; glUniform ignores -1.
        std::int32_t location (const std::string & uniform) const;
    };

}
