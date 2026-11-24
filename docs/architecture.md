# Architecture

This document grows with each phase of the [roadmap](../README.md#roadmap).

## Overview

```text
Game (executable)        main.cpp, scenes, scripts
   │  creates entities, components and scripts
   ▼
Engine (static library)
   ├── Kernel            runs the systems in priority order every frame        (phase 2)
   ├── Systems           Input → Script → Movement → Collision → Transform
   │                     → Event → Render → Debug UI                         (phases 2–8)
   ├── Registry (ECS)    entity IDs + one sparse-set pool per component type  (phase 4)
   ├── Event_Bus         typed publish / subscribe, dispatched once per frame (phase 6)
   └── Renderer          OpenGL 3.3: shaders, meshes, textures, materials     (phases 1, 3)
                         It does not know about the ECS; Render_System translates.
   ▼
libraries/               SDL2 · GLAD · GLM · stb_image · tinyobjloader (Dear ImGui later)
```

## Design rules

- **The game only talks to the engine through the registry and its own scripts.** Nothing in `Game/` calls OpenGL or SDL directly once the kernel exists.
- **The renderer is independent of the ECS.** Systems translate components into renderer calls, so the renderer can be reused or tested on its own.
- **Components are plain data.** No inheritance, no virtual functions, no back-pointer to an owner.
- **Typed, compile-time checked APIs.** Components and events are identified by their C++ type, never by string keys.

## Main loop (phase 2)

`Application` owns SDL, the `Window`, the `Input` state and the `Kernel`, and registers the engine's own tasks. The game adds its tasks to the kernel and calls `run()`:

```cpp
engine::Application application ({ .title = "ECS Engine" });

Model_Viewer viewer (application);          // a Task defined in Game/

application.get_kernel ().add (viewer);
application.run ();                         // returns when the kernel stops
```

Each frame the `Kernel`:

1. Applies tasks added or removed during the previous frame (`initialize` / `finalize` run then), so the list never changes while it is iterated.
2. Measures the previous frame with a monotonic `Timer` and clamps it to 0.25 s, so a breakpoint or a window drag does not produce one huge step.
3. Calls `execute(delta_time)` on every task in priority order. Equal priorities keep the order in which they were added.

When `stop()` is called the loop ends and every task is finalized in reverse order.

| Priority | Task | Phase |
|---|---|---|
| 10 | `Input_System` — SDL events → `Input`, resize, window close | 2 |
| 20 | Scripts (the test scene `Model_Viewer` runs here for now) | 2, 6 |
| 30–60 | Movement, collision, transform, events | 5–7 |
| 90 | `Render_System` | 5 |
| 100 | Debug UI | 8 |
| 1000 | `Present_System` — swaps the window buffers | 2 |

`Input` stores the current and previous state of every key (by scancode, so WASD keeps its place on any layout) and mouse button. That gives three queries per key: held (`is_key_down`), went down this frame (`was_key_pressed`) and went up this frame (`was_key_released`). Losing focus clears the state, because keys released outside the window never send a key-up event.

## Renderer (phase 3)

The renderer draws meshes with materials and knows nothing about entities. A frame is:

```cpp
renderer.begin_frame (camera, light, fog);              // view, projection, light and fog for this frame
renderer.submit (mesh, material, model_matrix);         // one call per object
```

| Class | Responsibility |
|---|---|
| `Shader` | Compiles and links a vertex + fragment program; throws with the driver's log on error. Caches uniform locations by name. |
| `Texture` | RGBA8 2D texture with mipmaps, loaded with stb_image (flipped so the first row is the bottom one, as OpenGL expects). |
| `Mesh` | One VAO with an interleaved vertex buffer (position, normal, uv at locations 0, 1, 2) and an index buffer. Computes its bounds. Factories for a cube and a plane. |
| `load_obj` | Reads an .obj with tinyobjloader, merges all its objects, shares identical vertices and generates smooth normals when the file has none. |
| `Material` | Shader, albedo texture, colour, UV tiling and Blinn-Phong parameters. Shared by any number of meshes. |
| `Camera` | Position, yaw and pitch, perspective parameters → view and projection matrices. No input: a controller moves it. |
| `Renderer` | Sends per-frame uniforms once per shader per frame and per-object uniforms (model-view and normal matrices, material) on each submit. Counts draw calls and triangles. |
| `Resource_Manager` | Loads shaders, textures and meshes on first request and returns the same `shared_ptr` afterwards. Owned by `Application`, declared after the window so GPU objects are deleted before the OpenGL context. |

Shaders live in `Game/assets/shaders/` as `.vert` / `.frag` files. Lighting is computed in eye space: the light position is transformed by the view matrix once per frame.

## Modules so far

| Module | Files | Responsibility |
|---|---|---|
| `core/Window` | `Window.hpp/.cpp` | Creates the SDL2 window and the OpenGL 3.3 Core context, loads the GL functions with GLAD, handles vsync, resize and buffer swaps. RAII: the destructor releases both handles. |
| `core/Log` | `Log.hpp` | `log_info`, `log_warning`, `log_error` over `SDL_Log` (console and Visual Studio Output window). |
| `renderer/Render_Command` | `Render_Command.hpp/.cpp` | Stateless per-frame GL calls: clear colour, clear, viewport. |
| `core/Task` | `Task.hpp` | Base class of every per-frame task: name, priority, `initialize` / `execute(dt)` / `finalize`. Priority constants. |
| `core/Kernel` | `Kernel.hpp/.cpp` | Main loop, priority-sorted task list, deferred add/remove, delta time, FPS. |
| `core/Timer` | `Timer.hpp` | Elapsed seconds over `std::chrono::steady_clock`. |
| `core/Input` | `Input.hpp` | Keyboard and mouse state for the current frame. |
| `core/Application` | `Application.hpp/.cpp` | Owns SDL, window, resources, renderer, input and kernel; registers the engine tasks. |
| `systems/Input_System` | `Input_System.hpp/.cpp` | Fills `Input` from the SDL event queue. |
| `systems/Present_System` | `Present_System.hpp` | Swaps the window buffers at the end of the frame. |
| `renderer/*` | `Shader`, `Texture`, `Mesh`, `Model_Loader`, `Material`, `Camera`, `Renderer`, `Render_Command` | See the renderer section above. |
| `resources/Resource_Manager` | `Resource_Manager.hpp/.cpp` | Asset cache by path. |
| `Game` | `main.cpp`, `Model_Viewer`, `Fly_Camera` | Creates the application and a test scene that uses the renderer directly; becomes entities in phase 5. |
