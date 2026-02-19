#pragma once
#include <cstdint>

namespace ege {

enum class EventType : uint8_t {
    None = 0,
    Input,
    Sound,
    Animation,
};

enum class InputCode : uint32_t {
    None = 0,
    Quit = 1,
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
    [[nodiscard]] inline bool is_shutdown_event() const {
        return type == EventType::Input && id == uint32_t(InputCode::Quit);
    }
    
    [[nodiscard]] inline bool is_right_click() const {
        return type == EventType::Input && payload.i == 1 && id == 3; // right mouse button
    }
    
    [[nodiscard]] inline bool is_left_click() const {
        return type == EventType::Input && payload.i == 1 && id == 1; // left mouse button
    }
};



} // namespace ege
