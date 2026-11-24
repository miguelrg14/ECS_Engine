# ECS Engine

A small game engine written from scratch in **C++20 and modern OpenGL 3.3 (Core Profile)**, built around a data-oriented **Entity Component System**.

It revisits the final assignment of the *Graphics Engines and Plugins* course (Game Design & Development degree): a task kernel, an entity/component subsystem and a messaging subsystem, split into an `Engine` static library and a `Game` executable. This version replaces the object-oriented entity model with a real ECS (entities as IDs, components stored contiguously per type, systems that iterate over views) and draws with my own renderer from [VS_3D-Renderer](https://github.com/miguelrg14/VS_3D-Renderer).

> **Status:** phase 7 of 8. The demo is a small playable game: collect five orbs in an arena before the time runs out while patrolling drones try to catch you. Walls, crates, orbs and drones are box colliders handled by the engine's collision system. See the [roadmap](#roadmap).

<p align="center">
  <img src="docs/images/07-orb-arena.png" alt="Phase 7: the orb arena seen from behind the player, with wooden walls, crate stairs in the far corner, patrolling drones, a glowing orb and the floating checker cube" width="800">
  <br><em>Phase 7 · the orb arena: walls and crates block, orbs and drones are triggers</em>
</p>

<p align="center">
  <img src="docs/images/05-ecs-scene.png" alt="Phase 5: the test scene as entities, with a field of 1,000 spinning coloured cubes behind an umbrella, a potted plant and a UV-checker cube with an orbiting child" width="800">
  <br><em>Phase 5 · the 1,000-cube stress test (key C)</em>
</p>

---

## Roadmap

| Phase | Goal | Status |
|---|---|---|
| 1 · Solution skeleton | `Engine` static library + `Game` executable, SDL2 window, OpenGL 3.3 context through GLAD | ✅ Done |
| 2 · Kernel, timer, input | Priority-ordered systems with `initialize` / `execute(dt)` / `finalize`, frame timer, keyboard and mouse state | ✅ Done |
| 3 · Renderer module | Shaders, meshes, textures and materials ported from VS_3D-Renderer, resource cache, .obj models | ✅ Done |
| 4 · ECS core | Sparse-set component pools, `Registry`, `view<A, B>()`, unit tests | ✅ Done |
| 5 · Core components and systems | Transform hierarchy, camera, lights, render system | ✅ Done |
| 6 · Scripts and events | Native scripts, typed event bus, player controller | ✅ Done |
| 7 · Collisions and demo | AABB colliders and triggers, small playable demo | ✅ Done |
| 8 · Debug tools and release | Dear ImGui entity inspector, docs, packaged build | ⏳ |

---

## The ECS at a glance

```cpp
engine::Registry registry;

engine::Entity player = registry.create ();
registry.add< Position >(player, 0.f, 1.f, 0.f);
registry.add< Velocity >(player, 2.f, 0.f, 0.f);

// Every entity with both components; the rarest one decides the cost.
registry.view< Position, Velocity >().each ([dt] (Position & position, Velocity & velocity)
{
    position.x += velocity.x * dt;
});
```

A scene object is an entity with the engine's components; the systems do the rest:

```cpp
engine::Entity cube = registry.create ();
registry.add< engine::Transform     >(cube).position = { 0.f, 1.2f, 0.f };
registry.add< engine::Mesh_Renderer >(cube, cube_mesh, checker_material);
registry.add< engine::Velocity      >(cube, glm::vec3(0.f), glm::vec3(0.f, 0.8f, 0.f));   // spins

engine::Entity moon = registry.create ();
registry.add< engine::Transform     >(moon).parent = cube;                                  // orbits with it
```

Behaviour goes in scripts, and scripts communicate through events without knowing each other:

```cpp
struct Landed { engine::Entity entity; float impact_speed; };          // any struct is an event

class Player_Controller : public engine::Script
{
    void on_update (float dt) override
    {
        // ... move, jump, fall ...
        if (just_landed) publish (Landed{ entity (), fall_speed });
    }
};

class Squash_And_Stretch : public engine::Script
{
    void on_create () override
    {
        subscribe< Landed >([this] (const Landed & event) { if (event.entity == entity ()) squash (event.impact_speed); });
    }
};

registry.add< engine::Script_Component >(player).bind< Player_Controller >(camera);
registry.get< engine::Script_Component >(player).bind< Squash_And_Stretch >();
```

Collisions are components too. A wall is a static box; a pickup is a trigger whose overlap arrives as an event:

```cpp
registry.add< engine::Box_Collider >(wall);                                         // static, solid
registry.add< engine::Box_Collider >(orb, glm::vec3(1.2f), glm::vec3(0.f), true);   // trigger

subscribe< engine::Trigger_Enter >([this] (const engine::Trigger_Enter & event)
{
    if (event.other == player && registry ().has< Orb >(event.trigger)) collect (event.trigger);
});
```

Entities are 32-bit IDs with a generation counter, so a handle to a destroyed entity is detected instead of reaching the entity that reused its slot. Each component type is stored in its own sparse set: add, remove and lookup are O(1), and components sit contiguously in memory. Details in [docs/architecture.md](docs/architecture.md#ecs-core-phase-4).

---

## Project layout

```text
ECS_Engine/
├── ECS_Engine.sln
├── props/Common.props        # settings shared by every project (C++20, /W4, paths)
├── Engine/                   # static library
│   ├── project/Engine.vcxproj
│   └── code/
│       ├── headers/          # core/, ecs/, renderer/, resources/, systems/
│       └── sources/
├── Game/                     # executable built on top of the engine
│   ├── project/Game.vcxproj
│   ├── code/
│   └── assets/
├── Tests/                    # unit tests of the engine, run on every build
│   ├── project/Tests.vcxproj
│   └── code/
├── libraries/                # third-party headers and prebuilt libraries
└── docs/
```

More detail in [docs/architecture.md](docs/architecture.md).

---

## Tech stack

| Library | Use |
|---|---|
| [SDL2](https://www.libsdl.org/) 2.30.9 | Window, OpenGL context, input |
| [GLAD](https://glad.dav1d.de/) 0.1.36 | OpenGL 3.3 Core function loader (compiled from source into `Engine`) |
| [GLM](https://github.com/g-truc/glm) 1.0.1 | Vectors, matrices, transforms |
| [stb_image](https://github.com/nothings/stb) 2.34 | PNG / JPG / TGA / BMP decoding (header-only) |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) 2.0.0rc13 | Wavefront .obj loading (header-only) |

All dependencies are **included in [`libraries/`](libraries/)**, so there is nothing to install. The SDL2 binaries are small enough to live in the repository directly, so Git LFS is not needed.

---

## Quick start

**Requirements:** Windows 10/11 x64, Visual Studio 2026 with the *Desktop development with C++* workload (platform toolset v145), and a GPU that supports OpenGL 3.3.

```text
1. git clone https://github.com/miguelrg14/ECS_Engine.git
2. Open ECS_Engine.sln
3. Select x64 and Debug or Release
4. Press F5
```

`Game` is the startup project and depends on `Engine`, so both build in order. Building the solution also builds `Tests` and runs it: a failing test fails the build. The executable is written to `binaries/<Configuration>/ECS_Engine.exe` together with `SDL2.dll` and `assets/`.

See **[docs/building.md](docs/building.md)** for details and troubleshooting.

### Controls

The current build is the phase 7 demo. **Goal:** collect the 5 orbs within 90 seconds. Touching a drone sends you back to the start. One orb waits on top of the crate stairs.

| Input | Action |
|---|---|
| `W` `A` `S` `D` | Move the character (relative to the camera) |
| `Shift` (hold) | Run |
| `Space` | Jump |
| Right mouse (hold) + move | Orbit the camera around the character |
| Mouse wheel | Zoom |
| `R` | Start a new round |
| `Tab` | Switch to the free fly camera and back (fly: `W` `A` `S` `D` + `Q` / `E`, right mouse to look) |
| `C` | Add / remove a field of 1,000 spinning cubes |
| `L` | Pause / resume the light |
| `F` | Toggle fog |
| `X` | Toggle wireframe |
| `V` | Toggle vsync |
| `Esc` | Quit |

The window title shows the orbs collected, the time left (or the result), FPS, entities and colliders.

---

## Credits

- Umbrella and plant models and their textures, wood and UV-checker textures: from my [VS_3D-Renderer](https://github.com/miguelrg14/VS_3D-Renderer) assignment.
- The lit shader is a port of the Blinn-Phong and fog shader from the same project.

## License

My code is released under the [MIT License](LICENSE). Third-party libraries in `libraries/` keep their own licenses, included next to each one.
