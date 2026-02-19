#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>
#include "render_command.hpp"

namespace ege {

// Compact, memory-backed command buffer. Fixed-size byte buffer that stores
// binary-encoded render commands to minimize memory overhead and improve cache.
template<std::size_t Capacity>
class MemoryCommandBuffer {
public:
    MemoryCommandBuffer() noexcept : writable_(true) { reset(); }
    explicit MemoryCommandBuffer(bool writable) noexcept : writable_(writable) { reset(); }

    void reset() noexcept { size_ = 0; }

    [[nodiscard]] std::size_t capacity() const noexcept { return Capacity; }
    [[nodiscard]] std::size_t size() const noexcept { return size_; }

    // Push a rectangle command. Crashes if there's not enough room.
    void push_rect(uint8_t layer, uint32_t color, int16_t x, int16_t y, int16_t w, int16_t h) noexcept {
        // layout: opcode(1) | layer(1) | color(4) | x(2) | y(2) | w(2) | h(2)
        assert(writable_ && "attempt to write to read-only command buffer");
        constexpr std::size_t needed = 1 + 1 + 4 + 2 + 2 + 2 + 2;
        assert(size_ + needed <= Capacity);
        uint8_t* ptr = &buf_[size_];
        ptr[0] = static_cast<uint8_t>(RenderCommandType::Rect);
        ptr[1] = layer;
        std::memcpy(ptr + 2, &color, 4);
        std::memcpy(ptr + 6, &x, 2);
        std::memcpy(ptr + 8, &y, 2);
        std::memcpy(ptr +10, &w, 2);
        std::memcpy(ptr +12, &h, 2);
        size_ += needed;
    }

    // Push a clear command. Crashes if there's not enough room.
    void push_clear(uint32_t color) noexcept {
        // opcode(1) | color(4)
        assert(writable_ && "attempt to write to read-only command buffer");
        constexpr std::size_t needed = 1 + 4;
        assert(size_ + needed <= Capacity);
        uint8_t* ptr = &buf_[size_];
        ptr[0] = static_cast<uint8_t>(RenderCommandType::Clear);
        std::memcpy(ptr + 1, &color, 4);
        size_ += needed;
    }

    // Decode into a FrameBuffer (caller supplies target). Returns number of commands decoded.
    template<std::size_t MaxCommands>
    std::size_t decode(FrameBuffer<MaxCommands>& out) const noexcept {
        out.reset();
        std::size_t p = 0;
        while (p < size_) {
            uint8_t opcode = buf_[p];
            if (opcode == static_cast<uint8_t>(RenderCommandType::Clear)) {
                if (p + 1 + 4 > size_) break; // malformed
                uint32_t color;
                std::memcpy(&color, &buf_[p + 1], 4);
                RenderCommand rc{};
                rc.type = RenderCommandType::Clear;
                rc.color = color;
                out.push(rc);
                p += 1 + 4;
            } else if (opcode == static_cast<uint8_t>(RenderCommandType::Rect)) {
                constexpr std::size_t chunk = 1 + 1 + 4 + 2 + 2 + 2 + 2;
                if (p + chunk > size_) break; // malformed
                uint8_t layer = buf_[p + 1];
                uint32_t color;
                int16_t x,y,w,h;
                std::memcpy(&color, &buf_[p + 2], 4);
                std::memcpy(&x, &buf_[p + 6], 2);
                std::memcpy(&y, &buf_[p + 8], 2);
                std::memcpy(&w, &buf_[p +10], 2);
                std::memcpy(&h, &buf_[p +12], 2);
                RenderCommand rc{};
                rc.type = RenderCommandType::Rect;
                rc.layer = layer;
                rc.color = color;
                rc.rect = {x,y,w,h};
                out.push(rc);
                p += chunk;
            } else {
                // unknown opcode, stop
                break;
            }
        }
        return out.size();
    }

private:
    uint8_t buf_[Capacity];
    std::size_t size_ = 0;
    bool writable_ = true;
};

} // namespace ege
