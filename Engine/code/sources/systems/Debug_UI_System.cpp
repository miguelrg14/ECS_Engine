// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "systems/Debug_UI_System.hpp"

#include "components.hpp"
#include "core/Input.hpp"
#include "core/Kernel.hpp"
#include "core/Window.hpp"
#include "renderer/Material.hpp"
#include "renderer/Mesh.hpp"
#include "renderer/Render_Command.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Shader.hpp"
#include "systems/Collision_System.hpp"
#include "systems/Input_System.hpp"
#include "systems/Render_System.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <typeinfo>

#if defined(__GNUG__)
#include <cstdlib>
#include <cxxabi.h>
#endif

namespace engine
{

    namespace
    {

        // Flat colour, for the collider boxes. Uses the same uniform names as the
        // lit shader, so it goes through Renderer::submit like any other material.

        const char * const debug_vertex_shader = R"(
            #version 330 core
            layout(location = 0) in vec3 vertex_position;
            uniform mat4 model_view_matrix;
            uniform mat4 projection_matrix;
            void main () { gl_Position = projection_matrix * model_view_matrix * vec4(vertex_position, 1.0); }
        )";

        const char * const debug_fragment_shader = R"(
            #version 330 core
            struct Material { vec3 color; };
            uniform Material material;
            out vec4 fragment_color;
            void main () { fragment_color = vec4(material.color, 1.0); }
        )";

        /// Readable class name of a script ("Player_Controller").
        std::string type_name (const Script & script)
        {
            const char * raw = typeid(script).name ();

            #if defined(__GNUG__)
                int    status    = 0;
                char * demangled = abi::__cxa_demangle (raw, nullptr, nullptr, &status);
                std::string name = status == 0 && demangled ? demangled : raw;
                std::free (demangled);
            #else
                std::string name = raw;                                    // MSVC: "class Player_Controller"
                if (name.rfind ("class ",  0) == 0) name.erase (0, 6);
                if (name.rfind ("struct ", 0) == 0) name.erase (0, 7);
            #endif

            return name;
        }

        bool edit_vec3 (const char * label, glm::vec3 & value, float speed = 0.05f)
        {
            return ImGui::DragFloat3 (label, glm::value_ptr (value), speed);
        }

    }

    Debug_UI_System::Debug_UI_System
    (
        Registry & given_registry, Kernel & given_kernel, Window & given_window, const Input & given_input,
        Input_System & given_input_system, Renderer & given_renderer, Render_System & given_render_system,
        Collision_System & given_collision_system
    )
    :
        System          ("Debug_UI_System", Priority::debug_ui, given_registry),
        kernel          (given_kernel          ),
        window          (given_window          ),
        input           (given_input           ),
        input_system    (given_input_system    ),
        renderer        (given_renderer        ),
        render_system   (given_render_system   ),
        collision_system(given_collision_system)
    {
    }

    Debug_UI_System::~Debug_UI_System ()
    {
        if (!initialized) return;

        input_system.set_event_filter (nullptr);

        ImGui_ImplOpenGL3_Shutdown ();
        ImGui_ImplSDL2_Shutdown    ();
        ImGui::DestroyContext      ();
    }

    void Debug_UI_System::initialize ()
    {
        if (initialized) return;

        IMGUI_CHECKVERSION ();
        ImGui::CreateContext ();

        ImGuiIO & io = ImGui::GetIO ();
        io.IniFilename  = nullptr;                       // no imgui.ini next to the assets
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark ();
        ImGuiStyle & style = ImGui::GetStyle ();
        style.WindowRounding   = 6.f;
        style.FrameRounding    = 4.f;
        style.GrabRounding     = 4.f;
        style.WindowBorderSize = 0.f;
        style.Colors[ImGuiCol_WindowBg].w = 0.92f;

        ImGui_ImplSDL2_InitForOpenGL (window.get_handle (), window.get_context ());
        ImGui_ImplOpenGL3_Init       ("#version 330 core");

        // ImGui sees every event first; what it takes (typing in a field, clicking
        // a window) does not reach the game.
        input_system.set_event_filter ([] (const SDL_Event & event)
        {
            ImGui_ImplSDL2_ProcessEvent (&event);

            const ImGuiIO & state = ImGui::GetIO ();
            return Input_System::Capture{ state.WantCaptureKeyboard, state.WantCaptureMouse };
        });

        // Collider boxes.

        auto shader = std::make_shared< Shader >(debug_vertex_shader, debug_fragment_shader, "debug_color");

        auto make = [&shader] (const glm::vec3 & color)
        {
            auto material    = std::make_shared< Material >();
            material->shader = shader;
            material->color  = color;
            return material;
        };

        box_mesh             = Mesh::create_cube ();
        static_box_material  = make ({ 0.95f, 0.35f, 0.30f });
        dynamic_box_material = make ({ 0.35f, 0.90f, 0.40f });
        trigger_box_material = make ({ 1.00f, 0.85f, 0.20f });

        add_engine_inspectors ();

        initialized = true;
    }

    void Debug_UI_System::execute (float delta_time)
    {
        frame_times[frame_index] = delta_time * 1000.f;
        frame_index = (frame_index + 1) % frame_times.size ();

        if (input.was_key_pressed (SDL_SCANCODE_F1)) visible = !visible;

        if (show_colliders) draw_colliders ();

        ImGui_ImplOpenGL3_NewFrame ();
        ImGui_ImplSDL2_NewFrame    ();
        ImGui::NewFrame            ();

        for (auto & overlay : overlays) overlay ();

        if (visible)
        {
            draw_stats     ();
            draw_scene     ();
            draw_inspector ();
            draw_render    ();
        }

        ImGui::Render ();
        ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData ());
    }

    // ------------------------------------------------------------------------
    // Stats

    void Debug_UI_System::draw_stats ()
    {
        ImGui::SetNextWindowPos  ({ 10.f, 10.f },   ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize ({ 330.f, 0.f },   ImGuiCond_FirstUseEver);

        if (!ImGui::Begin ("Stats  (F1 hides the tools)")) { ImGui::End (); return; }

        const float fps = kernel.get_fps ();

        char overlay[64];
        std::snprintf (overlay, sizeof(overlay), "%.0f FPS  %.2f ms", double(fps), fps > 0.f ? 1000.0 / double(fps) : 0.0);

        ImGui::PlotLines ("##frames", frame_times.data (), int(frame_times.size ()), int(frame_index), overlay, 0.f, 40.f, { -1.f, 60.f });

        const std::size_t colliders = collision_system.get_collider_count ();

        ImGui::Text ("Entities     %zu", registry.alive_count ());
        ImGui::Text ("Draw calls   %u  (%u triangles)", renderer.get_draw_calls (), renderer.get_triangles ());
        ImGui::Text ("Colliders    %zu", colliders);
        ImGui::Text ("Box tests    %zu of %zu pairs", collision_system.get_pair_tests (), colliders * (colliders > 0 ? colliders - 1 : 0) / 2);

        if (ImGui::IsItemHovered ())
        {
            ImGui::SetTooltip ("Sweep and prune only tests boxes whose x ranges overlap,\ninstead of every possible pair.");
        }

        ImGui::SeparatorText ("Time per task (ms)");

        if (ImGui::BeginTable ("tasks", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            for (const Kernel::Task_Timing & timing : kernel.get_task_timings ())
            {
                ImGui::TableNextRow ();
                ImGui::TableNextColumn (); ImGui::TextDisabled ("%d", timing.task->get_priority ());
                ImGui::TableNextColumn (); ImGui::TextUnformatted (timing.task->get_name ().c_str ());
                ImGui::TableNextColumn (); ImGui::Text ("%.3f", double(timing.milliseconds));
            }

            ImGui::EndTable ();
        }

        ImGui::End ();
    }

    // ------------------------------------------------------------------------
    // Scene hierarchy

    std::string Debug_UI_System::label_of (Entity entity) const
    {
        char id[32];
        std::snprintf (id, sizeof(id), "#%u", entity::index_of (entity));

        const Name * entity_name = registry.try_get< Name >(entity);

        return entity_name && !entity_name->value.empty () ? entity_name->value + "  " + id : std::string("Entity ") + id;
    }

    void Debug_UI_System::draw_scene ()
    {
        const ImVec2 display = ImGui::GetIO ().DisplaySize;

        ImGui::SetNextWindowPos  ({ display.x - 330.f, 10.f }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize ({ 320.f, 300.f },            ImGuiCond_FirstUseEver);

        if (!ImGui::Begin ("Scene")) { ImGui::End (); return; }

        if (ImGui::Button ("Create entity"))
        {
            selected = registry.create ();
            registry.add< Name      >(selected, "New entity");
            registry.add< Transform >(selected);
        }

        ImGui::SameLine ();
        ImGui::SetNextItemWidth (-1.f);

        char buffer[64];
        std::snprintf (buffer, sizeof(buffer), "%s", filter.c_str ());

        if (ImGui::InputTextWithHint ("##filter", "Filter by name", buffer, sizeof(buffer))) filter = buffer;

        // Children of each entity, from Transform::parent.

        std::unordered_map< Entity, std::vector< Entity > > children;
        std::vector< Entity > roots;

        registry.each ([&] (Entity entity)
        {
            const Transform * transform = registry.try_get< Transform >(entity);

            if (transform && registry.valid (transform->parent) && transform->parent != entity)
            {
                children[transform->parent].push_back (entity);
            }
            else
            {
                roots.push_back (entity);
            }
        });

        ImGui::BeginChild ("tree");

        for (Entity entity : roots)
        {
            if (!filter.empty () && label_of (entity).find (filter) == std::string::npos) continue;

            draw_entity (entity, children);
        }

        ImGui::EndChild ();
        ImGui::End ();
    }

    void Debug_UI_System::draw_entity (Entity entity, const std::unordered_map< Entity, std::vector< Entity > > & children)
    {
        const auto found      = children.find (entity);
        const bool has_childs = found != children.end ();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (!has_childs)         flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (entity == selected)  flags |= ImGuiTreeNodeFlags_Selected;

        const std::string label = label_of (entity);

        const bool open = ImGui::TreeNodeEx (reinterpret_cast< void * >(std::uintptr_t(entity)), flags, "%s", label.c_str ());

        if (ImGui::IsItemClicked () && !ImGui::IsItemToggledOpen ()) selected = entity;

        if (has_childs && open)
        {
            for (Entity child : found->second) draw_entity (child, children);
            ImGui::TreePop ();
        }
    }

    // ------------------------------------------------------------------------
    // Inspector

    void Debug_UI_System::draw_inspector ()
    {
        const ImVec2 display = ImGui::GetIO ().DisplaySize;

        ImGui::SetNextWindowPos  ({ display.x - 330.f, 320.f }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize ({ 320.f, 380.f },             ImGuiCond_FirstUseEver);

        if (!ImGui::Begin ("Inspector")) { ImGui::End (); return; }

        if (!registry.valid (selected))
        {
            selected = null_entity;
            ImGui::TextDisabled ("Select an entity in the Scene window.");
            ImGui::End ();
            return;
        }

        ImGui::Text ("Entity  index %u  version %u", entity::index_of (selected), entity::version_of (selected));

        ImGui::SameLine (ImGui::GetContentRegionAvail ().x - 50.f);

        if (ImGui::SmallButton ("Destroy"))
        {
            registry.destroy (selected);
            selected = null_entity;
            ImGui::End ();
            return;
        }

        ImGui::Separator ();

        for (Inspector & inspector : inspectors)
        {
            if (!inspector.has (registry, selected)) continue;

            ImGui::PushID (inspector.label.c_str ());

            if (ImGui::CollapsingHeader (inspector.label.c_str (), ImGuiTreeNodeFlags_DefaultOpen))
            {
                inspector.draw (registry, selected);
            }

            ImGui::PopID ();
        }

        ImGui::End ();
    }

    void Debug_UI_System::add_engine_inspectors ()
    {
        add_inspector< Name >("Name", [] (Name & component)
        {
            char buffer[128];
            std::snprintf (buffer, sizeof(buffer), "%s", component.value.c_str ());
            if (ImGui::InputText ("##name", buffer, sizeof(buffer))) component.value = buffer;
        });

        add_inspector< Transform >("Transform", [this] (Transform & transform)
        {
            edit_vec3 ("Position", transform.position);

            // Edited as Euler angles; written back only when changed, so it does not drift.
            glm::vec3 degrees = glm::degrees (glm::eulerAngles (transform.rotation));

            if (edit_vec3 ("Rotation", degrees, 0.5f)) transform.rotation = glm::quat (glm::radians (degrees));

            edit_vec3 ("Scale", transform.scale, 0.01f);

            if (registry.valid (transform.parent))
            {
                ImGui::Text ("Parent: %s", label_of (transform.parent).c_str ());
                ImGui::SameLine ();
                if (ImGui::SmallButton ("Select")) selected = transform.parent;
            }

            const glm::vec3 world = transform.get_world_position ();
            ImGui::TextDisabled ("World position  %.2f  %.2f  %.2f", double(world.x), double(world.y), double(world.z));
        });

        add_inspector< Velocity >("Velocity", [] (Velocity & velocity)
        {
            edit_vec3 ("Linear",  velocity.linear);
            edit_vec3 ("Angular", velocity.angular, 0.01f);
        });

        add_inspector< Mesh_Renderer >("Mesh Renderer", [] (Mesh_Renderer & mesh_renderer)
        {
            ImGui::Checkbox ("Visible", &mesh_renderer.visible);

            if (mesh_renderer.mesh) ImGui::TextDisabled ("%d triangles", mesh_renderer.mesh->get_index_count () / 3);

            if (Material * material = mesh_renderer.material.get ())
            {
                ImGui::TextDisabled ("Material shared by %ld renderers", long(mesh_renderer.material.use_count ()));

                ImGui::ColorEdit3  ("Color",     glm::value_ptr (material->color));
                ImGui::SliderFloat ("Ambient",   &material->ambient,   0.f, 1.f);
                ImGui::SliderFloat ("Diffuse",   &material->diffuse,   0.f, 1.f);
                ImGui::SliderFloat ("Specular",  &material->specular,  0.f, 1.f);
                ImGui::SliderFloat ("Shininess", &material->shininess, 1.f, 256.f, "%.0f", ImGuiSliderFlags_Logarithmic);
                ImGui::Checkbox    ("Lit",       &material->receives_lighting);
            }
        });

        add_inspector< Camera_Component >("Camera", [] (Camera_Component & camera)
        {
            ImGui::Checkbox    ("Active",          &camera.active);
            ImGui::SliderFloat ("Field of view",   &camera.fov_degrees, 20.f, 120.f, "%.0f deg");
            ImGui::DragFloat   ("Near",            &camera.near_plane, 0.01f, 0.01f, 10.f);
            ImGui::DragFloat   ("Far",             &camera.far_plane,  1.f,   1.f,   1000.f);
        });

        add_inspector< Light_Component >("Light", [] (Light_Component & light)
        {
            ImGui::ColorEdit3  ("Color",     glm::value_ptr (light.color));
            ImGui::SliderFloat ("Intensity", &light.intensity, 0.f, 3.f);
        });

        add_inspector< Box_Collider >("Box Collider", [] (Box_Collider & collider)
        {
            edit_vec3 ("Half extents", collider.half_extents, 0.01f);
            edit_vec3 ("Offset",       collider.offset,       0.01f);
            ImGui::Checkbox ("Trigger", &collider.is_trigger);
            ImGui::SameLine ();
            ImGui::Checkbox ("Static",  &collider.is_static);
            ImGui::TextDisabled ("Last push  %.3f  %.3f  %.3f",
                                 double(collider.last_push.x), double(collider.last_push.y), double(collider.last_push.z));
        });

        add_inspector< Script_Component >("Scripts", [] (Script_Component & component)
        {
            for (const auto & script : component.scripts)
            {
                if (script) ImGui::BulletText ("%s", type_name (*script).c_str ());
            }
        });
    }

    // ------------------------------------------------------------------------
    // Render settings and collider boxes

    void Debug_UI_System::draw_render ()
    {
        // Bottom-left corner, under the stats.
        ImGui::SetNextWindowPos  ({ 10.f, ImGui::GetIO ().DisplaySize.y - 10.f }, ImGuiCond_FirstUseEver, { 0.f, 1.f });
        ImGui::SetNextWindowSize ({ 330.f, 0.f }, ImGuiCond_FirstUseEver);

        if (!ImGui::Begin ("Render")) { ImGui::End (); return; }

        Fog & fog = render_system.get_fog ();

        ImGui::Checkbox   ("Fog", &fog.enabled);

        if (fog.enabled)
        {
            ImGui::DragFloatRange2 ("Fog distance", &fog.start, &fog.end, 0.5f, 0.f, 500.f);
            ImGui::ColorEdit3      ("Fog color",    glm::value_ptr (fog.color));
        }
        else
        {
            ImGui::ColorEdit3 ("Clear color", glm::value_ptr (render_system.get_clear_color ()));
        }

        bool wireframe = render_system.is_wireframe ();
        if (ImGui::Checkbox ("Wireframe", &wireframe)) render_system.set_wireframe (wireframe);

        bool vsync = window.is_vsync ();
        if (ImGui::Checkbox ("Vsync", &vsync)) window.set_vsync (vsync);

        ImGui::Checkbox ("Show colliders", &show_colliders);

        // Colour key on the same line, so the window never changes size.
        ImGui::SameLine (); ImGui::TextColored ({ 0.95f, 0.35f, 0.30f, 1.f }, "static");
        ImGui::SameLine (); ImGui::TextColored ({ 0.35f, 0.90f, 0.40f, 1.f }, "dynamic");
        ImGui::SameLine (); ImGui::TextColored ({ 1.00f, 0.85f, 0.20f, 1.f }, "trigger");

        ImGui::End ();
    }

    void Debug_UI_System::draw_colliders ()
    {
        // Needs the camera of this frame, set by Render_System.
        if (render_system.get_camera () == null_entity) return;

        Render_Command::set_wireframe (true);

        registry.view< Transform, Box_Collider >().each ([this] (const Transform & transform, const Box_Collider & collider)
        {
            const Aabb box = Collision_System::compute_world_box (transform, collider);

            const glm::mat4 model = glm::scale (glm::translate (glm::mat4(1.f), (box.min + box.max) * 0.5f), box.max - box.min);

            const Material & material = collider.is_trigger ? *trigger_box_material
                                      : collider.is_static  ? *static_box_material
                                      :                       *dynamic_box_material;

            renderer.submit (*box_mesh, material, model);
        });

        Render_Command::set_wireframe (false);
    }

}
