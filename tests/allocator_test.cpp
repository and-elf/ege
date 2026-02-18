#include <gtest/gtest.h>

#include <ege/engine/allocator.hpp>
#include <ege/engine/spsc_queue.hpp>

TEST(StaticArenaTest, BasicAllocation) {
    alignas(16) char buf[256];
    ege::StaticArena arena(buf, sizeof(buf));
    void* p1 = arena.allocate(64, 16);
    EXPECT_NE(p1, nullptr);
    void* p2 = arena.allocate(200, 8);
    EXPECT_EQ(p2, nullptr);
    arena.reset();
    EXPECT_EQ(arena.used(), 0u);
}

TEST(SPSCQueueTest, PushPop) {
    ege::SPSCQueue<int, 8> q;
    for (int i = 0; i < 7; ++i) EXPECT_TRUE(q.push(i));
    EXPECT_FALSE(q.push(99)); // full
    int v;
    for (int i = 0; i < 7; ++i) {
        EXPECT_TRUE(q.pop(v));
        EXPECT_EQ(v, i);
    }
    EXPECT_FALSE(q.pop(v)); // empty
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
