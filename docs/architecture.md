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
libraries/               SDL2 · GLAD · GLM (Assimp, SOIL2, Dear ImGui later)
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

Sandbox sandbox (application);              // a Task defined in Game/

application.get_kernel ().add (sandbox);
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
| 20 | Scripts (the phase 2 `Sandbox` runs here) | 2, 6 |
| 30–60 | Movement, collision, transform, events | 5–7 |
| 90 | `Render_System` | 5 |
| 100 | Debug UI | 8 |
| 1000 | `Present_System` — swaps the window buffers | 2 |

`Input` stores the current and previous state of every key (by scancode, so WASD keeps its place on any layout) and mouse button. That gives three queries per key: held (`is_key_down`), went down this frame (`was_key_pressed`) and went up this frame (`was_key_released`). Losing focus clears the state, because keys released outside the window never send a key-up event.

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
| `core/Application` | `Application.hpp/.cpp` | Owns SDL, window, input and kernel; registers the engine tasks. |
| `systems/Input_System` | `Input_System.hpp/.cpp` | Fills `Input` from the SDL event queue. |
| `systems/Present_System` | `Present_System.hpp` | Swaps the window buffers at the end of the frame. |
| `Game` | `main.cpp`, `Sandbox.hpp/.cpp` | Creates the application and a test task that uses timing and input. |
