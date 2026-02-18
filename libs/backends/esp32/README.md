# ESP32 Backend (stub)

Location: `libs/backends/esp32`

This directory contains a scaffolded ESP32 backend intended to be implemented against the ESP-IDF or another ESP32 SDK. The repo ships a native-build stub so the engine can build on desktop without the ESP toolchain.

Build
- The stub builds by default as a small static library. To build a real ESP32 backend you will need to enable `-DEGE_BUILD_ESP32=ON` and provide a suitable toolchain file or CI job that configures the ESP-IDF environment.

Integration notes
- Typical steps to target ESP32 hardware:
  1. Add or point to an `esp-idf` toolchain via `CMAKE_TOOLCHAIN_FILE` or use ESP-IDF's build system integration.
  2. Implement or adapt a display driver (e.g., ILI9341/ST7789) that can accept framebuffers or DMA transfers.
  3. Provide audio output using I2S or DAC and map hardware inputs (GPIO, buttons) to `ege::Event` structures.

Public API
- Public header: `include/ege/backends/esp32/esp32_backend.hpp` — implements `ege::backend::IBackend` in the repo as a stub.

Notes
- Cross-compilation requires careful toolchain and SDK configuration; it's recommended to create CI tasks that run the toolchain or provide a reproducible container with the ESP-IDF installed.
