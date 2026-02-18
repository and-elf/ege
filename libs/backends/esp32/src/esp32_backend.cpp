#include <ege/backends/esp32/esp32_backend.hpp>
#include <iostream>

namespace ege::backend {

bool ESP32Backend::init(std::size_t width, std::size_t height) {
    (void)width; (void)height;
    std::cerr << "ESP32 backend (stub) initialized.\n";
    return true;
}

void ESP32Backend::shutdown() {
    std::cerr << "ESP32 backend (stub) shutdown.\n";
}

void ESP32Backend::present(const ege::FrameBuffer<1024>& frame) {
    std::cerr << "ESP32 backend present (stub): commands=" << frame.size() << "\n";
    (void)frame;
}

} // namespace ege::backend
