# EGE — Embedded Game Engine (draft)

This repository contains EGE, a small data-oriented embedded-friendly game engine core written in modern C++ (C++20).

**Project Layout**
- **Core public API:** [include/ege](include/ege)
- **Engine sources:** [src/engine](src/engine)
- **Backends:** [libs/backends](libs/backends) (per-backend libraries)
- **Examples:** [examples](examples)

**Layer Concept**
- Layers are now a C++20 concept rather than an inheritance-based interface.
- A type satisfies `LayerConcept` when it provides:
  - `bool on_event(const ege::Event&)` — return true if the event was consumed.
  - `void on_update(float dt)` — update step with delta-time in seconds.
  - `void on_render(int frame_count)` — record render commands. The runtime binds an active command-buffer
    into the layer as a protected `cmdbuf_` pointer before calling `on_render`; layers should push commands
    into `cmdbuf_` and must not call pipeline lifecycle methods directly.
- See the example layer in [examples/sdl/main.cpp](examples/sdl/main.cpp) for a minimal usage pattern.

**Runtime**
- `ege::Runtime` drives the main loop: it polls the backend for events, dispatches them to layers (top-first), calls updates, records render commands and presents frames.
- `Runtime::push_layer` accepts a pointer to a `ege::Layer` (inheritance-based).
- Public runtime API: [include/ege/runtime.hpp](include/ege/runtime.hpp)

Detailed `Runtime` docs

The `ege::Runtime` is the central driver of the engine. It is intentionally simple and designed for single-threaded use by default; backends and the render pipeline implement concurrency-friendly primitives (SPSC) when targeting multi-core SoCs.

Key behaviors
- Construction: `Runtime(backend::IBackend &backend, ege::SPSCRenderPipeline<1024,4,8>& pipeline)` — the runtime holds references to the backend and the producer-side pipeline.
- Layers: add layers via the templated `push_layer(L* layer)` where `L` satisfies the `LayerConcept`. The runtime stores lightweight callable wrappers and invokes them in a deterministic order.
- Event dispatch: each frame the runtime polls the backend for new `ege::Event`s and dispatches them to layers in reverse order (top-most layer first). If a layer returns `true` from `on_event`, the event is considered handled and propagation stops.
- Update step: after event dispatch the runtime calls `on_update(dt)` for each layer in insertion order (bottom-to-top). `dt` is a fixed-step by default (1/60s) but can be adapted later.
- Render: the runtime acquires a writable command-buffer each frame, binds it to each visible layer as `cmdbuf_`, calls `on_render(frame_count)` for those layers, then submits the buffer. After submission the runtime consumes the latest completed frame and calls the backend's `present()`.
- Stop: calling `Runtime::stop()` sets an internal flag and the main loop will exit cleanly at the next iteration.

Example usage

```cpp
#include <ege/backends/sdl/sdl_backend.hpp>
#include <ege/engine/render_pipeline.hpp>
#include <ege/runtime.hpp>

int main() {
  ege::backend::SDLBackend backend;
  if (!backend.init(320,240)) return 1;

  ege::SPSCRenderPipeline<1024,4,8> pipeline;
  ege::PhysicsSystem physics;
  ege::Runtime rt(backend, pipeline, physics);

  struct MyLayer {
    bool on_event(const ege::Event &e) { return false; }
    void on_update(float dt) { (void)dt; }
    void on_render(int fc) {
      // Runtime binds a valid command buffer pointer to the layer as
      // `cmdbuf_` before calling `on_render`. Layers should use the
      // protected `cmdbuf_` pointer to push commands and not touch the
      // pipeline directly.
      (void)fc;
      if (!cmdbuf_) return;
      cmdbuf_->push_clear(0xFF000000);
    }
  } layer;

  rt.push_layer(&layer);
  rt.run();
  backend.shutdown();
}
```

Notes & constraints
- `Runtime` is not thread-safe for concurrent modification of its layer list. Add/remove layers should happen from the same thread that runs `run()` or be synchronized externally.
- The runtime uses the backend's `poll_input`/`drain_events` to collect events; backends implement their own event queues (SPSC) to allow safe cross-thread signaling when necessary.
- The render pipeline used by `Runtime` here is a fixed-template instantiation; you can replace or templatize the pipeline sizes in your own builds as needed.

