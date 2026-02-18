#pragma once
#include <cstddef>
#include "../../../../src/engine/render_command.hpp"

namespace ege::backend {

struct ESP32Backend {
    ESP32Backend() = default;
    ~ESP32Backend() = default;

    bool init(std::size_t width, std::size_t height);
    void shutdown();
    void present(const ege::FrameBuffer<1024>& frame);
};

} // namespace ege::backend

// Link-time alias for embedded backend
namespace ege::backend { using Backend = ESP32Backend; }
