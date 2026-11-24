// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "resources/Resource_Manager.hpp"

#include "core/Log.hpp"
#include "renderer/Model_Loader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace engine
{

    namespace
    {

        std::string read_text_file (const std::string & path)
        {
            std::ifstream file (path, std::ios::binary);

            if (!file)
            {
                throw std::runtime_error ("Resources: cannot open '" + path + "'");
            }

            std::ostringstream content;
            content << file.rdbuf ();
            return content.str ();
        }

    }

    Resource_Manager::Resource_Manager (std::string assets_folder)
    :
        root(std::move(assets_folder))
    {
        if (!root.empty () && root.back () != '/' && root.back () != '\\') root += '/';
    }

    std::shared_ptr< Shader > Resource_Manager::shader (const std::string & name)
    {
        if (auto found = shaders.find (name); found != shaders.end ()) return found->second;

        const std::string base = root + "shaders/" + name;

        auto loaded = std::make_shared< Shader >(read_text_file (base + ".vert"), read_text_file (base + ".frag"), name);

        log_info ("Resources: shader %s", name.c_str ());

        return shaders.emplace (name, std::move(loaded)).first->second;
    }

    std::shared_ptr< Texture > Resource_Manager::texture (const std::string & path)
    {
        if (auto found = textures.find (path); found != textures.end ()) return found->second;

        auto loaded = Texture::from_file (root + path);

        log_info ("Resources: texture %s (%dx%d)", path.c_str (), loaded->get_width (), loaded->get_height ());

        return textures.emplace (path, std::move(loaded)).first->second;
    }

    std::shared_ptr< Mesh > Resource_Manager::mesh (const std::string & path)
    {
        if (auto found = meshes.find (path); found != meshes.end ()) return found->second;

        return meshes.emplace (path, load_obj (root + path)).first->second;
    }

    std::shared_ptr< Mesh > Resource_Manager::add_mesh (const std::string & name, std::shared_ptr< Mesh > mesh)
    {
        meshes[name] = mesh;
        return mesh;
    }

    void Resource_Manager::clear ()
    {
        shaders .clear ();
        textures.clear ();
        meshes  .clear ();
    }

}
