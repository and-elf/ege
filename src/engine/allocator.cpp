#include <ege/engine/allocator.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace ege {

StaticArena::StaticArena(void* buffer, std::size_t size) noexcept
  : m_start(reinterpret_cast<uint8_t*>(buffer)),
    m_ptr(reinterpret_cast<uint8_t*>(buffer)),
    m_end(reinterpret_cast<uint8_t*>(buffer) + size)
{
}

void* StaticArena::allocate(std::size_t size, std::size_t align) noexcept
{
    std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(m_ptr);
    std::size_t offset = (align - (curr % align)) % align;
    if (m_ptr + offset + size > m_end) return nullptr;
    uint8_t* result = m_ptr + offset;
    m_ptr = result + size;
    return result;
}

void StaticArena::reset() noexcept
{
    m_ptr = m_start;
}

std::size_t StaticArena::capacity() const noexcept { return static_cast<std::size_t>(m_end - m_start); }
std::size_t StaticArena::used() const noexcept { return static_cast<std::size_t>(m_ptr - m_start); }

} // namespace ege
