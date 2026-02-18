#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <ege/engine/render_command.hpp>
#include <ege/engine/spsc_queue.hpp>
#include <ege/engine/event.hpp>
#include <cstdint>
// Forward-declare SDL types to avoid forcing consumers to have SDL headers in their include path.
extern "C" {
    struct SDL_Window;
    struct SDL_Renderer;
    struct SDL_Texture;
}
using SDL_AudioDeviceID = uint32_t;

namespace ege::backend {

struct SDLBackend {
    SDLBackend();
    ~SDLBackend();

    bool init(std::size_t width, std::size_t height);
    void shutdown();
    void present(const ege::FrameBuffer<1024>& frame);
    // Input/audio bridging
    void poll_input(std::vector<ege::Event>& out);
    bool open_audio(int sample_rate = 44100);
    // Trigger a simple synthesized sound (frequency in Hz, duration in seconds).
    void trigger_sound(uint32_t sound_id, float frequency = 440.0f, float duration = 0.2f);
    // Pull APIs for event queue
    bool try_pop_event(ege::Event &out);
    void drain_events(std::vector<ege::Event>& out);

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    std::size_t width_ = 0;
    std::size_t height_ = 0;
    std::vector<uint32_t> pixels_; // ARGB8888
    SDL_AudioDeviceID audio_dev_ = 0;
    int audio_rate_ = 0;
    // internal single-producer single-consumer queue for events
    ege::SPSCQueue<ege::Event, 1024> event_queue_;
};

} // namespace ege::backend

// Link-time alias: when building the SDL backend library, it exposes `ege::backend::Backend`.
namespace ege::backend { using Backend = SDLBackend; }
