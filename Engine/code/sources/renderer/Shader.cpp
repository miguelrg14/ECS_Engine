// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "renderer/Shader.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

namespace engine
{

    namespace
    {

        std::string shader_log (GLuint shader_id)
        {
            GLint length = 0;
            glGetShaderiv (shader_id, GL_INFO_LOG_LENGTH, &length);

            std::string log (std::size_t(std::max (length, 1)), '\0');
            glGetShaderInfoLog (shader_id, length, nullptr, log.data ());
            return log;
        }

        std::string program_log (GLuint program_id)
        {
            GLint length = 0;
            glGetProgramiv (program_id, GL_INFO_LOG_LENGTH, &length);

            std::string log (std::size_t(std::max (length, 1)), '\0');
            glGetProgramInfoLog (program_id, length, nullptr, log.data ());
            return log;
        }

        GLuint compile (GLenum type, const std::string & code, const std::string & name)
        {
            GLuint shader_id = glCreateShader (type);

            const char * source = code.c_str ();
            const GLint  size   = GLint(code.size ());

            glShaderSource  (shader_id, 1, &source, &size);
            glCompileShader (shader_id);

            GLint succeeded = GL_FALSE;
            glGetShaderiv (shader_id, GL_COMPILE_STATUS, &succeeded);

            if (!succeeded)
            {
                const std::string log = shader_log (shader_id);
                glDeleteShader (shader_id);

                throw std::runtime_error
                (
                    "Shader '" + name + "': " + (type == GL_VERTEX_SHADER ? "vertex" : "fragment") +
                    " shader failed to compile:\n" + log
                );
            }

            return shader_id;
        }

    }

    Shader::Shader (const std::string & vertex_code, const std::string & fragment_code, std::string debug_name)
    :
        name(std::move(debug_name))
    {
        const GLuint vertex_id   = compile (GL_VERTEX_SHADER,   vertex_code,   name);
        GLuint       fragment_id = 0;

        try
        {
            fragment_id = compile (GL_FRAGMENT_SHADER, fragment_code, name);
        }
        catch (...)
        {
            glDeleteShader (vertex_id);
            throw;
        }

        program_id = glCreateProgram ();

        glAttachShader (program_id, vertex_id  );
        glAttachShader (program_id, fragment_id);
        glLinkProgram  (program_id);

        // Once linked, the program keeps what it needs from the shader objects.

        glDetachShader (program_id, vertex_id  );
        glDetachShader (program_id, fragment_id);
        glDeleteShader (vertex_id  );
        glDeleteShader (fragment_id);

        GLint succeeded = GL_FALSE;
        glGetProgramiv (program_id, GL_LINK_STATUS, &succeeded);

        if (!succeeded)
        {
            const std::string log = program_log (program_id);
            glDeleteProgram (program_id);

            throw std::runtime_error ("Shader '" + name + "' failed to link:\n" + log);
        }
    }

    Shader::~Shader ()
    {
        glDeleteProgram (program_id);
    }

    void Shader::bind () const
    {
        glUseProgram (program_id);
    }

    std::int32_t Shader::location (const std::string & uniform) const
    {
        auto found = locations.find (uniform);

        if (found != locations.end ()) return found->second;

        const GLint location = glGetUniformLocation (program_id, uniform.c_str ());

        locations.emplace (uniform, location);

        return location;
    }

    void Shader::set (const std::string & uniform, int               value) const { glUniform1i        (location (uniform), value); }
    void Shader::set (const std::string & uniform, float             value) const { glUniform1f        (location (uniform), value); }
    void Shader::set (const std::string & uniform, const glm::vec2 & value) const { glUniform2fv       (location (uniform), 1, glm::value_ptr (value)); }
    void Shader::set (const std::string & uniform, const glm::vec3 & value) const { glUniform3fv       (location (uniform), 1, glm::value_ptr (value)); }
    void Shader::set (const std::string & uniform, const glm::vec4 & value) const { glUniform4fv       (location (uniform), 1, glm::value_ptr (value)); }
    void Shader::set (const std::string & uniform, const glm::mat3 & value) const { glUniformMatrix3fv (location (uniform), 1, GL_FALSE, glm::value_ptr (value)); }
    void Shader::set (const std::string & uniform, const glm::mat4 & value) const { glUniformMatrix4fv (location (uniform), 1, GL_FALSE, glm::value_ptr (value)); }

}
