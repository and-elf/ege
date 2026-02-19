#pragma once
#include <atomic>
#include <cstddef>
#include <type_traits>
#include <cassert>

namespace ege {

template<typename T, std::size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of two");
public:
    SPSCQueue() noexcept : head_(0), tail_(0) {}

    [[nodiscard]] bool push(const T& v) noexcept {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = (head + 1) & (Capacity - 1);
        if (next == tail_.load(std::memory_order_acquire)) return false; // full
        buffer_[head] = v;
        head_.store(next, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool pop(T& out) noexcept {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        const std::size_t last = (tail + 1) & (Capacity - 1);

        if (tail == head_.load(std::memory_order_acquire)) return false; // empty
        out = buffer_[tail];
        tail_.store(last, std::memory_order_release);
        return true;
    }

private:
    T buffer_[Capacity];
    std::atomic<std::size_t> head_;
    std::atomic<std::size_t> tail_;
};

} // namespace ege
