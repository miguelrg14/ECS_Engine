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

## Phase 1 — what exists now

| Module | Files | Responsibility |
|---|---|---|
| `core/Window` | `Window.hpp/.cpp` | Creates the SDL2 window and the OpenGL 3.3 Core context, loads the GL functions with GLAD, handles vsync, resize and buffer swaps. RAII: the destructor releases both handles. |
| `core/Log` | `Log.hpp` | `log_info`, `log_warning`, `log_error` over `SDL_Log` (console and Visual Studio Output window). |
| `renderer/Render_Command` | `Render_Command.hpp/.cpp` | Stateless per-frame GL calls: clear colour, clear, viewport. |
| `Game` | `main.cpp` | Temporary main loop: polls events, clears the screen, swaps buffers. It moves into the kernel in phase 2. |
