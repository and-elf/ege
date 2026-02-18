# SDL Backend

Location: `libs/backends/sdl`

This backend provides a desktop implementation using SDL2. It implements the `ege::backend::IBackend` interface and performs window creation, texture presentation, audio output and input polling.

Build
- Enable with CMake: `-DEGE_BUILD_SDL=ON`.
- Requirements: SDL2 development headers (Debian/Ubuntu: `sudo apt install libsdl2-dev`).

Public API
- Public header: `include/ege/backends/sdl/sdl_backend.hpp` — includes a small `ege::backend::SDLBackend` type that implements `IBackend`.

Usage
- Example (see `examples/sdl/main.cpp`): create `ege::backend::SDLBackend`, call `init(width,height)`, pass to `ege::Runtime`, and use the runtime loop.

Notes
- The backend includes `SDL.h` only in its implementation `.cpp` to avoid forcing consumers to install SDL unless they enable the backend.
- If configure fails due to missing SDL, either install system SDL or disable `EGE_BUILD_SDL`.
