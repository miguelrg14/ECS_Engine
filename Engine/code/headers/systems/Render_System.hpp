// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "System.hpp"
#include "renderer/Renderer.hpp"

namespace engine
{

    class Window;

    /// Draws every entity with Transform + Mesh_Renderer through the Renderer,
    /// seen from the first active Transform + Camera_Component and lit by the
    /// first Transform + Light_Component. It is the only place where the ECS
    /// and the renderer meet.
    class Render_System final : public System
    {
        Renderer & renderer;
        Window   & window;

        Fog        fog;
        glm::vec3  clear_color   { 0.08f, 0.10f, 0.14f };   // used when fog is off
        bool       warned_no_camera = false;
        bool       wireframe        = false;

        Entity     current_camera = null_entity;

    public:

        Render_System (Registry & registry, Renderer & renderer, Window & window);

        void execute (float delta_time) override;

    public:

        Fog       & get_fog         ()       { return fog; }
        glm::vec3 & get_clear_color ()       { return clear_color; }
        void        set_clear_color (const glm::vec3 & color) { clear_color = color; }

        bool        is_wireframe    () const { return wireframe; }
        void        set_wireframe   (bool enabled) { wireframe = enabled; }

        /// Camera used in the last frame, or null_entity.
        Entity      get_camera      () const { return current_camera; }

    private:

        bool find_camera (glm::mat4 & view, glm::mat4 & projection);
        Point_Light find_light () ;
    };

}
