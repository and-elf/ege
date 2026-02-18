#include <gtest/gtest.h>

#include <ege/engine/command_buffer.hpp>
#include <ege/engine/render_command.hpp>

TEST(CommandBufferTest, EncodeDecode) {
    ege::MemoryCommandBuffer<256> buf;
    EXPECT_EQ(buf.size(), 0u);

    buf.push_clear(0x11223344);
    buf.push_rect(2, 0xFF, 1,2,3,4);
    buf.push_rect(1, 0xAA, -5,-6,7,8);

    ege::FrameBuffer<16> out;
    auto count = buf.decode(out);
    EXPECT_EQ(count, 3u);
    EXPECT_EQ(out.commands[0].type, ege::RenderCommandType::Clear);
    EXPECT_EQ(out.commands[0].color, 0x11223344u);
    EXPECT_EQ(out.commands[1].type, ege::RenderCommandType::Rect);
    EXPECT_EQ(out.commands[1].layer, 2u);
    EXPECT_EQ(out.commands[1].rect.x, 1);
    EXPECT_EQ(out.commands[2].rect.x, -5);
}
