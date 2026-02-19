#pragma once
#include "spsc_queue.hpp"
#include "render_command.hpp"
#include "command_buffer.hpp"
#include <cassert>
#include <cstdint>
#include <optional>
#include <functional>

namespace ege {

// SPSCRenderPipeline that uses MemoryCommandBuffer as the producer-side buffer.
// Producer writes binary-encoded commands into an internal back buffer, then
// submits the full buffer to the SPSC queue. Consumer pops the binary buffer
// and decodes it into a FrameBuffer for rendering.
template<std::size_t CmdCapacity, std::size_t BufferCount = 4, std::size_t QueueCapacity = 8>
class SPSCRenderPipeline {
public:
    static_assert(BufferCount >= 2, "BufferCount should be at least 2");
    using CmdBuf = MemoryCommandBuffer<CmdCapacity>;
    using Frame = FrameBuffer<256>;

    SPSCRenderPipeline() noexcept {
        for (uint32_t i = 0; i < BufferCount; ++i) {
            (void)free_idx_q_.push(i);
        }
    }

    // Producer: acquire a free buffer to write into. Returns a reference to a buffer.
    // If no buffers are available, returns a reference to an internal empty buffer.
    using OptionalCmdBufRef = std::optional<std::reference_wrapper<CmdBuf>>;

    [[nodiscard]] OptionalCmdBufRef begin_frame() noexcept {
        uint32_t idx{};
        if (!free_idx_q_.pop(idx)) return std::nullopt;
        assert(idx_is_valid(idx));
        buffers_[idx].reset();
        current_write_idx_ = idx;
        return OptionalCmdBufRef{std::ref(buffers_[idx])};
    }

    // Submit the previously acquired buffer for consumption.
    void submit_frame() noexcept {
        // begin_frame() must have been called and set a valid index before submitting
        if(!idx_is_valid(current_write_idx_)) return;
        if (!used_idx_q_.push(current_write_idx_)) return;
        current_write_idx_ = UINT32_MAX;
    }

    bool idx_is_valid(uint32_t idx) const noexcept {
        return idx < BufferCount;
    }

    bool idx_sentinel(uint32_t idx) const noexcept {
        return idx == UINT32_MAX;
    }

    // Consumer: try to pop a used buffer index and return a const reference to it.
    // If none available, returns a reference to the internal empty buffer and sets out_idx to UINT32_MAX.
    [[nodiscard]] const CmdBuf& try_consume(uint32_t &out_idx) noexcept {
        uint32_t idx{};
        if (!used_idx_q_.pop(idx)) { out_idx = UINT32_MAX; return empty_buf_; }
        if (idx_sentinel(idx)) { out_idx = UINT32_MAX; return empty_buf_; }
        assert(idx_is_valid(idx));
        out_idx = idx;
        return buffers_[idx];
    }

    // Return a reference to the current write buffer if `begin_frame()` has
    // been called and a valid buffer is selected. This is a convenience for
    // producer-side code that expects the runtime to own begin/submit.
    [[nodiscard]] OptionalCmdBufRef current_cmdbuf_ref() noexcept {
        if (!idx_is_valid(current_write_idx_)) return std::nullopt;
        return OptionalCmdBufRef{std::ref(buffers_[current_write_idx_])};
    }

    // After consuming/processing the buffer, return it to the free pool.
    [[nodiscard]] bool release_buffer(uint32_t idx) noexcept {
        if (idx_sentinel(idx)) return false;
        assert(idx_is_valid(idx));
        return free_idx_q_.push(idx);
    }

    // Convenience: consume and decode into target; returns true if a frame was decoded.
    [[nodiscard]] bool consume_and_decode(Frame& target) noexcept {
        uint32_t idx{};
        const CmdBuf& b = try_consume(idx);
        if (idx_sentinel(idx)) return false;
        b.decode(target);
        release_buffer(idx);
        return true;
    }

private:
    CmdBuf buffers_[BufferCount];
    CmdBuf empty_buf_{false};
    SPSCQueue<uint32_t, QueueCapacity> free_idx_q_;
    SPSCQueue<uint32_t, QueueCapacity> used_idx_q_;
    uint32_t current_write_idx_ = UINT32_MAX;
};

} // namespace ege
