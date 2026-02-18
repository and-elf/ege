#include <ege/backends/sdl/sdl_backend.hpp>
#include <SDL.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <vector>

namespace ege::backend {

SDLBackend::SDLBackend() = default;
SDLBackend::~SDLBackend() { shutdown(); }

bool SDLBackend::init(std::size_t width, std::size_t height) {
    width_ = width;
    height_ = height;
    pixels_.assign(width_ * height_, 0u);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    window_ = SDL_CreateWindow("EGE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               static_cast<int>(width_), static_cast<int>(height_), 0);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return false;
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 static_cast<int>(width_), static_cast<int>(height_));
    if (!texture_) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        renderer_ = nullptr;
        window_ = nullptr;
        SDL_Quit();
        return false;
    }

    return true;
}

void SDLBackend::shutdown() {
    if (audio_dev_ != 0) {
        SDL_CloseAudioDevice(audio_dev_);
        audio_dev_ = 0;
    }
    if (texture_) { SDL_DestroyTexture(texture_); texture_ = nullptr; }
    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
    SDL_Quit();
}

static inline void draw_filled_rect(std::vector<uint32_t> &pixels, std::size_t w, std::size_t h,
                                    int x0, int y0, int rw, int rh, uint32_t color) {
    int x1 = x0 + rw;
    int y1 = y0 + rh;
    x0 = std::max(0, x0);
    y0 = std::max(0, y0);
    x1 = std::min((int)w, x1);
    y1 = std::min((int)h, y1);
    for (int y = y0; y < y1; ++y) {
        uint32_t *row = pixels.data() + static_cast<std::size_t>(y) * w;
        for (int x = x0; x < x1; ++x) row[x] = color;
    }
}

void SDLBackend::present(const ege::FrameBuffer<1024>& frame) {
    // Decode FrameBuffer commands into pixel buffer (ARGB8888)
    std::fill(pixels_.begin(), pixels_.end(), 0u);
    for (std::size_t i = 0; i < frame.size(); ++i) {
        const auto &cmd = frame.commands[i];
        switch (cmd.type) {
            case ege::RenderCommandType::Clear:
                std::fill(pixels_.begin(), pixels_.end(), cmd.color);
                break;
            case ege::RenderCommandType::Rect:
                draw_filled_rect(pixels_, width_, height_, cmd.rect.x, cmd.rect.y, cmd.rect.w, cmd.rect.h, cmd.color);
                break;
            default:
                break;
        }
    }

    // update texture and present
    void* texPixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(texture_, nullptr, &texPixels, &pitch) == 0) {
        // copy rows (pitch may be larger than width*4)
        for (std::size_t y = 0; y < height_; ++y) {
            std::memcpy(static_cast<uint8_t*>(texPixels) + y * pitch,
                        pixels_.data() + y * width_, width_ * sizeof(uint32_t));
        }
        SDL_UnlockTexture(texture_);
    }

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
    SDL_PumpEvents();
}

void SDLBackend::poll_input(std::vector<ege::Event>& out)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        ege::Event e{};
        switch (ev.type) {
        case SDL_QUIT:
            e.type = ege::EventType::Input; e.id = uint32_t(ege::InputCode::Quit); e.payload.i = 1; e.pos.x = 0; e.pos.y = 0; break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            e.type = ege::EventType::Input; e.id = uint32_t(ev.key.keysym.sym);
            e.payload.i = (ev.type == SDL_KEYDOWN) ? 1 : 0; e.pos.x = 0; e.pos.y = 0; break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            e.type = ege::EventType::Input; e.id = uint32_t(ev.button.button);
            e.payload.i = (ev.type == SDL_MOUSEBUTTONDOWN) ? 1 : 0;
            e.pos.x = ev.button.x; e.pos.y = ev.button.y; break;
        case SDL_MOUSEMOTION:
            e.type = ege::EventType::Input; e.id = 0; e.payload.i = 0;
            e.pos.x = ev.motion.x; e.pos.y = ev.motion.y; break;
        default:
            continue;
        }
        // enqueue into internal event queue; if full, drop event
        (void)event_queue_.push(e);
    }
    // drain internal queue into out vector
    ege::Event tmp;
    while (event_queue_.pop(tmp)) {
        out.push_back(tmp);
    }
}

bool SDLBackend::try_pop_event(ege::Event &out)
{
    return event_queue_.pop(out);
}

void SDLBackend::drain_events(std::vector<ege::Event>& out)
{
    ege::Event tmp;
    while (event_queue_.pop(tmp)) out.push_back(tmp);
}

bool SDLBackend::open_audio(int sample_rate)
{
    if (audio_dev_ != 0) return true;
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = sample_rate;
    want.format = AUDIO_F32SYS;
    want.channels = 1;
    want.samples = 1024;
    SDL_AudioSpec have;
    audio_dev_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audio_dev_ == 0) return false;
    audio_rate_ = have.freq;
    SDL_PauseAudioDevice(audio_dev_, 0);
    return true;
}

void SDLBackend::trigger_sound(uint32_t sound_id, float frequency, float duration)
{
    if (audio_dev_ == 0) {
        if (!open_audio(44100)) return;
    }
    const int rate = (audio_rate_ > 0) ? audio_rate_ : 44100;
    const int samples = int(duration * rate);
    if (samples <= 0) return;
    std::vector<float> buf;
    buf.resize(samples);
    const double two_pi_f = 2.0 * M_PI * double(frequency);
    for (int i = 0; i < samples; ++i) {
        double t = double(i) / double(rate);
        float env = 1.0f;
        const double attack = 0.01;
        const double release = 0.05;
        if (t < attack) env = float(t / attack);
        else if (t > (duration - release)) env = float((duration - t) / release);
        if (env < 0.0f) env = 0.0f;
        buf[i] = float(0.25f * env * std::sin(two_pi_f * t));
    }
    SDL_QueueAudio(audio_dev_, buf.data(), int(buf.size() * sizeof(float)));
    (void)sound_id;
}

} // namespace ege::backend
