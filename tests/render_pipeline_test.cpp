#include <gtest/gtest.h>

#include <ege/engine/render_pipeline.hpp>
#include <ege/engine/render_command.hpp>

TEST(RenderPipelineTest, ProduceConsume) {
    using Pipeline = ege::SPSCRenderPipeline<256, 4>;
    Pipeline pipeline;

    // Producer side: acquire buffer and encode commands
    auto &buf = pipeline.begin_frame();
    // If no buffers are available, begin_frame returns an empty buffer reference.
    buf.push_rect(1, 0xFF, 0, 0, 8, 8);
    buf.push_clear(0x12345678);
    pipeline.submit_frame();

    // Consumer side: pop index and decode
    uint32_t idx;
    const auto &popped = pipeline.try_consume(idx);
    EXPECT_NE(idx, UINT32_MAX);
    ege::FrameBuffer<16> out;
    auto n = popped.decode(out);
    EXPECT_EQ(n, 2u);
    EXPECT_EQ(out.commands[0].type, ege::RenderCommandType::Rect);
    EXPECT_EQ(out.commands[1].type, ege::RenderCommandType::Clear);
    // release buffer back to pool
    EXPECT_TRUE(pipeline.release_buffer(idx));
}
