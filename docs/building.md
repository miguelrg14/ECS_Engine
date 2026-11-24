# Building

## Requirements

- Windows 10/11 x64
- Visual Studio 2026 with the **Desktop development with C++** workload (platform toolset **v145**, MSVC 14.50)
- A GPU and driver with OpenGL 3.3 Core Profile support

## Steps

1. Clone the repository.
2. Open `ECS_Engine.sln`.
3. Pick the `x64` platform and `Debug` or `Release`.
4. Press **F5**. `Game` is the startup project; it references `Engine`, so the library builds first.

## Where things go

| Path | Content |
|---|---|
| `build/lib/<Configuration>/Engine.lib` | The engine static library |
| `build/<Project>/<Configuration>/` | Intermediate files (`.obj`, logs) |
| `binaries/<Configuration>/` | `ECS_Engine.exe`, `SDL2.dll` and a copy of `Game/assets/` |

Both `build/` and `binaries/` are ignored by Git. Visual Studio runs the game from `binaries/<Configuration>/`, the same folder the packaged executable runs from, so asset paths are always relative to the executable.

## Configurations

| | Debug | Release |
|---|---|---|
| Subsystem | Console (log output visible) | Windows (no console) |
| Optimization | Off | Full, with link-time code generation |
| OpenGL context | Debug flag on | Normal |

## Shared settings

Compiler settings live in [`props/Common.props`](../props/Common.props) and are imported by every project: C++20, `/W4`, `/permissive-`, `/utf-8`, multi-processor compilation and the include paths for `Engine` and `libraries/`. Headers included with `<...>` are treated as external, so SDL and GLM do not add warnings.

## Troubleshooting

| Problem | Fix |
|---|---|
| `The build tools for v145 cannot be found` | Install the C++ workload of Visual Studio 2026, or right-click the solution → **Retarget solution** to an installed toolset. |
| `SDL2.dll was not found` | Build again: the post-build step copies it next to the executable. |
| Message box: `OpenGL 3.3 is not available` | Update the GPU driver. Remote desktop sessions and some VMs only expose OpenGL 1.1. |
