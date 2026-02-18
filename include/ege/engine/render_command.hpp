#pragma once
#include <cstdint>
#include <array>
#include <cassert>

namespace ege {

enum class RenderCommandType : uint8_t {
    Clear = 0,
    Rect,
    Sprite,
};

struct RenderCommand {
    RenderCommandType type;
    uint32_t layer;
    uint32_t color; // simple packed color or palette index
    struct {
        int16_t x, y;
        int16_t w, h;
    } rect;
    // sprite metadata could be added later
};

template<std::size_t MaxCommands>
struct FrameBuffer {
    std::array<RenderCommand, MaxCommands> commands{};
    std::size_t count = 0;

    void push(const RenderCommand& cmd) noexcept {
        assert (count < MaxCommands);
        commands[count++] = cmd;
    }

    void reset() noexcept { count = 0; }
    [[nodiscard]] std::size_t size() const noexcept { return count; }
};

} // namespace ege
