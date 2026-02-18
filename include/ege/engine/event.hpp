#pragma once
#include <cstdint>

namespace ege {

enum class EventType : uint8_t {
    None = 0,
    Input,
    Sound,
    Animation,
};

struct Event {
    EventType type;
    uint32_t id;
    union {
        int32_t i;
        float f;
        void* p;
    } payload;
    struct Position { int32_t x = 0; int32_t y = 0; } pos;
};

enum class InputCode : uint32_t {
    None = 0,
    Quit = 1,
};

} // namespace ege
