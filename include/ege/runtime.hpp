#pragma once
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>
#include <ege/engine/render_command.hpp>
#include <ege/engine/render_pipeline.hpp>
#include <ege/engine/event.hpp>
#include <ege/backend.hpp>

namespace ege {

// Layer base class (inheritance-based)
struct Layer {
    virtual ~Layer() = default;
    // return true if event was consumed
    virtual bool on_event(const Event &e) { (void)e; return false; }
    // update logic (dt seconds)
    virtual void on_update(float dt) { (void)dt; }
    // record render commands into given pipeline (producer side)
    virtual void on_render(ege::SPSCRenderPipeline<1024,4,8>& pipeline, int frame_count) { (void)pipeline; (void)frame_count; }
};

// Simple runtime that drives backend, events and layers. Not thread-safe.
struct Runtime {
    Runtime(backend::Backend& backend, ege::SPSCRenderPipeline<1024,4,8>& pipeline) noexcept
        : backend_(backend), pipeline_(pipeline), running_(false) {}

    ~Runtime() = default;

    void push_layer(Layer* layer) { layers_.push_back(layer); }

    void run() {
        running_ = true;
        int frame_count = 0;
        while (running_) {
            // Poll input events
            std::vector<ege::Event> events;
            backend_.poll_input(events);
            backend_.drain_events(events);

            // Dispatch events to layers (top-first); if consumed, stop propagation
            for (const auto &ev : events) {
                bool handled = false;
                for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
                    if ((*it)->on_event(ev)) { handled = true; break; }
                }
                (void)handled;
            }

            // Update layers
            const float dt = 1.0f / 60.0f;
            for (auto* l : layers_) l->on_update(dt);

            // Render: producer records commands into pipeline
            auto &buf = pipeline_.begin_frame();
            for (auto* l : layers_) l->on_render(pipeline_, frame_count);
            pipeline_.submit_frame();

            // Consumer: present latest frame if available
            uint32_t idx;
            const auto &popped = pipeline_.try_consume(idx);
            if (idx != UINT32_MAX) {
                ege::FrameBuffer<1024> out;
                popped.decode(out);
                backend_.present(out);
                assert(pipeline_.release_buffer(idx));
            }

            ++frame_count;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    void stop() { running_ = false; }

private:
    backend::Backend& backend_;
    ege::SPSCRenderPipeline<1024,4,8>& pipeline_;

    std::vector<Layer*> layers_;
    bool running_ = false;
};

} // namespace ege
