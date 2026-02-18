#include <iostream>
#include <ege/engine/allocator.hpp>

int main() {
    std::cout << "EGE example: core library scaffolded.\n";
    // Create a small arena on the stack to exercise the allocator.
    alignas(16) char buf[1024];
    ege::StaticArena arena(buf, sizeof(buf));
    void* p = arena.allocate(64, 16);
    if (p) std::cout << "Allocated 64 bytes from arena. used=" << arena.used() << "\n";
    return 0;
}
