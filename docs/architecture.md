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

Demo_Scene scene (application);             // a Task defined in Game/

application.get_kernel ().add (scene);
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
| 20 | Scripts (the test scene `Demo_Scene` runs here for now) | 2, 6 |
| 30 | `Movement_System` | 5 |
| 40 | Collision | 7 |
| 50 | `Transform_System` | 5 |
| 60 | Events | 6 |
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

## ECS core (phase 4)

### Entity

`Entity` is a `std::uint32_t`: the low 24 bits are an index (a slot in the registry) and the high 8 bits a version. Destroying an entity frees its slot and increments the version stored there, so the next entity created in that slot has a different handle. `Registry::valid` compares the whole handle, which is how a stale handle is rejected instead of silently reaching the new entity. The version wraps after 256 reuses of one slot.

### Component_Pool<T>: a sparse set

```text
sparse  [entity index] -> position in dense/data  (or absent)
dense   [position]     -> entity
data    [position]     -> component T
```

- `add`: append to `dense` and `data`, write the position in `sparse`. O(1).
- `get` / `has`: one lookup in `sparse`; `has` also checks that `dense` holds the same handle, version included. O(1).
- `remove`: move the last component into the gap (swap and pop) and fix that entity's `sparse` entry. O(1); order is not kept.
- Components of one type are contiguous, so iterating them walks memory in order.

### Registry

Owns the entity slots, the free list and one pool per component type. A type gets a small consecutive id the first time it is used (`detail::component_id<T>()`), which indexes the array of pools, so looking up a pool is an array access. Pools are created on first `add`.

| Call | Effect |
|---|---|
| `create()` / `destroy(e)` / `valid(e)` | Entity lifetime. `destroy` removes every component of the entity. |
| `add<T>(e, args...)` | Builds T in place; plain structs are built with braces, so `add<Position>(e, 1.f, 2.f, 3.f)` works. |
| `get<T>(e)`, `try_get<T>(e)`, `has<T>(e)`, `all_of<Ts...>(e)`, `remove<T>(e)` | Component access. |
| `view<Ts...>()` | Every entity with all of `Ts`. |

### View

`view<A, B, C>().each(callback)` takes the pool with the fewest components among A, B and C, walks its entities and checks the others, so the cost depends on the rarest component. The callback receives `(Entity, A&, B&, C&)` or just `(A&, B&, C&)`. It iterates from the last element to the first, so the callback can remove components from, or destroy, the entity it is visiting.

### Tests

`Tests/` holds a minimal test runner (`TEST` / `CHECK`). `Test_Registry.cpp` has 17 tests: unique and valid handles, version reuse, `clear`, add/get/remove, swap-and-pop integrity, destroy removing every component, view membership, smallest-pool selection, destroying during iteration, and 10,000 entities with half destroyed. `Test_Systems.cpp` (phase 5) has 10 more for the transform hierarchy (parent rotation and scale, creation order, destroyed parents, cycles) and for `Movement_System`.

### Differences from the original assignment

The course's entity was an object holding `std::map<std::string, std::shared_ptr<Component>>`. Here an entity holds nothing: components of one type are packed together, they are found by type at compile time instead of by string, and systems iterate over the components they need instead of over every entity.

## Components and systems (phase 5)

Components are plain data in `Engine/code/headers/components/`:

| Component | Data | Used by |
|---|---|---|
| `Name` | Label | Logs, debug inspector (phase 8) |
| `Transform` | Position, rotation (quaternion), scale, `parent`; `world` matrix written by `Transform_System` | Every system that places things |
| `Velocity` | Linear (units/s) and angular (rad/s around local axes) | `Movement_System` |
| `Mesh_Renderer` | Shared `Mesh` and `Material`, `visible` | `Render_System` |
| `Camera_Component` | Field of view, near/far planes, `active` | `Render_System` |
| `Light_Component` | Colour and intensity of a point light | `Render_System` |

Each frame, after the game's tasks:

1. **`Movement_System`** (30) — `view<Transform, Velocity>`: moves and rotates.
2. **`Transform_System`** (50) — computes `world = parent's world × local` for every `Transform`. A parent is computed on demand the first time one of its children needs it, and each entity is computed once per frame (a stamp per entity index), so it does not matter in which order the pool holds them. An invalid parent (destroyed, or without a `Transform`) makes the child a root; cycles and chains deeper than 64 stop instead of hanging.
3. **`Render_System`** (90) — the only place where the ECS meets the renderer. It takes the first active `Transform + Camera_Component` (view matrix = inverse of its world matrix, without scale) and the first `Transform + Light_Component`, then submits every visible `Transform + Mesh_Renderer`.

All of them derive from `System`, a `Task` that holds the `Registry`. `Application` owns the registry and registers these systems, so a game only creates entities.

Destroying a parent does not destroy its children: they become roots. Destroying a whole branch is left to the game for now.

## Modules so far

| Module | Files | Responsibility |
|---|---|---|
| `core/Window` | `Window.hpp/.cpp` | Creates the SDL2 window and the OpenGL 3.3 Core context, loads the GL functions with GLAD, handles vsync, resize and buffer swaps. RAII: the destructor releases both handles. |
| `core/Log` | `Log.hpp` | `log_info`, `log_warning`, `log_error` over `SDL_Log` (console and Visual Studio Output window). |
| `renderer/Render_Command` | `Render_Command.hpp/.cpp` | Stateless per-frame GL calls: clear colour, clear, viewport. |
| `ecs/Entity` | `Entity.hpp` | Entity handle: index + version. |
| `ecs/Component_Pool` | `Component_Pool.hpp` | Sparse set for one component type. |
| `ecs/Registry` | `Registry.hpp/.cpp` | Entities, pools, component access, views. |
| `ecs/View` | `View.hpp` | Iteration over entities with a set of components. |
| `components/*` | `Transform`, `Velocity`, `Mesh_Renderer`, `Camera_Component`, `Light_Component`, `Name` | Engine components. |
| `systems/System` | `System.hpp` | Base class: a task with access to the registry. |
| `systems/Movement_System` | `Movement_System.hpp/.cpp` | Applies `Velocity`. |
| `systems/Transform_System` | `Transform_System.hpp/.cpp` | World matrices and hierarchy. |
| `systems/Render_System` | `Render_System.hpp/.cpp` | Camera, light and mesh entities → `Renderer`. |
| `core/Task` | `Task.hpp` | Base class of every per-frame task: name, priority, `initialize` / `execute(dt)` / `finalize`. Priority constants. |
| `core/Kernel` | `Kernel.hpp/.cpp` | Main loop, priority-sorted task list, deferred add/remove, delta time, FPS. |
| `core/Timer` | `Timer.hpp` | Elapsed seconds over `std::chrono::steady_clock`. |
| `core/Input` | `Input.hpp` | Keyboard and mouse state for the current frame. |
| `core/Application` | `Application.hpp/.cpp` | Owns SDL, window, resources, renderer, registry, input and kernel; registers the engine systems. |
| `systems/Input_System` | `Input_System.hpp/.cpp` | Fills `Input` from the SDL event queue. |
| `systems/Present_System` | `Present_System.hpp` | Swaps the window buffers at the end of the frame. |
| `renderer/*` | `Shader`, `Texture`, `Mesh`, `Model_Loader`, `Material`, `Camera`, `Renderer`, `Render_Command` | See the renderer section above. |
| `resources/Resource_Manager` | `Resource_Manager.hpp/.cpp` | Asset cache by path. |
| `Game` | `main.cpp`, `Demo_Scene`, `Fly_Camera` | Builds the test scene as entities and handles its input; the logic moves into scripts in phase 6. |