**Backends & SoC**
- Backends are organized as libraries under [libs/backends](libs/backends). Current backends:
  - `sdl` — Desktop SDL2 backend (system SDL required to build with `-DEGE_BUILD_SDL=ON`).
  - `esp32` — ESP32 backend stub for the Espressif ESP32 SoC (placeholder for hardware-specific implementation).
- When targeting embedded SoCs (for example, an Espressif ESP32), implement the platform-specific display/audio/input glue inside a backend library (follow the existing `libs/backends/esp32` stub layout).

**Building**
- Configure with CMake from the repository root. Example:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DEGE_BUILD_SDL=ON -DEGE_BUILD_ESP32=OFF ..
cmake --build . -- -j$(nproc)
ctest --output-on-failure -C Debug
```

**Notes & Next Steps**
- The ESP32 backend is a stub and requires platform toolchain and driver code to be useful on hardware.
- The public headers intentionally avoid leaking platform headers (SDL) into the public API; backends include platform headers in their implementation files.
# EGE — Embedded Game Engine

EGE is a small, retro-focused, data-oriented game engine written in modern C++.
It targets constrained embedded targets (ESP32 dual-core) and desktop native builds
via SDL. The engine emphasizes static allocation, predictable performance, and
explicit pipelines suitable for retro-style games and demos.

**Design goals**
- **Predictability:** static allocation and deterministic memory usage.
- **Data-Oriented:** contiguous arrays, SoA where helpful, and cache-friendly layouts.
- **Minimal dynamic allocation:** use preallocated arenas and pools.
- **Simple concurrency:** SPSC pipelines for render/framebuffer building across cores/threads.
- **Portable backends:** a thin rendering/audio/input abstraction with SDL and ESP32 backends.
- **Event-driven:** input, animation, and sound driven by a small event system.

Core concepts
- **Static allocation and arenas:** allocate and size memory up-front (compile-time or bootstrap).
- **Data-oriented entities/components:** avoid per-entity heap allocations; store component arrays.
- **SPSC render pipeline:** one producer builds render commands/framebuffer, the other consumes and presents.
- **Double-buffered framebuffer:** producer writes to back buffer while consumer presents front buffer.
- **Layers:** layered rendering (game world, UI layers) with automatic culling where possible.
- **Events:** lightweight event queue for input, sound, and animation to decouple systems.

Project layout (recommended)
- `cmake/` — CMake helper files and toolchain snippets.
- `src/engine/` — core engine code (allocator, ECS-lite structures, events, pipeline).
- `src/backends/` — platform backends (sdl/, esp32/).
- `examples/` — small demos and retro-game examples.
- `tests/` — unit and integration tests (use the cmake_template testing layout).

Technical notes
- Use modern C++ (C++20 minimum). Prefer `constexpr`, `span`, `concepts` and `std::atomic` for synchronization.
- Keep structures POD-like where possible to enable memcpy and predictable layout.
- Favor Structure-of-Arrays (SoA) for large component sets (positions, sprites, velocities).
- Use explicit sizes for arenas and pools; provide debug-time checks for overflow.

Render pipeline
- The render producer constructs a compact list of render commands (sprites, rectangles, text) into a preallocated command buffer.
- A single-producer single-consumer (SPSC) queue hands frame-ready command buffers or full framebuffers to the renderer task.
- Double-buffered framebuffer optionally used on targets that render pixel-by-pixel.
- Time-delta (dt) is calculated per-frame to drive animations and simulation; the render pipeline should be tolerant to variable dt.

Layers & culling
- Layers are simple ordered lists of render groups. Each group can have a small bounding rectangle used for coarse culling.
- UI layer sits on top with automatic visibility checks (frustum/rect intersection).

Events
- Engine provides a small, lock-free or single-writer event queue for input, sound, and animation events.
- Systems subscribe to event types and consume relevant events each tick.

Audio
- Abstract `AudioBackend` interface. On desktop use SDL_mixer or SDL Audio; on ESP32 use native I2S / DAC backend.
- Use event-driven sound triggers; audio system mixes channels into a preallocated buffer.

Input
- Abstract `InputBackend` providing polled and event-driven APIs. Map hardware-specific inputs to engine `InputEvent`s.

ESP32 considerations
- Use FreeRTOS tasks pinned to cores for producer/consumer roles. Prefer the dual-core setup: simulation + render on separate cores.
- Avoid dynamic allocation in high-frequency paths; use global arenas.
- Keep stack sizes conservative and check for stack usage.

Development workflow & testing
- Use https://github.com/cpp-best-practices/cmake_template as the basis for the project: compiler flags, testing, coverage, and packaging.
- Provide unit tests for allocator, event queue, SPSC pipeline, and culling logic.
- Continuous Integration should run tests and static analysis; coverage tools optional for desktop builds.
 - Use https://github.com/cpp-best-practices/cmake_template as the basis for the project: compiler flags, testing, coverage, and packaging.
 - Use GoogleTest (gtest) for unit testing. Prefer adding GoogleTest via CMake `FetchContent` so tests build reproducibly across CI environments. Example snippet:

```cmake
include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(testsuite tests/foo_test.cpp)
target_link_libraries(testsuite PRIVATE ege_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(testsuite)
```

- Continuous Integration should run tests and static analysis; coverage tools optional for desktop builds.


**Backends**

The engine exposes backend abstractions for rendering, audio and input. Backends live as separate libraries under `libs/backends/` and are intended to contain only the platform-specific glue (windowing, display drivers, audio output, and input mapping).

Each backend builds as an independent static library and installs/public-headers under `include/ege/backends/<name>`.

Per-backend documentation lives beside each backend. See:

- SDL backend: [libs/backends/sdl/README.md](libs/backends/sdl/README.md)
- ESP32 backend: [libs/backends/esp32/README.md](libs/backends/esp32/README.md)

Common guidance
- When adding a new backend, follow the existing layout: `libs/backends/<name>/{include,src,CMakeLists.txt}` and expose a minimal public header under `include/ege/backends/<name>/*.hpp` that implements the `ege::backend::IBackend` interface.
- Keep platform headers out of public engine headers; include them only in the backend implementation `.cpp` to avoid forcing consumers to install platform SDKs unless they enable that backend.
- Prefer small, testable backend implementations and add integration tests that run only on CI or when the appropriate SDK is present.

API & coding conventions
- Prefer descriptive names and small translation units. Keep APIs explicit and minimal.
- Use `concept`s to express requirements for renderables and backends.
- Avoid exceptions in embedded builds; provide `EGE_ASSERT` and error codes.
 - Annotate functions that return resources or indicate failure with `[[nodiscard]]` so callers don't accidentally ignore important return values (for example: allocators, push/pop on queues, and factory functions).
 - Use `.hpp`/`.cpp` pairs for headers and implementation. Prefer the `.hpp` extension for engine headers, keep headers minimal (declarations, inline helpers, and templates) and put all non-trivial implementation into `.cpp` files.

Roadmap (initial milestones)
1. Minimal README and project plan (this file).
2. Scaffold CMake project using `cmake_template` and create core library target.
3. Implement static arena allocator and tests.
4. Implement compact render command buffer and SPSC queue.
5. SDL backend: simple window, palette-based framebuffer, present loop.
6. ESP32 backend prototype: framebuffer and display driver integration.
7. Event system: input, audio triggers, animation events.
8. Layers, culling, and sample games/demos.
9. Tests, CI, and documentation.

Quick start
1. Clone this repository and follow the `cmake_template` instructions to scaffold toolchain and build.
2. Build desktop example with SDL for rapid iteration.
3. Run tests and iterate on small systems (allocator, SPSC queue) first.

Contributing
- Open issues for features and design proposals. Small, focused PRs are preferred.
- Include tests for behavioral changes and keep API changes minimal.

License
- Choose a permissive license (MIT/Apache-2.0) for the engine. Specify in `LICENSE`.

Contact
- Start issues and design discussions in this repository's issue tracker.

----
This README is a starting point. If you want, I can now scaffold the CMake project using
the cpp-best-practices `cmake_template`, add initial build targets, and create the first
unit tests for the arena allocator and SPSC queue.
