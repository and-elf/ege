#pragma once
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>
#include <ege/engine/render_command.hpp>
#include <ege/engine/render_pipeline.hpp>
#include <ege/engine/event.hpp>
#include <ege/backend.hpp>
#include <ege/physics.hpp>

namespace ege {

// Layer base class (inheritance-based)
struct Layer {
    virtual ~Layer() = default;
    // return true if event was consumed
    virtual bool on_event(const Event &e) { (void)e; return false; }
    // update logic (dt seconds)
    virtual void on_update(float dt) { (void)dt; }
    // called when runtime is shutting down; layers should release resources here
    virtual void on_exit() noexcept { }
    // record render commands into given pipeline (producer side)
    virtual void on_render(ege::SPSCRenderPipeline<1024,4,8>& pipeline, int frame_count) { (void)pipeline; (void)frame_count; }
};

bool is_shutdown_event(const Event &e) {
    return e.type == EventType::Input && e.id == uint32_t(InputCode::Quit);
}

// Simple runtime that drives backend, events and layers. Not thread-safe.
struct Runtime {
    Runtime(backend::Backend& backend, ege::SPSCRenderPipeline<1024,4,8>& pipeline, PhysicsSystem &physics) noexcept
        : backend_(backend), pipeline_(pipeline), running_(false), physics_(physics) {}

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

            // Dispatch events to layers (top-first); if consumed, stop propagation.
            // If we receive a Quit input event, request runtime stop.
            for (const auto &ev : events) {
                if (is_shutdown_event(ev)) {
                    running_ = false;
                    std::printf("Received shutdown event, stopping runtime...\n");
                    break;
                }
                for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
                    if ((*it)->on_event(ev)) break;
                }
            }

            // Update layers and physics (fixed step)
            const float dt = 1.0f / 60.0f;
            for (auto* l : layers_) l->on_update(dt);
            physics_.step(dt);

            // Render: producer records commands into pipeline. Layers are
            // responsible for calling `begin_frame()` and `submit_frame()`
            // on the pipeline when recording commands.
            for (auto* l : layers_) l->on_render(pipeline_, frame_count);

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

        // Runtime stopping: notify layers to clean up in reverse order.
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            (*it)->on_exit();
        }
    }

    void stop() { running_ = false; }

private:
    backend::Backend& backend_;
    ege::SPSCRenderPipeline<1024,4,8>& pipeline_;

    std::vector<Layer*> layers_;
    bool running_ = false;
    PhysicsSystem& physics_;
};

} // namespace ege
