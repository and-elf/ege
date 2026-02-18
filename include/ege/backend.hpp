#pragma once
#include <vector>
#include <cstddef>
#include "ege/engine/render_command.hpp"
#include "ege/engine/command_buffer.hpp"
#include "ege/engine/event.hpp"

namespace ege { namespace backend {
template<typename L>
concept BackendConcept = requires(L l, std::size_t w, std::size_t h, const ege::FrameBuffer<1024>& frame, std::vector<ege::Event>& events, int sample_rate, uint32_t sound_id, float frequency, uint32_t duration_ms, ege::Event &out) {
    { l.init(w, h) } -> std::convertible_to<bool>;
    { l.shutdown() } -> std::same_as<void>;
    { l.present(frame) } -> std::same_as<void>;
    { l.poll_input(events) } -> std::same_as<void>;
    { l.open_audio(sample_rate) } -> std::convertible_to<bool>;
    { l.trigger_sound(sound_id, frequency, duration_ms) } -> std::same_as<void>;
    { l.try_pop_event(out) } -> std::convertible_to<bool>;
    { l.drain_events(events) } -> std::same_as<void>;
};

} }
