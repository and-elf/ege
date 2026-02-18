#pragma once
#include <cstddef>
#include <cstdint>

namespace ege {

class StaticArena {
public:
    StaticArena(void* buffer, std::size_t size) noexcept;
    [[nodiscard]] void* allocate(std::size_t size, std::size_t align = alignof(std::max_align_t)) noexcept;
    void reset() noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::size_t used() const noexcept;

private:
    uint8_t* m_start;
    uint8_t* m_ptr;
    uint8_t* m_end;
};

} // namespace ege
