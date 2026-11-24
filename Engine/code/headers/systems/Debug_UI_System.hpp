// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "System.hpp"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine
{

    class Collision_System;
    class Input;
    class Input_System;
    class Kernel;
    class Mesh;
    class Renderer;
    class Render_System;
    class Window;

    struct Material;

    /// Dear ImGui tools drawn over the game, toggled with F1:
    ///
    /// - Stats:     FPS and frame-time graph, time spent in each task, entity,
    ///              draw call, collider and broad-phase counts.
    /// - Scene:     every entity as a tree that follows Transform::parent, with a filter.
    /// - Inspector: the selected entity's components, editable live.
    /// - Render:    fog, clear colour, wireframe, vsync and collider boxes.
    ///
    /// The game can add its own always-visible overlays (a HUD) with add_overlay
    /// and editors for its own components with add_inspector.
    class Debug_UI_System final : public System
    {
    public:

        struct Inspector
        {
            std::string                                  label;
            std::function< bool (Registry &, Entity) >   has;
            std::function< void (Registry &, Entity) >   draw;
        };

    private:

        Kernel           & kernel;
        Window           & window;
        const Input      & input;
        Input_System     & input_system;
        Renderer         & renderer;
        Render_System    & render_system;
        Collision_System & collision_system;

        bool        initialized    = false;
        bool        visible        = true;
        bool        show_colliders = false;
        Entity      selected       = null_entity;
        std::string filter;

        std::array< float, 120 > frame_times {};
        std::size_t              frame_index = 0;

        std::vector< Inspector >               inspectors;
        std::vector< std::function< void () > > overlays;

        std::shared_ptr< Mesh     > box_mesh;
        std::shared_ptr< Material > static_box_material;
        std::shared_ptr< Material > dynamic_box_material;
        std::shared_ptr< Material > trigger_box_material;

    public:

        Debug_UI_System
        (
            Registry & registry, Kernel & kernel, Window & window, const Input & input, Input_System & input_system,
            Renderer & renderer, Render_System & render_system, Collision_System & collision_system
        );

       ~Debug_UI_System () override;

        void initialize ()                 override;
        void execute    (float delta_time) override;

    public:

        /// Drawn every frame, also when the debug tools are hidden. Use ImGui calls inside.
        void add_overlay (std::function< void () > overlay) { overlays.push_back (std::move(overlay)); }

        /// Adds an editor for a component type to the inspector.
        template< typename T, typename Function >
        void add_inspector (std::string label, Function && editor)
        {
            inspectors.push_back
            ({
                std::move(label),
                [] (Registry & world, Entity entity) { return world.has< T >(entity); },
                [editor = std::forward< Function >(editor)] (Registry & world, Entity entity) { editor (world.get< T >(entity)); }
            });
        }

        bool   is_visible   () const { return visible; }
        void   set_visible  (bool value) { visible = value; }
        Entity get_selected () const { return selected; }

    private:

        void add_engine_inspectors ();

        void draw_stats     ();
        void draw_scene     ();
        void draw_entity    (Entity entity, const std::unordered_map< Entity, std::vector< Entity > > & children);
        void draw_inspector ();
        void draw_render    ();
        void draw_colliders ();

        std::string label_of (Entity entity) const;
    };

}
