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

// Layer base class (inheritance-based).
//
// A layer implements a small lifecycle: event handling, update, render and
// optional cleanup on exit. For rendering, the runtime owns the frame
// lifecycle: before calling `on_render` it binds a writable command-buffer
// pointer into the layer and unbinds it afterwards. Layers should not call
// pipeline APIs directly — they must use the protected `cmdbuf_` pointer to
// push rendering commands. When `on_render` is invoked the runtime guarantees
// the layer is visible (unless `is_visible()` is false) so layers may omit
// their own visibility guards if desired.
struct Layer {
    virtual ~Layer() = default;

    // return true if event was consumed
    virtual bool on_event(const Event &e) { (void)e; return false; }

    // update logic (dt seconds)
    virtual void on_update(float dt) { (void)dt; }

    // called when runtime is shutting down; layers should release resources here
    virtual void on_exit() noexcept { }

    // Render entry point for layers. The runtime will bind a valid
    // `ege::SPSCRenderPipeline<...>::CmdBuf*` into `cmdbuf_` before calling
    // `on_render(frame_count)` and will unbind it afterwards. Layers should
    // push commands into `cmdbuf_` (for example: `cmdbuf_->push_rect(...)`).
    // The signature intentionally hides pipeline details so user code only
    // focuses on what to render, not how frames are managed.
    virtual void on_render(int /*frame_count*/) { }

    // Runtime-internal helpers to bind/unbind the active command buffer.
    // These are called by `Runtime` and should not be used by client code.
    void _bind_cmdbuf(ege::SPSCRenderPipeline<1024,4,8>::CmdBuf* b) noexcept { cmdbuf_ = b; }
    void _unbind_cmdbuf() noexcept { cmdbuf_ = nullptr; }

    // Visibility helpers layers can use; runtime consults `is_visible()` to
    // decide whether to call `on_render` for a given layer.
    bool is_visible() const noexcept { return visible_; }
    void show() { visible_ = true; }
    void hide() { visible_ = false; }

protected:
    // Layers may use this protected pointer when recording commands. It is
    // non-owning and only valid during the `on_render` call invoked by the
    // runtime.
    ege::SPSCRenderPipeline<1024,4,8>::CmdBuf* cmdbuf_ = nullptr;
    bool visible_ = false;

};

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

            // Dispatch events to layers (top-first); if consumed, stop propagation.
            // If we receive a Quit input event, request runtime stop.
            for (const auto &ev : events) {
                if (ev.is_shutdown_event()) {
                    running_ = false;
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

            // Render: acquire a producer buffer once and let layers record
            // commands into the same buffer. Layers should assume a valid
            // frame is already available when `on_render()` is called and
            // only push commands — they must NOT call `begin_frame()` or
            // `submit_frame()` themselves.
            {
                auto opt = pipeline_.begin_frame();
                if (opt) {
                    auto &refwrap = *opt; // reference_wrapper<CmdBuf>
                    auto &buf = refwrap.get();
                    // runtime provides the active buffer; bind it to each
                    // layer, call `on_render`, then unbind.
                    for (auto* l : layers_) {
                        if (!l->is_visible()) continue;
                        l->_bind_cmdbuf(&buf);
                        l->on_render(frame_count);
                        l->_unbind_cmdbuf();
                    }
                    pipeline_.submit_frame();
                }
                // if no buffer available, skip recording this frame
            }

            // Consumer: drain any produced frames and present the latest one.
            // Layers render in push order; consuming all and presenting the last
            // ensures top-most layers drawn later appear on screen.
            ege::FrameBuffer<1024> last_out;
            bool have_frame = false;
            while (true) {
                uint32_t idx;
                const auto &popped = pipeline_.try_consume(idx);
                if (idx == UINT32_MAX) break;
                ege::FrameBuffer<1024> tmp;
                popped.decode(tmp);
                // release this buffer immediately
                assert(pipeline_.release_buffer(idx));
                last_out = tmp;
                have_frame = true;
            }
            if (have_frame) {
                backend_.present(last_out);
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
