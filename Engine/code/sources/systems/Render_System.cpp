// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "systems/Render_System.hpp"

#include "components/Camera_Component.hpp"
#include "components/Light_Component.hpp"
#include "components/Mesh_Renderer.hpp"
#include "components/Transform.hpp"
#include "core/Log.hpp"
#include "core/Window.hpp"
#include "renderer/Render_Command.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>

namespace engine
{

    Render_System::Render_System (Registry & given_registry, Renderer & given_renderer, Window & given_window)
    :
        System  ("Render_System", Priority::render, given_registry),
        renderer(given_renderer),
        window  (given_window  )
    {
    }

    void Render_System::execute (float )
    {
        Render_Command::set_clear_color (glm::vec4(fog.enabled ? fog.color : clear_color, 1.f));
        Render_Command::clear ();

        glm::mat4 view, projection;

        if (!find_camera (view, projection))
        {
            if (!warned_no_camera)
            {
                log_warning ("Render_System: no entity with Transform + an active Camera_Component; nothing is drawn");
                warned_no_camera = true;
            }
            return;
        }

        renderer.begin_frame (view, projection, find_light (), fog);

        registry.view< Transform, Mesh_Renderer >().each ([this] (const Transform & transform, const Mesh_Renderer & mesh_renderer)
        {
            if (mesh_renderer.visible && mesh_renderer.mesh && mesh_renderer.material)
            {
                renderer.submit (*mesh_renderer.mesh, *mesh_renderer.material, transform.world);
            }
        });
    }

    bool Render_System::find_camera (glm::mat4 & view, glm::mat4 & projection)
    {
        current_camera = null_entity;

        registry.view< Transform, Camera_Component >().each ([&] (Entity entity, const Transform & transform, const Camera_Component & camera)
        {
            if (current_camera != null_entity || !camera.active) return;

            current_camera = entity;

            // The view matrix is the inverse of the camera's placement in the world.
            // Scale is removed so a scaled parent does not distort the view.
            glm::mat4 placement = transform.world;
            placement[0] = glm::vec4(glm::normalize (glm::vec3(placement[0])), 0.f);
            placement[1] = glm::vec4(glm::normalize (glm::vec3(placement[1])), 0.f);
            placement[2] = glm::vec4(glm::normalize (glm::vec3(placement[2])), 0.f);

            view       = glm::inverse (placement);
            projection = glm::perspective
            (
                glm::radians (camera.fov_degrees), window.get_aspect (), camera.near_plane, camera.far_plane
            );
        });

        return current_camera != null_entity;
    }

    Point_Light Render_System::find_light ()
    {
        Point_Light light;
        bool        found = false;

        registry.view< Transform, Light_Component >().each ([&] (const Transform & transform, const Light_Component & component)
        {
            if (found) return;

            light.position = transform.get_world_position ();
            light.color    = component.color * component.intensity;
            found          = true;
        });

        if (!found) light.color = glm::vec3(0.f);       // only ambient light

        return light;
    }

}
